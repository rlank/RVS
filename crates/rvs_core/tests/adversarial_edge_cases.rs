//! Empirical Adversarial Stress Suite for Tier 5 Edge Cases & Concurrency
//!
//! Specifically challenges:
//! 1. Extreme landscape extents: 0 plots, 1 plot, 50,000+ plots
//! 2. Variable shrub sub-canopies: 0 shrubs per cell, 100 shrubs per cell, unlinked shrub foreign keys
//! 3. Extreme climate/raster inputs: 0 precip, extreme drought, extreme rainfall 5000mm,
//!    negative elevation, and NaN/NoData sentinel masking
//! 4. Rayon multithreading concurrency and determinism: bit-identical results between 1 thread
//!    and multi-threaded batch execution across prime and power-of-two landscape sizes
//! 5. Memory allocation: batch kernel iteration hot paths maintain zero heap allocations

use std::alloc::{GlobalAlloc, Layout, System};
use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
use std::sync::Mutex;

use rvs_core::batch::{
    batch_evaluate_biomass,
    batch_evaluate_disturbance_graze,
    batch_evaluate_fuels,
    batch_evaluate_succession,
    ReferenceData,
};
use rvs_core::data_loader::{
    DataLoaderError, SQLiteDataLoader, SpatialRasterDataLoader,
};
use rvs_core::soa::{LandscapeCellBuffers, LandscapeShrubBuffers};
use rvs_core::succession::{calc_herb_biomass_production, evaluate_herb_growth};

// Global Tracking Allocator for Zero-Allocation Verification
struct TrackingAllocator;
static ALLOC_COUNT: AtomicUsize = AtomicUsize::new(0);
static TRACKING_ACTIVE: AtomicBool = AtomicBool::new(false);

unsafe impl GlobalAlloc for TrackingAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        if TRACKING_ACTIVE.load(Ordering::Relaxed) {
            ALLOC_COUNT.fetch_add(1, Ordering::SeqCst);
        }
        System.alloc(layout)
    }
    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        System.dealloc(ptr, layout)
    }
}

#[global_allocator]
static GLOBAL: TrackingAllocator = TrackingAllocator;

static TEST_MUTEX: Mutex<()> = Mutex::new(());

fn make_test_landscape(
    num_cells: usize,
    shrubs_per_cell: usize,
) -> (LandscapeCellBuffers, LandscapeShrubBuffers, ReferenceData) {
    let num_shrubs = num_cells * shrubs_per_cell;
    let mut cells = LandscapeCellBuffers::new(num_cells);
    let mut shrubs = LandscapeShrubBuffers::new(num_cells, num_shrubs);

    for i in 0..num_cells {
        let fi = i as f64;
        cells.herb_cover[i] = 10.0 + (fi % 60.0);
        cells.herb_height[i] = 20.0 + (fi % 40.0);
        cells.herb_biomass[i] = 200.0 + (fi % 800.0);
        cells.shrub_cover[i] = 5.0 + (fi % 40.0);
        cells.shrub_height[i] = 40.0 + (fi % 120.0);
        cells.ndvi[i] = 0.2 + (fi % 50.0) * 0.01;
        cells.precip[i] = 250.0 + (fi % 500.0);
        cells.elevation[i] = 1000.0 + (fi % 1500.0);
    }

    let mut current_offset = 0;
    for c in 0..num_cells {
        shrubs.cell_offsets[c] = current_offset;
        for s in 0..shrubs_per_cell {
            let idx = current_offset + s;
            shrubs.species_id[idx] = (s % 5) as u32;
            shrubs.cover[idx] = 10.0 + (s as f64 * 5.0);
            shrubs.height[idx] = 50.0 + (s as f64 * 20.0);
            shrubs.biomass[idx] = 100.0 + (s as f64 * 50.0);
        }
        current_offset += shrubs_per_cell;
    }
    shrubs.cell_offsets[num_cells] = num_shrubs;

    shrubs.validate().expect("Landscape must satisfy CSR invariants");

    let refs = ReferenceData {
        shrub_eq_cf1: vec![1.5, 2.0, 2.5, 1.8, 2.2],
        shrub_eq_cf2: vec![1.1, 1.3, 1.2, 1.4, 1.15],
        ..Default::default()
    };

    (cells, shrubs, refs)
}

// =============================================================================
// 1. Extreme Landscape Extents (0 plots, 1 plot, 50,000+ plots)
// =============================================================================

#[test]
fn test_adv_extent_0_cells() {
    let _lock = TEST_MUTEX.lock().unwrap();

    let mut cells = LandscapeCellBuffers::new(0);
    let mut shrubs = LandscapeShrubBuffers::new(0, 0);
    let refs = ReferenceData::default();
    let mut fuels = vec![0.0; 0];

    // Must execute cleanly without error or out-of-bounds indexing
    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    batch_evaluate_fuels(&cells, &mut fuels, 0.5);
    batch_evaluate_succession(&mut cells.view_mut(), 1.0, 1.0);
    batch_evaluate_disturbance_graze(&mut cells.view_mut(), 20.0);

    assert_eq!(cells.num_cells, 0);
    assert_eq!(shrubs.num_shrubs, 0);
    assert_eq!(fuels.len(), 0);
}

#[test]
fn test_adv_extent_1_cell() {
    let _lock = TEST_MUTEX.lock().unwrap();

    let (mut cells, mut shrubs, refs) = make_test_landscape(1, 1);
    let mut fuels = vec![0.0; 1];

    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    batch_evaluate_fuels(&cells, &mut fuels, 0.5);
    batch_evaluate_succession(&mut cells.view_mut(), 1.5, 2.0);
    batch_evaluate_disturbance_graze(&mut cells.view_mut(), 10.0);

    assert_eq!(cells.num_cells, 1);
    assert!(cells.herb_biomass[0].is_finite());
    assert!(fuels[0].is_finite());
    assert!(cells.shrub_height[0].is_finite());
}

#[test]
fn test_adv_extent_50k_plus_cells() {
    let _lock = TEST_MUTEX.lock().unwrap();

    const N: usize = 50_000;
    const M_PER_CELL: usize = 3;
    let (mut cells, mut shrubs, refs) = make_test_landscape(N, M_PER_CELL);
    let mut fuels = vec![0.0; N];

    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    batch_evaluate_fuels(&cells, &mut fuels, 0.7);
    batch_evaluate_succession(&mut cells.view_mut(), 1.0, 0.5);
    batch_evaluate_disturbance_graze(&mut cells.view_mut(), 15.0);

    assert_eq!(cells.herb_biomass.len(), N);
    assert_eq!(shrubs.stems_per_acre.len(), N * M_PER_CELL);
    assert_eq!(fuels.len(), N);
    assert!(cells.herb_biomass[0].is_finite());
    assert!(cells.herb_biomass[N - 1].is_finite());
}

// =============================================================================
// 2. Variable Shrub Sub-Canopies (0 shrubs/cell, 100 shrubs/cell, unlinked FK)
// =============================================================================

#[test]
fn test_adv_variable_shrubs_zero_shrubs_per_cell() {
    let _lock = TEST_MUTEX.lock().unwrap();

    const N: usize = 1_000;
    let mut cells = LandscapeCellBuffers::new(N);
    for i in 0..N {
        cells.herb_cover[i] = 30.0;
        cells.herb_height[i] = 25.0;
        cells.herb_biomass[i] = 500.0;
    }
    let mut shrubs = LandscapeShrubBuffers::new(N, 0);
    assert!(shrubs.validate().is_ok());

    let refs = ReferenceData::default();
    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);

    // With 0 shrubs, attenuation ratio si <= 0 returns pre-biomass unchanged
    for b in &cells.herb_biomass {
        assert_eq!(*b, 500.0);
    }
}

#[test]
fn test_adv_variable_shrubs_100_shrubs_per_cell() {
    let _lock = TEST_MUTEX.lock().unwrap();

    const N: usize = 50;
    const M_PER_CELL: usize = 100;
    let (mut cells, mut shrubs, refs) = make_test_landscape(N, M_PER_CELL);

    assert_eq!(shrubs.num_shrubs, N * M_PER_CELL);
    assert!(shrubs.validate().is_ok());

    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);

    for s in &shrubs.stems_per_acre {
        assert!(s.is_finite());
        assert!(*s > 0.0);
    }
}

#[test]
fn test_adv_variable_shrubs_sparse_irregular() {
    let _lock = TEST_MUTEX.lock().unwrap();

    let shrub_counts = [0, 10, 0, 50, 0, 1, 0, 100];
    let n_cells = shrub_counts.len();
    let total_shrubs: usize = shrub_counts.iter().sum();

    let mut cells = LandscapeCellBuffers::new(n_cells);
    let mut shrubs = LandscapeShrubBuffers::new(n_cells, total_shrubs);

    let mut offset = 0;
    shrubs.cell_offsets[0] = 0;
    for (i, &count) in shrub_counts.iter().enumerate() {
        for s in 0..count {
            let idx = offset + s;
            shrubs.species_id[idx] = (s % 3) as u32;
            shrubs.cover[idx] = 2.0;
            shrubs.height[idx] = 50.0;
        }
        offset += count;
        shrubs.cell_offsets[i + 1] = offset;
    }

    assert!(shrubs.validate().is_ok());

    let refs = ReferenceData {
        shrub_eq_cf1: vec![1.5, 2.0, 2.5],
        shrub_eq_cf2: vec![1.1, 1.2, 1.3],
        ..Default::default()
    };

    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);

    for s in &shrubs.stems_per_acre {
        assert!(s.is_finite());
    }
}

#[test]
fn test_adv_unlinked_shrub_foreign_keys_sqlite_safety() {
    let _lock = TEST_MUTEX.lock().unwrap();

    let temp_dir = std::env::temp_dir();
    let db_path = temp_dir.join(format!("test_orphan_shrub_{}.db", std::process::id()));

    // Create sqlite db with orphan shrub pointing to non-existent PLOT_ID 999
    {
        let db_c = std::ffi::CString::new(db_path.to_str().unwrap()).unwrap();
        let mut handle = std::ptr::null_mut();
        unsafe {
            extern "C" {
                fn sqlite3_open(filename: *const std::os::raw::c_char, ppDb: *mut *mut std::ffi::c_void) -> i32;
                fn sqlite3_exec(
                    db: *mut std::ffi::c_void,
                    sql: *const std::os::raw::c_char,
                    callback: *const std::ffi::c_void,
                    arg: *mut std::ffi::c_void,
                    errmsg: *mut *mut std::os::raw::c_char,
                ) -> i32;
                fn sqlite3_close(db: *mut std::ffi::c_void) -> i32;
            }

            sqlite3_open(db_c.as_ptr(), &mut handle);
            let ddl = std::ffi::CString::new(
                "CREATE TABLE Plots_FCCS (PLOT_ID INTEGER PRIMARY KEY, BPS_CODE INTEGER, herb_cover REAL, herb_height REAL, PPT_3 REAL, NDVI_3 REAL);\
                 CREATE TABLE Shrubs_FCCS (PLOT_ID INTEGER, SPP_CODE TEXT, Cover REAL, Height REAL);\
                 INSERT INTO Plots_FCCS VALUES (1, 101, 20.0, 30.0, 400.0, 0.5);\
                 INSERT INTO Plots_FCCS VALUES (2, 101, 15.0, 25.0, 350.0, 0.4);\
                 INSERT INTO Shrubs_FCCS VALUES (1, 'ARTR2', 10.0, 50.0);\
                 INSERT INTO Shrubs_FCCS VALUES (2, 'PUTR2', 15.0, 60.0);\
                 INSERT INTO Shrubs_FCCS VALUES (999, 'ORPHAN', 5.0, 30.0);"
            ).unwrap();
            sqlite3_exec(handle, ddl.as_ptr(), std::ptr::null(), std::ptr::null_mut(), std::ptr::null_mut());
            sqlite3_close(handle);
        }
    }

    let loader = SQLiteDataLoader::new(db_path.to_str().unwrap());
    let result = loader.load_landscape();

    // Must safely return Err(DataLoaderError::Validation) instead of panicking or corrupting memory
    assert!(result.is_err(), "Unlinked shrub foreign key must be detected by CSR validation");
    match result.unwrap_err() {
        DataLoaderError::Validation(msg) => {
            assert!(
                msg.contains("CSR invariant 4 violated"),
                "Expected CSR invariant 4 failure due to orphan shrubs, got: {}",
                msg
            );
        }
        other => panic!("Expected DataLoaderError::Validation, got: {:?}", other),
    }

    let _ = std::fs::remove_file(db_path);
}

// =============================================================================
// 3. Extreme Climate & Raster Inputs (0 precip, drought, 5000mm, -elev, NoData)
// =============================================================================

#[test]
fn test_adv_extreme_climate_zero_precip() {
    let _lock = TEST_MUTEX.lock().unwrap();

    let prod = evaluate_herb_growth(1.0, 0.5, 0.5, 0.1, 0.0, 0.5);
    assert_eq!(prod, 0.0, "Zero precip must evaluate to 0.0 production");
}

#[test]
fn test_adv_extreme_climate_drought_negative_precip() {
    let _lock = TEST_MUTEX.lock().unwrap();

    let prod = evaluate_herb_growth(1.0, 0.5, 0.5, 0.1, -500.0, 0.5);
    assert_eq!(prod, 0.0, "Negative precip must evaluate to 0.0 production");

    // In SpatialRasterDataLoader: negative precip is clamped/masked to 0.0
    let mut loader = SpatialRasterDataLoader::new(1, 1, -9999.0);
    loader.set_band("precip", vec![-500.0]).unwrap();
    let (cells, _) = loader.load_landscape().unwrap();
    assert_eq!(cells.precip[0], 0.0, "Negative precipitation must be masked to 0.0");
}

#[test]
fn test_adv_extreme_climate_rainfall_5000mm() {
    let _lock = TEST_MUTEX.lock().unwrap();

    // 5000 mm rainfall
    let mut loader = SpatialRasterDataLoader::new(1, 1, -9999.0);
    loader.set_band("precip", vec![5000.0]).unwrap();
    loader.set_band("ndvi", vec![0.85]).unwrap();
    let (cells, _) = loader.load_landscape().unwrap();
    assert_eq!(cells.precip[0], 5000.0);

    let prod = evaluate_herb_growth(0.5, 0.2, 0.3, 0.05, cells.precip[0], cells.ndvi[0]);
    assert!(prod.is_finite(), "5000mm rainfall must produce finite production");
    assert!(prod > 0.0);

    let stoch = calc_herb_biomass_production(prod, 100.0, 1.96);
    assert!(stoch.is_finite());
}

#[test]
fn test_adv_raster_negative_elevation() {
    let _lock = TEST_MUTEX.lock().unwrap();

    // Death Valley (-86.0 m)
    let mut loader = SpatialRasterDataLoader::new(1, 1, -9999.0);
    loader.set_band("elevation", vec![-86.0]).unwrap();
    let (cells, _) = loader.load_landscape().unwrap();

    // Physical negative elevation is preserved
    assert_eq!(cells.elevation[0], -86.0);
    assert!(cells.elevation[0].is_finite());
}

#[test]
fn test_adv_raster_nan_nodata_sentinel_masking() {
    let _lock = TEST_MUTEX.lock().unwrap();

    let mut loader = SpatialRasterDataLoader::new(2, 3, -9999.0);
    let raw_ndvi = vec![0.5, -9999.0, f64::NAN, f64::INFINITY, f64::NEG_INFINITY, 0.8];
    loader.set_band("ndvi", raw_ndvi).unwrap();

    let (cells, _) = loader.load_landscape().unwrap();
    assert_eq!(cells.ndvi[0], 0.5);
    assert_eq!(cells.ndvi[1], 0.0); // NoData masked to 0.0
    assert_eq!(cells.ndvi[2], 0.0); // NaN masked to 0.0
    assert_eq!(cells.ndvi[3], 0.0); // +Inf masked to 0.0
    assert_eq!(cells.ndvi[4], 0.0); // -Inf masked to 0.0
    assert_eq!(cells.ndvi[5], 0.8);
}

// =============================================================================
// 4. Rayon Multithreading Concurrency & Determinism (1 vs N threads)
// =============================================================================

#[test]
fn test_adv_rayon_determinism_across_scales() {
    let _lock = TEST_MUTEX.lock().unwrap();

    // Test across odd, prime, threshold, and large scales
    let scales = [1, 17, 1023, 1024, 1025, 4099, 10_000];

    let pool_1 = rayon::ThreadPoolBuilder::new().num_threads(1).build().unwrap();
    let pool_n = rayon::ThreadPoolBuilder::new().num_threads(4).build().unwrap();

    for &n in &scales {
        let (mut cells_1, mut shrubs_1, refs) = make_test_landscape(n, 2);
        let (mut cells_n, mut shrubs_n, _) = make_test_landscape(n, 2);
        let mut fuels_1 = vec![0.0; n];
        let mut fuels_n = vec![0.0; n];

        // 1-thread execution
        pool_1.install(|| {
            batch_evaluate_biomass(&mut cells_1.view_mut(), &mut shrubs_1, &refs);
            batch_evaluate_fuels(&cells_1, &mut fuels_1, 0.65);
            batch_evaluate_succession(&mut cells_1.view_mut(), 1.2, 0.8);
            batch_evaluate_disturbance_graze(&mut cells_1.view_mut(), 22.5);
        });

        // 4-thread execution
        pool_n.install(|| {
            batch_evaluate_biomass(&mut cells_n.view_mut(), &mut shrubs_n, &refs);
            batch_evaluate_fuels(&cells_n, &mut fuels_n, 0.65);
            batch_evaluate_succession(&mut cells_n.view_mut(), 1.2, 0.8);
            batch_evaluate_disturbance_graze(&mut cells_n.view_mut(), 22.5);
        });

        // Assert BIT-IDENTICAL equality
        for i in 0..n {
            assert_eq!(
                cells_1.herb_biomass[i].to_bits(),
                cells_n.herb_biomass[i].to_bits(),
                "Herb biomass bit mismatch at scale N={}, index {}", n, i
            );
            assert_eq!(
                cells_1.herb_height[i].to_bits(),
                cells_n.herb_height[i].to_bits(),
                "Herb height bit mismatch at scale N={}, index {}", n, i
            );
            assert_eq!(
                cells_1.shrub_height[i].to_bits(),
                cells_n.shrub_height[i].to_bits(),
                "Shrub height bit mismatch at scale N={}, index {}", n, i
            );
            assert_eq!(
                cells_1.shrub_cover[i].to_bits(),
                cells_n.shrub_cover[i].to_bits(),
                "Shrub cover bit mismatch at scale N={}, index {}", n, i
            );
            assert_eq!(
                fuels_1[i].to_bits(),
                fuels_n[i].to_bits(),
                "Fuels bit mismatch at scale N={}, index {}", n, i
            );
        }

        for s in 0..(n * 2) {
            assert_eq!(
                shrubs_1.stems_per_acre[s].to_bits(),
                shrubs_n.stems_per_acre[s].to_bits(),
                "Shrub stems bit mismatch at scale N={}, shrub index {}", n, s
            );
        }
    }
}

// =============================================================================
// 5. Memory Allocation Verification (Zero Heap Allocation in Hot Paths)
// =============================================================================

#[test]
fn test_adv_zero_heap_allocation_hot_paths() {
    let _lock = TEST_MUTEX.lock().unwrap();

    const N: usize = 20_000;
    const M_PER_CELL: usize = 3;
    let (mut cells, mut shrubs, refs) = make_test_landscape(N, M_PER_CELL);
    let mut fuels = vec![0.0; N];

    let pool = rayon::ThreadPoolBuilder::new().num_threads(4).build().unwrap();

    // Warm up threadpool to ensure Rayon internal worker structures are ready
    pool.install(|| {
        let mut d_cells = LandscapeCellBuffers::new(32);
        let mut d_shrubs = LandscapeShrubBuffers::new(32, 32);
        let mut d_fuels = vec![0.0; 32];
        d_shrubs.cell_offsets[32] = 32;
        batch_evaluate_biomass(&mut d_cells.view_mut(), &mut d_shrubs, &refs);
        batch_evaluate_fuels(&d_cells, &mut d_fuels, 0.5);
        batch_evaluate_succession(&mut d_cells.view_mut(), 1.0, 1.0);
        batch_evaluate_disturbance_graze(&mut d_cells.view_mut(), 10.0);
    });

    // Zero-allocation assertion during hot path execution
    pool.install(|| {
        ALLOC_COUNT.store(0, Ordering::SeqCst);
        TRACKING_ACTIVE.store(true, Ordering::SeqCst);

        // Run 10 annual simulation steps
        for _ in 0..10 {
            batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
            batch_evaluate_fuels(&cells, &mut fuels, 0.7);
            batch_evaluate_succession(&mut cells.view_mut(), 1.0, 0.5);
            batch_evaluate_disturbance_graze(&mut cells.view_mut(), 20.0);
        }

        TRACKING_ACTIVE.store(false, Ordering::SeqCst);
    });

    let allocs = ALLOC_COUNT.load(Ordering::SeqCst);
    assert_eq!(
        allocs, 0,
        "Batch hot path MUST perform ZERO heap allocations across 10 simulation cycles! Found: {}",
        allocs
    );
}
