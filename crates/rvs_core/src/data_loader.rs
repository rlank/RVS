//! Asynchronous DataLoader and tile streaming pipeline adhering to `rust-async-patterns`.
//!
//! Decouples I/O ingestion (raster GeoTIFF / SQLite / vector) from CPU batch computation
//! using Tokio bounded channels, async traits, and cooperative cancellation tokens.

use std::collections::HashMap;
use std::sync::Arc;
use async_trait::async_trait;
use tokio::sync::mpsc;
use tokio_util::sync::CancellationToken;
use tracing::{info, instrument};
use thiserror::Error;
use crate::soa::{LandscapeCellBuffers, LandscapeShrubBuffers};

// =============================================================================
// 1. Error Definitions
// =============================================================================

/// Errors arising during asynchronous data ingestion.
#[derive(Debug, Error)]
pub enum DataLoaderError {
    #[error("I/O error during ingestion: {0}")]
    Io(#[from] std::io::Error),

    #[error("Database error during ingestion: {0}")]
    Database(String),

    #[error("Channel communication error: {0}")]
    Channel(String),

    #[error("Data parsing or validation error: {0}")]
    Validation(String),

    #[error("Format parsing error: {0}")]
    Format(String),

    #[error("Ingestion task cancelled")]
    Cancelled,
}

// =============================================================================
// 2. Reference Data Models (Feature 15)
// =============================================================================

/// Pre-compiled biomass equation record.
#[derive(Debug, Clone, Copy, PartialEq)]
#[repr(C)]
pub struct BioEquationRecord {
    pub eqn_num: u16,
    pub eqn_type: u8,
    pub cf1: f64,
    pub cf2: f64,
    pub cf3: f64,
    pub cf4: f64,
}

impl Default for BioEquationRecord {
    fn default() -> Self {
        Self {
            eqn_num: 0,
            eqn_type: 0,
            cf1: 0.0,
            cf2: 0.0,
            cf3: 0.0,
            cf4: 0.0,
        }
    }
}

/// Pre-compiled fuel equation record.
#[derive(Debug, Clone, Copy, PartialEq)]
#[repr(C)]
pub struct FuelEquationRecord {
    pub eqn_num: u16,
    pub eqn_type: u8,
    pub cf1: f64,
    pub cf2: f64,
    pub cf3: f64,
    pub cf4: f64,
}

impl Default for FuelEquationRecord {
    fn default() -> Self {
        Self {
            eqn_num: 0,
            eqn_type: 0,
            cf1: 0.0,
            cf2: 0.0,
            cf3: 0.0,
            cf4: 0.0,
        }
    }
}

/// Shrub species crosswalk entry mapping to allometric equation numbers.
#[derive(Debug, Clone, Copy, PartialEq)]
#[repr(C)]
pub struct ShrubCrosswalkRecord {
    pub species_id: u32,
    pub bat_eq_num: u16,
    pub pch_eq_num: u16,
    pub fuel_fl_eq: u16,
}

impl Default for ShrubCrosswalkRecord {
    fn default() -> Self {
        Self {
            species_id: 0,
            bat_eq_num: 0,
            pch_eq_num: 0,
            fuel_fl_eq: 0,
        }
    }
}

/// Succession cohort entry from BPS_Combined_Growthrates.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct BpsCohortRecord {
    pub cohort: u8,
    pub cohort_type: u8,
    pub start_age: f64,
    pub end_age: f64,
    pub min_cc: f64,
    pub max_cc: f64,
    pub min_ht: f64,
    pub max_ht: f64,
    pub gr_ht: f64,
    pub gr_cov: f64,
}

impl Default for BpsCohortRecord {
    fn default() -> Self {
        Self {
            cohort: 0,
            cohort_type: b' ',
            start_age: 0.0,
            end_age: 0.0,
            min_cc: 0.0,
            max_cc: 0.0,
            min_ht: 0.0,
            max_ht: 0.0,
            gr_ht: 0.0,
            gr_cov: 0.0,
        }
    }
}

/// Disturbance event record from Dist_year7fire or spatial schedules.
#[derive(Debug, Clone, PartialEq)]
pub struct DisturbanceRecord {
    pub plot_id: i64,
    pub start_year: u32,
    pub stop_year: u32,
    pub dist_type: String,
    pub p1_val: f64,
}

/// Comprehensive In-Memory Reference Catalog (Feature 15).
///
/// Pre-loads all static equation coefficients, species crosswalks, and succession
/// growth rates once at startup, eliminating runtime database queries.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct ReferenceData {
    /// Direct array lookup for Biomass equations (indexed by EQN_NUM, length >= 1200)
    pub bio_equations: Vec<BioEquationRecord>,

    /// Direct array lookup for Fuel equations (indexed by EQN_NUM)
    pub fuel_equations: Vec<FuelEquationRecord>,

    /// Direct array lookup for Shrub crosswalks (indexed by interned species_id)
    pub shrub_crosswalk: Vec<ShrubCrosswalkRecord>,

    /// Shrub species code (e.g. "ARTR2") to biomass equation number (BAT2 / PCH)
    pub bio_crosswalk: HashMap<String, u16>,

    /// Shrub species code to fine fuel loading equation number
    pub fuel_crosswalk: HashMap<String, u16>,

    /// Dense species code string to interned species_id lookup table
    pub species_code_to_id: HashMap<String, u32>,

    /// Interned species_id to original species code string
    pub id_to_species_code: Vec<String>,

    /// Succession cohorts keyed by numeric BPS model code
    pub growth_rates: HashMap<u32, Vec<BpsCohortRecord>>,

    /// Contiguous coefficient slices for Rayon batch kernel acceleration
    pub shrub_eq_cf1: Vec<f64>,
    pub shrub_eq_cf2: Vec<f64>,
}

impl ReferenceData {
    /// O(1) direct array lookup for biomass equation coefficients.
    #[inline]
    pub fn get_bio_equation(&self, eqn_num: u16) -> Option<&BioEquationRecord> {
        let idx = eqn_num as usize;
        if idx < self.bio_equations.len() && self.bio_equations[idx].eqn_num != 0 {
            Some(&self.bio_equations[idx])
        } else {
            None
        }
    }

    /// O(1) direct array lookup for fuel equation coefficients.
    #[inline]
    pub fn get_fuel_equation(&self, eqn_num: u16) -> Option<&FuelEquationRecord> {
        let idx = eqn_num as usize;
        if idx < self.fuel_equations.len() && self.fuel_equations[idx].eqn_num != 0 {
            Some(&self.fuel_equations[idx])
        } else {
            None
        }
    }

    /// O(1) direct array lookup for shrub crosswalk by interned species_id.
    #[inline]
    pub fn get_shrub_crosswalk(&self, species_id: u32) -> Option<&ShrubCrosswalkRecord> {
        self.shrub_crosswalk.get(species_id as usize)
    }

    /// Returns a zero-allocation reference borrow of ReferenceData for Rayon batch kernel execution.
    #[inline]
    pub fn to_batch_ref(&self) -> &Self {
        self
    }
}

// =============================================================================
// 3. Tile Model & Abstract Interface (Feature 16 & Feature 17)
// =============================================================================

/// Spatial landscape tile holding canonical SoA cell buffers.
#[derive(Debug, Clone, PartialEq)]
pub struct LandscapeTileSoA {
    pub tile_id: u64,
    pub x_offset: usize,
    pub y_offset: usize,
    pub width: usize,
    pub height: usize,
    pub buffers: LandscapeCellBuffers,
}

/// Abstract async DataLoader interface decoupling simulation logic from storage schemas.
#[async_trait]
pub trait AsyncDataLoader: Send + Sync {
    /// Streams landscape tiles into a bounded mpsc sender with cooperative cancellation.
    async fn stream_landscape_tiles(
        &self,
        sender: mpsc::Sender<LandscapeTileSoA>,
        cancel: CancellationToken,
    ) -> Result<(), DataLoaderError>;

    /// Loads static reference coefficients catalog into memory.
    fn load_reference_data(&self) -> Result<ReferenceData, DataLoaderError>;
}

// =============================================================================
// 4. Producer-Consumer Rayon Bridge (Feature 18 & Feature 19)
// =============================================================================

/// Producer-Consumer Pipeline Runner:
///
/// Decouples asynchronous tile I/O from synchronous Rayon batch processing via bounded
/// mpsc channels and structured concurrency. Uses `tokio::task::block_in_place` on
/// multi-threaded runtimes to prevent async worker starvation.
#[instrument(skip(loader, process_tile))]
pub async fn run_producer_consumer_pipeline<F>(
    loader: Arc<dyn AsyncDataLoader>,
    channel_capacity: usize,
    mut process_tile: F,
) -> Result<(), DataLoaderError>
where
    F: FnMut(LandscapeTileSoA) -> Result<(), DataLoaderError> + Send,
{
    if channel_capacity == 0 {
        return Err(DataLoaderError::Validation(
            "channel_capacity must be greater than zero".into(),
        ));
    }

    let (tx, mut rx) = mpsc::channel::<LandscapeTileSoA>(channel_capacity);
    let cancel = CancellationToken::new();
    let cancel_producer = cancel.clone();
    let producer_loader = Arc::clone(&loader);

    // Background Tokio task prefetching tiles
    let producer_handle = tokio::spawn(async move {
        info!("Starting tile prefetching producer task");
        producer_loader
            .stream_landscape_tiles(tx, cancel_producer)
            .await
    });

    // Consumer loop processing tiles as they arrive without blocking async runtime
    let mut consumer_err = None;
    while let Some(tile) = rx.recv().await {
        let res = match tokio::runtime::Handle::try_current() {
            Ok(handle) if handle.runtime_flavor() == tokio::runtime::RuntimeFlavor::MultiThread => {
                tokio::task::block_in_place(|| process_tile(tile))
            }
            _ => process_tile(tile),
        };
        if let Err(err) = res {
            cancel.cancel();
            consumer_err = Some(err);
            break;
        }
    }

    // Explicitly drop receiver to close channel and awaken any producer blocked on send()
    drop(rx);

    // Await producer task completion and propagate any error
    let producer_res = producer_handle
        .await
        .map_err(|e| DataLoaderError::Channel(e.to_string()))?;

    if let Some(err) = consumer_err {
        return Err(err);
    }

    producer_res?;

    info!("Producer-consumer pipeline completed successfully");
    Ok(())
}

// =============================================================================
// 5. Native SQLite FFI Bindings & RAII Wrappers (Zero External Dependencies)
// =============================================================================

#[link(name = "sqlite3")]
extern "C" {
    fn sqlite3_open(
        filename: *const std::os::raw::c_char,
        ppDb: *mut *mut std::ffi::c_void,
    ) -> std::os::raw::c_int;

    fn sqlite3_prepare_v2(
        db: *mut std::ffi::c_void,
        zSql: *const std::os::raw::c_char,
        nByte: std::os::raw::c_int,
        ppStmt: *mut *mut std::ffi::c_void,
        pzTail: *mut *const std::os::raw::c_char,
    ) -> std::os::raw::c_int;

    fn sqlite3_step(pStmt: *mut std::ffi::c_void) -> std::os::raw::c_int;

    fn sqlite3_column_int64(
        pStmt: *mut std::ffi::c_void,
        iCol: std::os::raw::c_int,
    ) -> i64;

    fn sqlite3_column_int(
        pStmt: *mut std::ffi::c_void,
        iCol: std::os::raw::c_int,
    ) -> std::os::raw::c_int;

    fn sqlite3_column_double(
        pStmt: *mut std::ffi::c_void,
        iCol: std::os::raw::c_int,
    ) -> f64;

    fn sqlite3_column_text(
        pStmt: *mut std::ffi::c_void,
        iCol: std::os::raw::c_int,
    ) -> *const std::os::raw::c_uchar;

    fn sqlite3_column_type(
        pStmt: *mut std::ffi::c_void,
        iCol: std::os::raw::c_int,
    ) -> std::os::raw::c_int;

    fn sqlite3_finalize(pStmt: *mut std::ffi::c_void) -> std::os::raw::c_int;

    fn sqlite3_close(db: *mut std::ffi::c_void) -> std::os::raw::c_int;

    fn sqlite3_errmsg(db: *mut std::ffi::c_void) -> *const std::os::raw::c_char;
}

const SQLITE_OK: i32 = 0;
const SQLITE_ROW: i32 = 100;
const SQLITE_NULL: i32 = 5;

/// RAII wrapper around sqlite3 database connection.
struct SqliteDb {
    handle: *mut std::ffi::c_void,
}

// SqliteDb handle is only accessed linearly or sent across threads when closed
unsafe impl Send for SqliteDb {}
unsafe impl Sync for SqliteDb {}

impl SqliteDb {
    fn open(path: &str) -> Result<Self, DataLoaderError> {
        let c_path = std::ffi::CString::new(path)
            .map_err(|e| DataLoaderError::Validation(format!("Invalid path string: {}", e)))?;
        let mut handle = std::ptr::null_mut();
        let rc = unsafe { sqlite3_open(c_path.as_ptr(), &mut handle) };
        if rc != SQLITE_OK || handle.is_null() {
            let msg = if !handle.is_null() {
                let err = unsafe {
                    std::ffi::CStr::from_ptr(sqlite3_errmsg(handle))
                        .to_string_lossy()
                        .into_owned()
                };
                unsafe { sqlite3_close(handle) };
                err
            } else {
                format!("Failed to open sqlite database, rc = {}", rc)
            };
            return Err(DataLoaderError::Database(msg));
        }
        Ok(Self { handle })
    }

    fn prepare(&self, sql: &str) -> Result<SqliteStmt, DataLoaderError> {
        let c_sql = std::ffi::CString::new(sql)
            .map_err(|e| DataLoaderError::Validation(format!("Invalid SQL string: {}", e)))?;
        let mut stmt = std::ptr::null_mut();
        let rc = unsafe {
            sqlite3_prepare_v2(
                self.handle,
                c_sql.as_ptr(),
                -1,
                &mut stmt,
                std::ptr::null_mut(),
            )
        };
        if rc != SQLITE_OK || stmt.is_null() {
            let err = unsafe {
                std::ffi::CStr::from_ptr(sqlite3_errmsg(self.handle))
                    .to_string_lossy()
                    .into_owned()
            };
            return Err(DataLoaderError::Database(format!("SQL prepare error: {}", err)));
        }
        Ok(SqliteStmt { handle: stmt })
    }
}

impl Drop for SqliteDb {
    fn drop(&mut self) {
        if !self.handle.is_null() {
            unsafe {
                sqlite3_close(self.handle);
            }
            self.handle = std::ptr::null_mut();
        }
    }
}

/// RAII wrapper around prepared sqlite3 statement.
struct SqliteStmt {
    handle: *mut std::ffi::c_void,
}

impl SqliteStmt {
    fn step(&self) -> i32 {
        unsafe { sqlite3_step(self.handle) }
    }

    fn column_int64(&self, col: i32) -> i64 {
        unsafe { sqlite3_column_int64(self.handle, col) }
    }

    fn column_int(&self, col: i32) -> i32 {
        unsafe { sqlite3_column_int(self.handle, col) }
    }

    fn column_double(&self, col: i32) -> f64 {
        unsafe { sqlite3_column_double(self.handle, col) }
    }

    fn column_text(&self, col: i32) -> String {
        unsafe {
            let ptr = sqlite3_column_text(self.handle, col);
            if ptr.is_null() {
                String::new()
            } else {
                std::ffi::CStr::from_ptr(ptr as *const std::os::raw::c_char)
                    .to_string_lossy()
                    .trim()
                    .to_string()
            }
        }
    }

    fn is_null(&self, col: i32) -> bool {
        unsafe { sqlite3_column_type(self.handle, col) == SQLITE_NULL }
    }
}

impl Drop for SqliteStmt {
    fn drop(&mut self) {
        if !self.handle.is_null() {
            unsafe {
                sqlite3_finalize(self.handle);
            }
            self.handle = std::ptr::null_mut();
        }
    }
}

// =============================================================================
// 6. SQLite DataLoader Backend (Feature 20)
// =============================================================================

/// SQLite DataLoader ingesting legacy relational tables (`Plots_FCCS`, `Shrubs_FCCS`).
///
/// Ingests data once at startup into canonical SoA buffers and immediately closes
/// the database handle, achieving zero runtime queries during simulation loops.
pub struct SQLiteDataLoader {
    pub db_path: String,
    pub tile_size: usize,
}

impl SQLiteDataLoader {
    /// Creates a new `SQLiteDataLoader` targeting the specified database path.
    pub fn new<P: Into<String>>(db_path: P) -> Self {
        Self {
            db_path: db_path.into(),
            tile_size: 64,
        }
    }

    /// Sets the tile chunking size for async streaming.
    pub fn with_tile_size(mut self, tile_size: usize) -> Self {
        self.tile_size = if tile_size > 0 { tile_size } else { 64 };
        self
    }

    /// Ingests `Plots_FCCS` (61 plots) and `Shrubs_FCCS` (102 shrubs) into canonical SoA buffers.
    ///
    /// The database connection is closed immediately upon exiting this method.
    pub fn load_landscape(&self) -> Result<(LandscapeCellBuffers, LandscapeShrubBuffers), DataLoaderError> {
        let db = SqliteDb::open(&self.db_path)?;

        // 1. Ingest Plots_FCCS
        let stmt_plots = db.prepare(
            "SELECT PLOT_ID, BPS_CODE, herb_cover, herb_height, PPT_3, NDVI_3 FROM Plots_FCCS ORDER BY PLOT_ID ASC"
        )?;

        struct PlotRecord {
            herb_cover: f64,
            herb_height: f64,
            precip: f64,
            ndvi: f64,
        }

        let mut plot_records = Vec::new();
        let mut plot_id_to_index: HashMap<i64, usize> = HashMap::new();

        while stmt_plots.step() == SQLITE_ROW {
            let pid = stmt_plots.column_int64(0);
            let hc = if stmt_plots.is_null(2) { 0.0 } else { stmt_plots.column_double(2).clamp(0.0, 100.0) };
            let hh = if stmt_plots.is_null(3) { 0.0 } else { stmt_plots.column_double(3).max(0.0) };
            let ppt = if stmt_plots.is_null(4) { 0.0 } else { stmt_plots.column_double(4).max(0.0) };
            let ndvi = if stmt_plots.is_null(5) { 0.0 } else { stmt_plots.column_double(5).clamp(0.0, 1.0) };

            let idx = plot_records.len();
            plot_id_to_index.insert(pid, idx);
            plot_records.push(PlotRecord {
                herb_cover: hc,
                herb_height: hh,
                precip: ppt,
                ndvi,
            });
        }
        drop(stmt_plots);

        let n = plot_records.len();
        let mut cells = LandscapeCellBuffers::new(n);
        for (i, p) in plot_records.iter().enumerate() {
            cells.herb_cover[i] = p.herb_cover;
            cells.herb_height[i] = p.herb_height;
            cells.precip[i] = p.precip;
            cells.ndvi[i] = p.ndvi;
            cells.elevation[i] = 0.0;
        }

        // 2. Ingest Shrubs_FCCS into Segmented CSR layout
        let stmt_shrubs = db.prepare(
            "SELECT PLOT_ID, SPP_CODE, Cover, Height FROM Shrubs_FCCS ORDER BY PLOT_ID ASC"
        )?;

        struct ShrubRow {
            plot_id: i64,
            spp_code: String,
            cover: f64,
            height: f64,
        }

        let mut shrub_rows = Vec::new();
        let mut counts_per_cell = vec![0usize; n];

        while stmt_shrubs.step() == SQLITE_ROW {
            let pid = stmt_shrubs.column_int64(0);
            let spp = stmt_shrubs.column_text(1);
            let cov = if stmt_shrubs.is_null(2) { 0.0 } else { stmt_shrubs.column_double(2).clamp(0.0, 100.0) };
            let ht = if stmt_shrubs.is_null(3) { 0.0 } else { stmt_shrubs.column_double(3).max(0.0) };

            if let Some(&c_idx) = plot_id_to_index.get(&pid) {
                counts_per_cell[c_idx] += 1;
            }

            shrub_rows.push(ShrubRow {
                plot_id: pid,
                spp_code: spp,
                cover: cov,
                height: ht,
            });
        }
        drop(stmt_shrubs);

        let m = shrub_rows.len();
        let mut shrubs = LandscapeShrubBuffers::new(n, m);

        // Build CSR prefix sums: cell_offsets length N + 1
        let mut offset = 0usize;
        shrubs.cell_offsets[0] = 0;
        for i in 0..n {
            offset += counts_per_cell[i];
            shrubs.cell_offsets[i + 1] = offset;
        }

        let mut current_offset = shrubs.cell_offsets.clone();
        let mut species_map: HashMap<String, u32> = HashMap::new();

        for row in shrub_rows {
            if let Some(&c_idx) = plot_id_to_index.get(&row.plot_id) {
                let s_idx = current_offset[c_idx];
                let next_id = species_map.len() as u32;
                let sp_id = *species_map.entry(row.spp_code.clone()).or_insert(next_id);

                shrubs.species_id[s_idx] = sp_id;
                shrubs.cover[s_idx] = row.cover;
                shrubs.height[s_idx] = row.height;
                shrubs.biomass[s_idx] = 0.0;
                shrubs.stems_per_acre[s_idx] = 0.0;

                current_offset[c_idx] += 1;
            }
        }

        // Aggregate plot-level shrub cover & max shrub height into cells
        for c in 0..n {
            let start = shrubs.cell_offsets[c];
            let end = shrubs.cell_offsets[c + 1];
            let mut sum_cover = 0.0;
            let mut max_ht = 0.0;
            for s in start..end {
                sum_cover += shrubs.cover[s];
                if shrubs.height[s] > max_ht {
                    max_ht = shrubs.height[s];
                }
            }
            cells.shrub_cover[c] = sum_cover.min(100.0);
            cells.shrub_height[c] = max_ht;
        }

        // Validate all 5 CSR invariants
        shrubs.validate().map_err(DataLoaderError::Validation)?;

        // Close DB handle automatically when db drops
        drop(db);

        Ok((cells, shrubs))
    }

    /// Loads disturbance schedule from `Dist_year7fire`.
    pub fn load_disturbance_schedule(&self) -> Result<Vec<DisturbanceRecord>, DataLoaderError> {
        let db = SqliteDb::open(&self.db_path)?;
        let stmt = db.prepare(
            "SELECT PLOT_ID, START_YEAR, DIST_TYPE, P1_VAL FROM Dist_year7fire"
        )?;

        let mut records = Vec::new();
        while stmt.step() == SQLITE_ROW {
            let pid = stmt.column_int64(0);
            let start_year = stmt.column_int(1) as u32;
            let dist_type = stmt.column_text(2);
            let p1_val = if stmt.is_null(3) { 0.0 } else { stmt.column_double(3) };

            records.push(DisturbanceRecord {
                plot_id: pid,
                start_year,
                stop_year: start_year,
                dist_type,
                p1_val,
            });
        }

        Ok(records)
    }
}

#[async_trait]
impl AsyncDataLoader for SQLiteDataLoader {
    async fn stream_landscape_tiles(
        &self,
        sender: mpsc::Sender<LandscapeTileSoA>,
        cancel: CancellationToken,
    ) -> Result<(), DataLoaderError> {
        let (cells, _shrubs) = self.load_landscape()?;
        let total_cells = cells.len();
        if total_cells == 0 {
            return Ok(());
        }

        let chunk_size = self.tile_size;
        let mut tile_id = 0u64;
        let mut start = 0usize;

        while start < total_cells {
            if cancel.is_cancelled() {
                return Err(DataLoaderError::Cancelled);
            }

            let end = (start + chunk_size).min(total_cells);
            let count = end - start;

            let mut tile_buffers = LandscapeCellBuffers::new(count);
            for i in 0..count {
                let src_idx = start + i;
                tile_buffers.herb_cover[i] = cells.herb_cover[src_idx];
                tile_buffers.herb_height[i] = cells.herb_height[src_idx];
                tile_buffers.herb_biomass[i] = cells.herb_biomass[src_idx];
                tile_buffers.shrub_cover[i] = cells.shrub_cover[src_idx];
                tile_buffers.shrub_height[i] = cells.shrub_height[src_idx];
                tile_buffers.ndvi[i] = cells.ndvi[src_idx];
                tile_buffers.precip[i] = cells.precip[src_idx];
                tile_buffers.elevation[i] = cells.elevation[src_idx];
            }

            let tile = LandscapeTileSoA {
                tile_id,
                x_offset: start,
                y_offset: 0,
                width: count,
                height: 1,
                buffers: tile_buffers,
            };

            tokio::select! {
                biased;
                _ = cancel.cancelled() => {
                    return Err(DataLoaderError::Cancelled);
                }
                send_res = sender.send(tile) => {
                    if send_res.is_err() {
                        return Ok(());
                    }
                }
            }

            tile_id += 1;
            start = end;
        }

        Ok(())
    }

    fn load_reference_data(&self) -> Result<ReferenceData, DataLoaderError> {
        let db = SqliteDb::open(&self.db_path)?;
        let mut ref_data = ReferenceData::default();

        // 1. Ingest Bio_Equation (find max equation number to build flat array)
        let stmt_eq = db.prepare(
            "SELECT EQN_NUM, CF1, CF2, CF3, CF4 FROM Bio_Equation WHERE EQN_NUM IS NOT NULL"
        )?;

        let mut max_eqn_num = 0u16;
        let mut eq_rows = Vec::new();

        while stmt_eq.step() == SQLITE_ROW {
            let eq_num = stmt_eq.column_int(0) as u16;
            let cf1 = if stmt_eq.is_null(1) { 0.0 } else { stmt_eq.column_double(1) };
            let cf2 = if stmt_eq.is_null(2) { 0.0 } else { stmt_eq.column_double(2) };
            let cf3 = if stmt_eq.is_null(3) { 0.0 } else { stmt_eq.column_double(3) };
            let cf4 = if stmt_eq.is_null(4) { 0.0 } else { stmt_eq.column_double(4) };

            if eq_num > max_eqn_num {
                max_eqn_num = eq_num;
            }
            eq_rows.push(BioEquationRecord {
                eqn_num: eq_num,
                eqn_type: 0,
                cf1,
                cf2,
                cf3,
                cf4,
            });
        }
        drop(stmt_eq);

        let array_size = (max_eqn_num as usize + 1).max(1200);
        ref_data.bio_equations = vec![BioEquationRecord::default(); array_size];
        for eq in eq_rows {
            let idx = eq.eqn_num as usize;
            ref_data.bio_equations[idx] = eq;
        }

        // 2. Ingest Bio_Crosswalk
        let stmt_cross = db.prepare("SELECT spp_code, BAT2, PCH FROM Bio_Crosswalk")?;
        while stmt_cross.step() == SQLITE_ROW {
            let spp = stmt_cross.column_text(0);
            let bat2 = stmt_cross.column_int(1) as u16;
            let pch = stmt_cross.column_int(2) as u16;

            if !spp.is_empty() {
                ref_data.bio_crosswalk.insert(spp.clone(), bat2);
                let next_id = ref_data.species_code_to_id.len() as u32;
                let sp_id = *ref_data.species_code_to_id.entry(spp.clone()).or_insert(next_id);

                while ref_data.id_to_species_code.len() <= sp_id as usize {
                    ref_data.id_to_species_code.push(String::new());
                }
                ref_data.id_to_species_code[sp_id as usize] = spp;

                while ref_data.shrub_crosswalk.len() <= sp_id as usize {
                    ref_data.shrub_crosswalk.push(ShrubCrosswalkRecord::default());
                }
                ref_data.shrub_crosswalk[sp_id as usize] = ShrubCrosswalkRecord {
                    species_id: sp_id,
                    bat_eq_num: bat2,
                    pch_eq_num: pch,
                    fuel_fl_eq: 0,
                };
            }
        }
        drop(stmt_cross);

        // 3. Ingest Fuel_Crosswalk
        let stmt_fuel = db.prepare("SELECT spp_code, FL1 FROM Fuel_Crosswalk")?;
        while stmt_fuel.step() == SQLITE_ROW {
            let spp = stmt_fuel.column_text(0);
            let fl1 = stmt_fuel.column_int(1) as u16;
            if !spp.is_empty() {
                ref_data.fuel_crosswalk.insert(spp.clone(), fl1);
                if let Some(&sp_id) = ref_data.species_code_to_id.get(&spp) {
                    if (sp_id as usize) < ref_data.shrub_crosswalk.len() {
                        ref_data.shrub_crosswalk[sp_id as usize].fuel_fl_eq = fl1;
                    }
                }
            }
        }
        drop(stmt_fuel);

        // 4. Ingest Fuel_Equation
        let stmt_fuel_eq = db.prepare(
            "SELECT EQN_NUM, CF1, CF2, CF3, CF4, eqn_type FROM Fuel_Equation WHERE EQN_NUM IS NOT NULL"
        )?;

        let mut max_fuel_eqn_num = 0u16;
        let mut fuel_eq_rows = Vec::new();

        while stmt_fuel_eq.step() == SQLITE_ROW {
            let eq_num = stmt_fuel_eq.column_int(0) as u16;
            let cf1 = if stmt_fuel_eq.is_null(1) { 0.0 } else { stmt_fuel_eq.column_double(1) };
            let cf2 = if stmt_fuel_eq.is_null(2) { 0.0 } else { stmt_fuel_eq.column_double(2) };
            let cf3 = if stmt_fuel_eq.is_null(3) { 0.0 } else { stmt_fuel_eq.column_double(3) };
            let cf4 = if stmt_fuel_eq.is_null(4) { 0.0 } else { stmt_fuel_eq.column_double(4) };
            let eqn_type = if stmt_fuel_eq.is_null(5) { 0 } else { stmt_fuel_eq.column_int(5) as u8 };

            if eq_num > max_fuel_eqn_num {
                max_fuel_eqn_num = eq_num;
            }
            fuel_eq_rows.push(FuelEquationRecord {
                eqn_num: eq_num,
                eqn_type,
                cf1,
                cf2,
                cf3,
                cf4,
            });
        }
        drop(stmt_fuel_eq);

        let fuel_array_size = (max_fuel_eqn_num as usize + 1).max(100);
        ref_data.fuel_equations = vec![FuelEquationRecord::default(); fuel_array_size];
        for eq in fuel_eq_rows {
            let idx = eq.eqn_num as usize;
            ref_data.fuel_equations[idx] = eq;
        }

        // 5. Ingest BPS_Combined_Growthrates
        let stmt_bps = db.prepare(
            "SELECT BPS_MODEL, COHORT, GR_HT, GR_COV, StartAge, EndAge, MIN_CC, MAX_CC, MIN_HT, MAX_HT FROM BPS_Combined_Growthrates"
        )?;
        while stmt_bps.step() == SQLITE_ROW {
            let bps_val = stmt_bps.column_int(0) as u32;
            let cohort = stmt_bps.column_int(1) as u8;
            let gr_ht = if stmt_bps.is_null(2) { 0.0 } else { stmt_bps.column_double(2) };
            let gr_cov = if stmt_bps.is_null(3) { 0.0 } else { stmt_bps.column_double(3) };
            let start_age = if stmt_bps.is_null(4) { 0.0 } else { stmt_bps.column_double(4) };
            let end_age = if stmt_bps.is_null(5) { 0.0 } else { stmt_bps.column_double(5) };
            let min_cc = if stmt_bps.is_null(6) { 0.0 } else { stmt_bps.column_double(6) };
            let max_cc = if stmt_bps.is_null(7) { 0.0 } else { stmt_bps.column_double(7) };
            let min_ht = if stmt_bps.is_null(8) { 0.0 } else { stmt_bps.column_double(8) };
            let max_ht = if stmt_bps.is_null(9) { 0.0 } else { stmt_bps.column_double(9) };

            let record = BpsCohortRecord {
                cohort,
                cohort_type: b'S',
                start_age,
                end_age,
                min_cc,
                max_cc,
                min_ht,
                max_ht,
                gr_ht,
                gr_cov,
            };

            ref_data.growth_rates.entry(bps_val).or_default().push(record);
        }
        drop(stmt_bps);

        // 6. Pre-compute shrub_eq_cf1 and shrub_eq_cf2 slices for species
        let num_species = ref_data.species_code_to_id.len();
        ref_data.shrub_eq_cf1 = vec![1.0; num_species];
        ref_data.shrub_eq_cf2 = vec![1.5; num_species];

        for (sp_code, &sp_id) in &ref_data.species_code_to_id {
            if let Some(&eq_num) = ref_data.bio_crosswalk.get(sp_code) {
                let eq_idx = eq_num as usize;
                if eq_idx < ref_data.bio_equations.len() && ref_data.bio_equations[eq_idx].eqn_num != 0 {
                    let cf1 = ref_data.bio_equations[eq_idx].cf1;
                    let cf2 = ref_data.bio_equations[eq_idx].cf2;
                    let idx = sp_id as usize;
                    if idx < ref_data.shrub_eq_cf1.len() {
                        ref_data.shrub_eq_cf1[idx] = cf1;
                        ref_data.shrub_eq_cf2[idx] = cf2;
                    }
                }
            }
        }

        Ok(ref_data)
    }
}

// =============================================================================
// 7. Spatial Raster DataLoader Backend (Feature 21)
// =============================================================================

/// Spatial Raster DataLoader ingesting multi-band gridded layers (NDVI, PPT, elevation, cover, height).
///
/// Flattens 2D grid pixels using canonical row-major indexing ($i = r \cdot C + c$) and
/// safely masks NoData sentinels (-9999.0 -> 0.0).
pub struct SpatialRasterDataLoader {
    pub rows: usize,
    pub cols: usize,
    pub nodata_val: f64,
    pub tile_size: usize,
    pub bands: HashMap<String, Vec<f64>>,
    pub reference_data: Option<ReferenceData>,
}

impl SpatialRasterDataLoader {
    /// Creates a new `SpatialRasterDataLoader` with dimensions and NoData sentinel.
    pub fn new(rows: usize, cols: usize, nodata_val: f64) -> Self {
        Self {
            rows,
            cols,
            nodata_val,
            tile_size: 64,
            bands: HashMap::new(),
            reference_data: None,
        }
    }

    /// Sets the 2D tile chunking size (e.g. 64 for 64x64 tiles).
    pub fn with_tile_size(mut self, tile_size: usize) -> Self {
        self.tile_size = if tile_size > 0 { tile_size } else { 64 };
        self
    }

    /// Attaches pre-loaded in-memory reference catalog.
    pub fn with_reference_data(mut self, ref_data: ReferenceData) -> Self {
        self.reference_data = Some(ref_data);
        self
    }

    /// Sets a gridded band by name, verifying that length equals `rows * cols`.
    pub fn set_band<S: Into<String>>(&mut self, name: S, data: Vec<f64>) -> Result<(), DataLoaderError> {
        let expected_len = self.rows * self.cols;
        if data.len() != expected_len {
            return Err(DataLoaderError::Validation(format!(
                "Band length ({}) does not match grid dimensions ({} x {} = {})",
                data.len(),
                self.rows,
                self.cols,
                expected_len
            )));
        }
        self.bands.insert(name.into(), data);
        Ok(())
    }

    /// Ingests multi-band dictionary into canonical 1D contiguous `LandscapeCellBuffers`.
    ///
    /// Implements row-major flattening and NoData sentinel clamping:
    /// - `ndvi`: masked to 0.0 or clamped to [0.0, 1.0]
    /// - `precip`: masked to 0.0 or clamped >= 0.0
    /// - `elevation`: masked to 0.0
    /// - `herb_cover`: masked to 0.0 or clamped to [0.0, 100.0]
    /// - `herb_height`: masked to 0.0 or clamped >= 0.0
    /// - `shrub_cover`: masked to 0.0 or clamped to [0.0, 100.0]
    /// - `shrub_height`: masked to 0.0 or clamped >= 0.0
    pub fn ingest_bands(&self, bands: &HashMap<String, Vec<f64>>) -> Result<LandscapeCellBuffers, DataLoaderError> {
        let n = self.rows * self.cols;
        let mut cells = LandscapeCellBuffers::new(n);

        let ndvi_band = bands.get("ndvi");
        let ppt_band = bands.get("precip");
        let elev_band = bands.get("elevation");
        let hc_band = bands.get("herb_cover");
        let hh_band = bands.get("herb_height");
        let sc_band = bands.get("shrub_cover");
        let sh_band = bands.get("shrub_height");

        for i in 0..n {
            if let Some(b) = ndvi_band {
                let v = b.get(i).copied().unwrap_or(self.nodata_val);
                cells.ndvi[i] = if v == self.nodata_val || !v.is_finite() || v < 0.0 {
                    0.0
                } else {
                    v.clamp(0.0, 1.0)
                };
            }

            if let Some(b) = ppt_band {
                let v = b.get(i).copied().unwrap_or(self.nodata_val);
                cells.precip[i] = if v == self.nodata_val || !v.is_finite() || v < 0.0 {
                    0.0
                } else {
                    v.max(0.0)
                };
            }

            if let Some(b) = elev_band {
                let v = b.get(i).copied().unwrap_or(self.nodata_val);
                cells.elevation[i] = if v == self.nodata_val || !v.is_finite() {
                    0.0
                } else {
                    v
                };
            }

            if let Some(b) = hc_band {
                let v = b.get(i).copied().unwrap_or(self.nodata_val);
                cells.herb_cover[i] = if v == self.nodata_val || !v.is_finite() || v < 0.0 {
                    0.0
                } else {
                    v.clamp(0.0, 100.0)
                };
            }

            if let Some(b) = hh_band {
                let v = b.get(i).copied().unwrap_or(self.nodata_val);
                cells.herb_height[i] = if v == self.nodata_val || !v.is_finite() || v < 0.0 {
                    0.0
                } else {
                    v.max(0.0)
                };
            }

            if let Some(b) = sc_band {
                let v = b.get(i).copied().unwrap_or(self.nodata_val);
                cells.shrub_cover[i] = if v == self.nodata_val || !v.is_finite() || v < 0.0 {
                    0.0
                } else {
                    v.clamp(0.0, 100.0)
                };
            }

            if let Some(b) = sh_band {
                let v = b.get(i).copied().unwrap_or(self.nodata_val);
                cells.shrub_height[i] = if v == self.nodata_val || !v.is_finite() || v < 0.0 {
                    0.0
                } else {
                    v.max(0.0)
                };
            }
        }

        Ok(cells)
    }

    /// Ingests currently stored bands into canonical SoA and CSR buffers.
    pub fn load_landscape(&self) -> Result<(LandscapeCellBuffers, LandscapeShrubBuffers), DataLoaderError> {
        let cells = self.ingest_bands(&self.bands)?;
        let n = cells.len();
        let shrubs = LandscapeShrubBuffers::new(n, 0);
        shrubs.validate().map_err(DataLoaderError::Validation)?;
        Ok((cells, shrubs))
    }
}

#[async_trait]
impl AsyncDataLoader for SpatialRasterDataLoader {
    async fn stream_landscape_tiles(
        &self,
        sender: mpsc::Sender<LandscapeTileSoA>,
        cancel: CancellationToken,
    ) -> Result<(), DataLoaderError> {
        let cells = self.ingest_bands(&self.bands)?;
        let mut tile_id = 0u64;

        let t_size = self.tile_size;
        let mut r = 0usize;

        while r < self.rows {
            if cancel.is_cancelled() {
                return Err(DataLoaderError::Cancelled);
            }

            let r_end = (r + t_size).min(self.rows);
            let tile_h = r_end - r;

            let mut c = 0usize;
            while c < self.cols {
                if cancel.is_cancelled() {
                    return Err(DataLoaderError::Cancelled);
                }

                let c_end = (c + t_size).min(self.cols);
                let tile_w = c_end - c;
                let tile_cells = tile_w * tile_h;

                let mut tile_buffers = LandscapeCellBuffers::new(tile_cells);

                for tr in 0..tile_h {
                    for tc in 0..tile_w {
                        let global_idx = (r + tr) * self.cols + (c + tc);
                        let local_idx = tr * tile_w + tc;

                        tile_buffers.herb_cover[local_idx] = cells.herb_cover[global_idx];
                        tile_buffers.herb_height[local_idx] = cells.herb_height[global_idx];
                        tile_buffers.herb_biomass[local_idx] = cells.herb_biomass[global_idx];
                        tile_buffers.shrub_cover[local_idx] = cells.shrub_cover[global_idx];
                        tile_buffers.shrub_height[local_idx] = cells.shrub_height[global_idx];
                        tile_buffers.ndvi[local_idx] = cells.ndvi[global_idx];
                        tile_buffers.precip[local_idx] = cells.precip[global_idx];
                        tile_buffers.elevation[local_idx] = cells.elevation[global_idx];
                    }
                }

                let tile = LandscapeTileSoA {
                    tile_id,
                    x_offset: c,
                    y_offset: r,
                    width: tile_w,
                    height: tile_h,
                    buffers: tile_buffers,
                };

                tokio::select! {
                    biased;
                    _ = cancel.cancelled() => {
                        return Err(DataLoaderError::Cancelled);
                    }
                    send_res = sender.send(tile) => {
                        if send_res.is_err() {
                            return Ok(());
                        }
                    }
                }

                tile_id += 1;
                c = c_end;
            }

            r = r_end;
        }

        Ok(())
    }

    fn load_reference_data(&self) -> Result<ReferenceData, DataLoaderError> {
        Ok(self.reference_data.clone().unwrap_or_default())
    }
}
