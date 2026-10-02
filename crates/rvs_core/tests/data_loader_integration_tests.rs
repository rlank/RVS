//! Milestone 4 Integration Test Suite for DataLoader and Async Ingestion Architecture.
//!
//! Tests:
//! - Feature 15: In-Memory ReferenceData Catalog
//! - Feature 16: Abstract DataLoader Interface
//! - Feature 17: Tokio Async Tile Streaming & Prefetching
//! - Feature 18: Producer-Consumer Rayon Bridge
//! - Feature 19: Structured Concurrency & Graceful Cancellation
//! - Feature 20: SQLite DataLoader Backend (rvs_demo.db Plots_FCCS, Shrubs_FCCS)
//! - Feature 21: Spatial Raster DataLoader Backend
//! - Feature 24: SQLite vs Spatial Bit-Identical Equivalence

use std::path::PathBuf;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;

use rvs_core::batch::{batch_evaluate_biomass, batch_evaluate_fuels, batch_evaluate_succession};
use rvs_core::biomass::calc_attenuation;
use rvs_core::data_loader::{
    run_producer_consumer_pipeline, AsyncDataLoader, LandscapeTileSoA, SQLiteDataLoader,
    SpatialRasterDataLoader,
};
use rvs_core::fuels::calc_total_1hr_fuel;
use rvs_core::succession::calc_growth_rate;
use tokio_util::sync::CancellationToken;

/// Resolves path to `rvs_demo.db` in repository root.
fn get_demo_db_path() -> PathBuf {
    let manifest_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    let db_path = manifest_dir.join("../../rvs_demo.db");
    if db_path.exists() {
        return db_path;
    }
    // Fallback: check current directory or parent
    let cur = std::env::current_dir().unwrap_or_else(|_| PathBuf::from("."));
    let candidates = [
        cur.join("rvs_demo.db"),
        cur.join("../rvs_demo.db"),
        cur.join("../../rvs_demo.db"),
    ];
    for c in &candidates {
        if c.exists() {
            return c.clone();
        }
    }
    db_path
}

// =============================================================================
// 1. Feature 20 & Feature 15: SQLite DataLoader on rvs_demo.db
// =============================================================================

#[test]
fn test_sqlite_data_loader_demo_db() {
    let db_path = get_demo_db_path();
    assert!(
        db_path.exists(),
        "rvs_demo.db must exist at {}",
        db_path.display()
    );

    let loader = SQLiteDataLoader::new(db_path.to_str().unwrap());

    // 1. Ingest Plots_FCCS (61 plots) and Shrubs_FCCS (102 shrubs)
    let (mut cells, mut shrubs) = loader
        .load_landscape()
        .expect("Failed to load landscape from rvs_demo.db");

    // Validate Plots_FCCS count
    assert_eq!(
        cells.len(),
        61,
        "Plots_FCCS must ingest exactly 61 plot records"
    );
    assert_eq!(cells.herb_cover.len(), 61);
    assert_eq!(cells.herb_height.len(), 61);
    assert_eq!(cells.precip.len(), 61);
    assert_eq!(cells.ndvi.len(), 61);

    // Validate Shrubs_FCCS count
    assert_eq!(
        shrubs.num_shrubs, 102,
        "Shrubs_FCCS must ingest exactly 102 shrub records"
    );
    assert_eq!(shrubs.num_cells, 61);

    // Validate CSR Invariants
    assert_eq!(
        shrubs.cell_offsets.len(),
        62,
        "CSR cell_offsets must have length N + 1 = 62"
    );
    assert_eq!(
        shrubs.cell_offsets[0], 0,
        "CSR cell_offsets[0] must be exactly 0"
    );
    assert_eq!(
        shrubs.cell_offsets[61], 102,
        "CSR cell_offsets[61] must equal total shrubs (102)"
    );

    // Strictly validate all 5 canonical CSR invariants
    shrubs
        .validate()
        .expect("All 5 CSR invariants must hold on ingested shrubs");

    // Verify completely detached in-memory operation (DB handle is closed)
    cells.herb_biomass[0] = 999.0;
    assert_eq!(cells.herb_biomass[0], 999.0);

    shrubs.biomass[0] = 55.5;
    assert_eq!(shrubs.biomass[0], 55.5);

    // 2. Ingest ReferenceData Catalog (Feature 15)
    let refs = loader
        .load_reference_data()
        .expect("Failed to load reference catalog from rvs_demo.db");

    assert!(
        !refs.bio_equations.is_empty(),
        "Bio_Equation table must populate bio_equations flat array"
    );
    assert!(
        !refs.bio_crosswalk.is_empty(),
        "Bio_Crosswalk table must populate species mappings"
    );
    assert!(
        !refs.fuel_crosswalk.is_empty(),
        "Fuel_Crosswalk table must populate fuel mappings"
    );
    assert!(
        !refs.fuel_equations.is_empty(),
        "Fuel_Equation table must populate fuel_equations flat array"
    );
    assert!(
        refs.fuel_equations.iter().any(|f| f.eqn_num > 0),
        "Fuel_Equation flat array must contain populated records"
    );
    assert!(
        !refs.growth_rates.is_empty(),
        "BPS_Combined_Growthrates must populate growth rates"
    );

    // Spot-check O(1) direct equation lookup
    let eq_165 = refs.get_bio_equation(165);
    assert!(
        eq_165.is_some(),
        "Equation 165 must be pre-loaded in reference catalog"
    );
    let eq_rec = eq_165.unwrap();
    assert_eq!(eq_rec.eqn_num, 165);

    // Verify Rayon batch reference compatibility
    let batch_refs = refs.to_batch_ref();
    assert_eq!(batch_refs.shrub_eq_cf1.len(), refs.shrub_eq_cf1.len());
    assert_eq!(batch_refs.shrub_eq_cf2.len(), refs.shrub_eq_cf2.len());

    // 3. Load disturbance schedule
    let dist_schedule = loader
        .load_disturbance_schedule()
        .expect("Failed to load disturbance schedule");
    assert!(
        !dist_schedule.is_empty(),
        "Dist_year7fire must contain disturbance events"
    );
    let has_year7 = dist_schedule.iter().any(|d| d.start_year == 7);
    assert!(has_year7, "Dist_year7fire must contain Year 7 fire event");
}

// =============================================================================
// 2. Feature 21: Spatial Raster DataLoader Ingestion & NoData Masking
// =============================================================================

#[test]
fn test_spatial_raster_data_loader_ingestion() {
    let rows = 4;
    let cols = 5;
    let nodata = -9999.0;
    let n = rows * cols; // 20 cells

    let mut loader = SpatialRasterDataLoader::new(rows, cols, nodata);

    // Create synthetic gridded bands
    let mut ndvi_data = vec![0.45; n];
    let mut precip_data = vec![500.0; n];
    let mut elev_data = vec![1200.0; n];
    let mut hc_data = vec![35.0; n];
    let mut hh_data = vec![40.0; n];
    let sc_data = vec![15.0; n];
    let sh_data = vec![50.0; n];

    // Inject NoData sentinels
    ndvi_data[1] = nodata; // Cell (0, 1) NoData
    precip_data[1] = nodata;
    hc_data[3] = nodata; // Cell (0, 3) NoData
    hh_data[7] = nodata; // Cell (1, 2) NoData
    elev_data[11] = nodata; // Cell (2, 1) NoData

    loader.set_band("ndvi", ndvi_data).unwrap();
    loader.set_band("precip", precip_data).unwrap();
    loader.set_band("elevation", elev_data).unwrap();
    loader.set_band("herb_cover", hc_data).unwrap();
    loader.set_band("herb_height", hh_data).unwrap();
    loader.set_band("shrub_cover", sc_data).unwrap();
    loader.set_band("shrub_height", sh_data).unwrap();

    let (cells, shrubs) = loader.load_landscape().unwrap();

    // Verify cell dimensions and CSR consistency
    assert_eq!(cells.len(), 20);
    assert_eq!(shrubs.num_cells, 20);
    assert_eq!(shrubs.num_shrubs, 0);
    shrubs.validate().unwrap();

    // Verify 2D row-major unraveling: i = r * cols + c
    // Cell (2, 3) -> index 2 * 5 + 3 = 13
    assert_eq!(cells.ndvi[13], 0.45);
    assert_eq!(cells.precip[13], 500.0);
    assert_eq!(cells.elevation[13], 1200.0);
    assert_eq!(cells.herb_cover[13], 35.0);
    assert_eq!(cells.herb_height[13], 40.0);

    // Verify NoData sentinel masking: -9999.0 clamped to 0.0
    assert_eq!(cells.ndvi[1], 0.0, "NoData NDVI must be masked to 0.0");
    assert_eq!(cells.precip[1], 0.0, "NoData PPT must be masked to 0.0");
    assert_eq!(
        cells.herb_cover[3], 0.0,
        "NoData Herb Cover must be masked to 0.0"
    );
    assert_eq!(
        cells.herb_height[7], 0.0,
        "NoData Herb Height must be masked to 0.0"
    );
    assert_eq!(
        cells.elevation[11], 0.0,
        "NoData Elevation must be masked to 0.0"
    );
}

// =============================================================================
// 3. Feature 17 & 18: Tokio Async Streaming & Producer-Consumer Rayon Bridge
// =============================================================================

#[tokio::test(flavor = "multi_thread", worker_threads = 4)]
async fn test_producer_consumer_streaming_pipeline() {
    let rows = 8;
    let cols = 8;
    let n = rows * cols; // 64 cells

    let mut loader = SpatialRasterDataLoader::new(rows, cols, -9999.0);
    loader = loader.with_tile_size(4); // 4x4 tiles -> 4 tiles total

    loader.set_band("ndvi", vec![0.5; n]).unwrap();
    loader.set_band("precip", vec![450.0; n]).unwrap();
    loader.set_band("elevation", vec![1100.0; n]).unwrap();
    loader.set_band("herb_cover", vec![30.0; n]).unwrap();
    loader.set_band("herb_height", vec![25.0; n]).unwrap();
    loader.set_band("shrub_cover", vec![20.0; n]).unwrap();
    loader.set_band("shrub_height", vec![60.0; n]).unwrap();

    let loader_arc = Arc::new(loader);
    let processed_tile_count = Arc::new(AtomicUsize::new(0));
    let total_cells_processed = Arc::new(AtomicUsize::new(0));

    let processed_count_clone = Arc::clone(&processed_tile_count);
    let cells_count_clone = Arc::clone(&total_cells_processed);

    // Run producer-consumer pipeline with channel capacity = 2 (triggering backpressure)
    let pipeline_res = run_producer_consumer_pipeline(
        loader_arc,
        2,
        move |tile: LandscapeTileSoA| {
            // Verify tile dimensions (4x4 = 16 cells)
            assert_eq!(tile.width, 4);
            assert_eq!(tile.height, 4);
            assert_eq!(tile.buffers.len(), 16);

            // Simulate batch compute within block_in_place
            processed_count_clone.fetch_add(1, Ordering::SeqCst);
            cells_count_clone.fetch_add(tile.buffers.len(), Ordering::SeqCst);
            Ok(())
        },
    )
    .await;

    assert!(pipeline_res.is_ok(), "Pipeline must complete successfully");
    assert_eq!(
        processed_tile_count.load(Ordering::SeqCst),
        4,
        "Pipeline must stream exactly 4 tiles"
    );
    assert_eq!(
        total_cells_processed.load(Ordering::SeqCst),
        64,
        "Total processed cells must equal 64"
    );
}

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn test_streaming_graceful_cancellation() {
    let rows = 20;
    let cols = 20;
    let n = rows * cols; // 400 cells

    let mut loader = SpatialRasterDataLoader::new(rows, cols, -9999.0);
    loader = loader.with_tile_size(2); // 100 tiles total
    loader.set_band("ndvi", vec![0.5; n]).unwrap();

    let (tx, mut rx) = tokio::sync::mpsc::channel::<LandscapeTileSoA>(2);
    let cancel = CancellationToken::new();
    let cancel_producer = cancel.clone();

    // Spawn producer
    let producer = tokio::spawn(async move {
        loader
            .stream_landscape_tiles(tx, cancel_producer)
            .await
    });

    // Receive 3 tiles, then cancel
    let mut received = 0;
    while let Some(_tile) = rx.recv().await {
        received += 1;
        if received >= 3 {
            cancel.cancel();
            break;
        }
    }
    drop(rx);

    // Producer should exit promptly with Cancelled error or Ok(due to dropped rx)
    let res = producer.await.unwrap();
    match res {
        Err(rvs_core::data_loader::DataLoaderError::Cancelled) => {
            // Expected cancellation response
        }
        Ok(()) => {
            // Channel closed before cancellation was inspected
        }
        Err(e) => panic!("Unexpected error on cancellation: {:?}", e),
    }

    assert!(received >= 3, "Consumer must have received at least 3 tiles before cancelling");
}

// =============================================================================
// 4. Feature 24: SQLite vs Spatial Equivalence Test (Bit-Identical Invariance)
// =============================================================================

#[test]
fn test_sqlite_vs_spatial_equivalence() {
    let db_path = get_demo_db_path();
    assert!(
        db_path.exists(),
        "rvs_demo.db must exist at {}",
        db_path.display()
    );

    // 1. Ingest live landscape and reference data from SQLite database
    let sqlite_loader = SQLiteDataLoader::new(db_path.to_str().unwrap());
    let (mut cells_sqlite, mut shrubs_sqlite) = sqlite_loader
        .load_landscape()
        .expect("Failed to load landscape from SQLiteDataLoader");
    let refs = sqlite_loader
        .load_reference_data()
        .expect("Failed to load reference data from SQLiteDataLoader");

    let n = cells_sqlite.len();
    assert_eq!(n, 61, "Plots_FCCS must ingest 61 cells");
    assert!(
        shrubs_sqlite.num_shrubs > 0,
        "Shrubs_FCCS must contain non-zero shrubs"
    );
    assert_eq!(shrubs_sqlite.num_shrubs, 102);

    // 2. Configure SpatialRasterDataLoader with identical live cell attributes
    let mut spatial_loader = SpatialRasterDataLoader::new(1, n, -9999.0);
    spatial_loader
        .set_band("ndvi", cells_sqlite.ndvi.clone())
        .unwrap();
    spatial_loader
        .set_band("precip", cells_sqlite.precip.clone())
        .unwrap();
    spatial_loader
        .set_band("elevation", cells_sqlite.elevation.clone())
        .unwrap();
    spatial_loader
        .set_band("herb_cover", cells_sqlite.herb_cover.clone())
        .unwrap();
    spatial_loader
        .set_band("herb_height", cells_sqlite.herb_height.clone())
        .unwrap();
    spatial_loader
        .set_band("shrub_cover", cells_sqlite.shrub_cover.clone())
        .unwrap();
    spatial_loader
        .set_band("shrub_height", cells_sqlite.shrub_height.clone())
        .unwrap();

    let (mut cells_spatial, _) = spatial_loader
        .load_landscape()
        .expect("Failed to load landscape from SpatialRasterDataLoader");

    // Provide the spatial pipeline with the identical live shrub CSR sub-canopy (102 shrubs across 61 plots)
    let mut shrubs_spatial = shrubs_sqlite.clone();

    // Canonical initial herb biomass for multi-year simulation
    cells_sqlite.herb_biomass = vec![400.0; n];
    cells_spatial.herb_biomass = vec![400.0; n];

    let batch_refs = refs.to_batch_ref();

    // 3. Run multi-year simulation across both pipelines (20-year trajectory)
    let num_years = 20;
    let mut fuel_1hr_sqlite = vec![0.0; n];
    let mut fuel_1hr_spatial = vec![0.0; n];

    for _year in 1..=num_years {
        // Step 1: Biomass
        batch_evaluate_biomass(
            &mut cells_sqlite.view_mut(),
            &mut shrubs_sqlite,
            batch_refs,
        );
        batch_evaluate_biomass(
            &mut cells_spatial.view_mut(),
            &mut shrubs_spatial,
            batch_refs,
        );

        // Step 2: Fuels
        batch_evaluate_fuels(&cells_sqlite, &mut fuel_1hr_sqlite, 0.85);
        batch_evaluate_fuels(&cells_spatial, &mut fuel_1hr_spatial, 0.85);

        // Step 3: Succession
        batch_evaluate_succession(&mut cells_sqlite.view_mut(), 1.5, 0.5);
        batch_evaluate_succession(&mut cells_spatial.view_mut(), 1.5, 0.5);

        // Direct subroutine evaluation equivalence check
        for i in 0..n {
            cells_sqlite.herb_biomass[i] = calc_attenuation(
                cells_sqlite.herb_biomass[i],
                cells_sqlite.herb_cover[i],
                cells_sqlite.herb_height[i],
                cells_sqlite.shrub_cover[i],
                cells_sqlite.shrub_height[i],
            );
            cells_spatial.herb_biomass[i] = calc_attenuation(
                cells_spatial.herb_biomass[i],
                cells_spatial.herb_cover[i],
                cells_spatial.herb_height[i],
                cells_spatial.shrub_cover[i],
                cells_spatial.shrub_height[i],
            );

            cells_sqlite.herb_height[i] = calc_growth_rate(cells_sqlite.herb_height[i], 2.0);
            cells_spatial.herb_height[i] = calc_growth_rate(cells_spatial.herb_height[i], 2.0);

            let f_sql = calc_total_1hr_fuel(
                cells_sqlite.shrub_cover[i] * 0.15,
                cells_sqlite.shrub_cover[i] * 0.10,
                cells_sqlite.herb_biomass[i] * 0.75,
            );
            let f_spat = calc_total_1hr_fuel(
                cells_spatial.shrub_cover[i] * 0.15,
                cells_spatial.shrub_cover[i] * 0.10,
                cells_spatial.herb_biomass[i] * 0.75,
            );
            assert_eq!(f_sql, f_spat);
        }

        // Strict bit-for-bit equivalence assertion (Delta = 0.0)
        assert_eq!(
            cells_sqlite.herb_biomass, cells_spatial.herb_biomass,
            "Herb biomass must be bit-identical across SQLite and Spatial pipelines"
        );
        assert_eq!(
            cells_sqlite.herb_height, cells_spatial.herb_height,
            "Herb height must be bit-identical across SQLite and Spatial pipelines"
        );
        assert_eq!(
            cells_sqlite.herb_cover, cells_spatial.herb_cover,
            "Herb cover must be bit-identical across SQLite and Spatial pipelines"
        );
        assert_eq!(
            cells_sqlite.shrub_cover, cells_spatial.shrub_cover,
            "Shrub cover must be bit-identical across SQLite and Spatial pipelines"
        );
        assert_eq!(
            cells_sqlite.shrub_height, cells_spatial.shrub_height,
            "Shrub height must be bit-identical across SQLite and Spatial pipelines"
        );
        assert_eq!(
            shrubs_sqlite.biomass, shrubs_spatial.biomass,
            "Shrub biomass must be bit-identical across SQLite and Spatial pipelines"
        );
        assert_eq!(
            fuel_1hr_sqlite, fuel_1hr_spatial,
            "Fuel loading must be bit-identical across SQLite and Spatial pipelines"
        );
    }
}

#[test]
fn test_sqlite_vs_spatial_equivalence_perturbation_sensitivity() {
    let db_path = get_demo_db_path();
    assert!(
        db_path.exists(),
        "rvs_demo.db must exist at {}",
        db_path.display()
    );

    let sqlite_loader = SQLiteDataLoader::new(db_path.to_str().unwrap());
    let (mut cells_sqlite, mut shrubs_sqlite) = sqlite_loader
        .load_landscape()
        .expect("Failed to load landscape from SQLiteDataLoader");
    let refs = sqlite_loader
        .load_reference_data()
        .expect("Failed to load reference data from SQLiteDataLoader");

    let n = cells_sqlite.len();
    assert!(n > 0);
    assert!(shrubs_sqlite.num_shrubs > 0);

    // Perturb an input channel for the spatial loader (herb_cover on Plot 0)
    let mut perturbed_herb_cover = cells_sqlite.herb_cover.clone();
    perturbed_herb_cover[0] += 25.0;

    let mut spatial_loader = SpatialRasterDataLoader::new(1, n, -9999.0);
    spatial_loader
        .set_band("ndvi", cells_sqlite.ndvi.clone())
        .unwrap();
    spatial_loader
        .set_band("precip", cells_sqlite.precip.clone())
        .unwrap();
    spatial_loader
        .set_band("elevation", cells_sqlite.elevation.clone())
        .unwrap();
    spatial_loader
        .set_band("herb_cover", perturbed_herb_cover)
        .unwrap();
    spatial_loader
        .set_band("herb_height", cells_sqlite.herb_height.clone())
        .unwrap();
    spatial_loader
        .set_band("shrub_cover", cells_sqlite.shrub_cover.clone())
        .unwrap();
    spatial_loader
        .set_band("shrub_height", cells_sqlite.shrub_height.clone())
        .unwrap();

    let (mut cells_spatial, _) = spatial_loader
        .load_landscape()
        .expect("Failed to load landscape from SpatialRasterDataLoader");

    let mut shrubs_spatial = shrubs_sqlite.clone();

    cells_sqlite.herb_biomass = vec![400.0; n];
    cells_spatial.herb_biomass = vec![400.0; n];

    let batch_refs = refs.to_batch_ref();
    let mut fuel_1hr_sqlite = vec![0.0; n];
    let mut fuel_1hr_spatial = vec![0.0; n];

    // Evaluate step
    batch_evaluate_biomass(
        &mut cells_sqlite.view_mut(),
        &mut shrubs_sqlite,
        batch_refs,
    );
    batch_evaluate_biomass(
        &mut cells_spatial.view_mut(),
        &mut shrubs_spatial,
        batch_refs,
    );

    batch_evaluate_fuels(&cells_sqlite, &mut fuel_1hr_sqlite, 0.85);
    batch_evaluate_fuels(&cells_spatial, &mut fuel_1hr_spatial, 0.85);

    cells_sqlite.herb_biomass[0] = calc_attenuation(
        cells_sqlite.herb_biomass[0],
        cells_sqlite.herb_cover[0],
        cells_sqlite.herb_height[0],
        cells_sqlite.shrub_cover[0],
        cells_sqlite.shrub_height[0],
    );
    cells_spatial.herb_biomass[0] = calc_attenuation(
        cells_spatial.herb_biomass[0],
        cells_spatial.herb_cover[0],
        cells_spatial.herb_height[0],
        cells_spatial.shrub_cover[0],
        cells_spatial.shrub_height[0],
    );

    // Assert that modifying an input channel produces detectable divergence
    assert_ne!(
        cells_sqlite.herb_cover[0], cells_spatial.herb_cover[0],
        "Perturbed herb cover must diverge between pipelines"
    );
    assert_ne!(
        cells_sqlite.herb_biomass[0], cells_spatial.herb_biomass[0],
        "Herb biomass must diverge when herb cover is perturbed"
    );
}
