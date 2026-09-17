//! Empirical stress test suite challenging memory safety, slice alignment,
//! species indexing, and concurrency robustness of:
//! - Structure-of-Arrays engine (`rvs_core::soa`)
//! - Rayon parallel batch execution kernels (`rvs_core::batch`)
//! - Tokio asynchronous streaming pipeline (`rvs_core::data_loader`)

use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::Duration;
use async_trait::async_trait;
use tokio::sync::mpsc;
use tokio_util::sync::CancellationToken;

use rvs_core::soa::{LandscapeCellBuffers, LandscapeShrubBuffers};
use rvs_core::batch::{
    batch_evaluate_biomass,
    batch_evaluate_fuels,
    batch_evaluate_succession,
    batch_evaluate_disturbance_graze,
    ReferenceData,
};
use rvs_core::data_loader::{
    AsyncDataLoader, DataLoaderError, LandscapeTileSoA, run_producer_consumer_pipeline,
};

// =========================================================================
// 1. Empty Slice Tests (num_cells = 0, num_shrubs = 0)
// =========================================================================

#[test]
fn test_empty_slices_biomass_zero_iteration() {
    let mut cells = LandscapeCellBuffers::new(0);
    let mut shrubs = LandscapeShrubBuffers::new(0, 0);
    let refs = ReferenceData::default();

    assert!(cells.is_empty());
    assert_eq!(cells.len(), 0);
    assert_eq!(shrubs.num_cells, 0);
    assert_eq!(shrubs.num_shrubs, 0);

    // Must execute cleanly without panics on empty slices
    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);

    assert_eq!(cells.herb_biomass.len(), 0);
    assert_eq!(shrubs.stems_per_acre.len(), 0);
}

#[test]
fn test_empty_slices_fuels_zero_iteration() {
    let cells = LandscapeCellBuffers::new(0);
    let mut out_1hr_fuel: Vec<f64> = Vec::new();

    batch_evaluate_fuels(&cells, &mut out_1hr_fuel, 0.5);

    assert_eq!(out_1hr_fuel.len(), 0);
}

#[test]
fn test_empty_slices_succession_zero_iteration() {
    let mut cells = LandscapeCellBuffers::new(0);
    batch_evaluate_succession(&mut cells.view_mut(), 1.5, 2.5);
    assert_eq!(cells.shrub_height.len(), 0);
    assert_eq!(cells.shrub_cover.len(), 0);
}

#[test]
fn test_empty_slices_disturbance_zero_iteration() {
    let mut cells = LandscapeCellBuffers::new(0);
    batch_evaluate_disturbance_graze(&mut cells.view_mut(), 30.0);
    assert_eq!(cells.herb_biomass.len(), 0);
    assert_eq!(cells.herb_height.len(), 0);
}

#[test]
fn test_empty_slices_csr_shrub_bounds_check() {
    let shrubs = LandscapeShrubBuffers::new(0, 0);
    assert_eq!(shrubs.cell_offsets.len(), 1);

    // Accessing cell 0 when num_cells == 0 must panic
    let result = std::panic::catch_unwind(|| {
        shrubs.shrub_range_for_cell(0);
    });
    assert!(result.is_err(), "Accessing out-of-bounds cell 0 on empty shrub buffer must panic");
}

// =========================================================================
// 2. Buffer Length Mismatch Assertions
// =========================================================================

#[test]
fn test_buffer_length_mismatch_cell_shrub_cover() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();

    // Corrupt one slice length
    cells.shrub_cover.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched shrub_cover buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_cell_herb_biomass() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();

    cells.herb_biomass.push(10.0); // 6 elements instead of 5

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched herb_biomass buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_shrub_species_id() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();

    shrubs.species_id.pop(); // 4 elements instead of 5

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched species_id buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_shrub_stems_per_acre() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();

    shrubs.stems_per_acre.push(0.0); // 6 elements instead of 5

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched stems_per_acre buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_succession() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.shrub_height.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_succession(&mut cells.view_mut(), 1.0, 1.0);
    }));
    assert!(result.is_err(), "Mismatched succession shrub_height must panic");
}

#[test]
fn test_buffer_length_mismatch_disturbance() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.herb_biomass.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_disturbance_graze(&mut cells.view_mut(), 20.0);
    }));
    assert!(result.is_err(), "Mismatched disturbance herb_biomass must panic");
}

#[test]
fn test_buffer_length_mismatch_fuels_output_slice() {
    let cells = LandscapeCellBuffers::new(4);
    let mut out_1hr_fuel = vec![0.0; 2]; // 2 instead of 4

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_fuels(&cells, &mut out_1hr_fuel, 0.5);
    }));
    assert!(result.is_err(), "Mismatched out_1hr_fuel length must panic");
}

#[test]
fn test_buffer_length_mismatch_fuels_herb_biomass_short() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.herb_biomass.pop(); // 3 elements instead of 4
    let mut out_1hr_fuel = vec![0.0; 4];

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_fuels(&cells, &mut out_1hr_fuel, 0.5);
    }));
    assert!(result.is_err(), "Underflow herb_biomass buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_fuels_herb_biomass_long() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.herb_biomass.push(10.0); // 5 elements instead of 4
    let mut out_1hr_fuel = vec![0.0; 4];

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_fuels(&cells, &mut out_1hr_fuel, 0.5);
    }));
    assert!(result.is_err(), "Overflow herb_biomass buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_biomass_cell_shrub_height() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();
    cells.shrub_height.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched shrub_height buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_biomass_cell_herb_cover() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();
    cells.herb_cover.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched herb_cover buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_biomass_cell_herb_height() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();
    cells.herb_height.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched herb_height buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_biomass_shrub_height() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();
    shrubs.height.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched shrubs.height buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_biomass_shrub_cover() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();
    shrubs.cover.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched shrubs.cover buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_biomass_shrub_biomass() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();
    shrubs.biomass.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched shrubs.biomass buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_biomass_shrub_num_cells() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(3, 5); // 3 cells vs 5 cells
    let refs = ReferenceData::default();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched shrubs.num_cells vs cells.num_cells must panic");
}

#[test]
fn test_buffer_length_mismatch_biomass_shrub_cell_offsets() {
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 5);
    let refs = ReferenceData::default();
    shrubs.cell_offsets.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(result.is_err(), "Mismatched shrubs.cell_offsets buffer must panic before zip");
}

#[test]
fn test_buffer_length_mismatch_succession_shrub_cover() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.shrub_cover.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_succession(&mut cells.view_mut(), 1.0, 1.0);
    }));
    assert!(result.is_err(), "Mismatched succession shrub_cover must panic");
}

#[test]
fn test_buffer_length_mismatch_disturbance_herb_height() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.herb_height.pop();

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_disturbance_graze(&mut cells.view_mut(), 20.0);
    }));
    assert!(result.is_err(), "Mismatched disturbance herb_height must panic");
}

#[test]
fn test_shrub_buffers_default_preserves_csr_invariant() {
    let shrubs = LandscapeShrubBuffers::default();
    assert_eq!(shrubs.num_cells, 0);
    assert_eq!(shrubs.num_shrubs, 0);
    assert_eq!(
        shrubs.cell_offsets.len(),
        1,
        "Default must satisfy cell_offsets.len() == num_cells + 1"
    );
    assert_eq!(shrubs.cell_offsets[0], 0);
    assert_eq!(shrubs.species_id.len(), 0);
    assert_eq!(shrubs.cover.len(), 0);
    assert_eq!(shrubs.height.len(), 0);
    assert_eq!(shrubs.biomass.len(), 0);
    assert_eq!(shrubs.stems_per_acre.len(), 0);
}

#[test]
fn test_csr_shrub_range_corrupted_offsets_length() {
    let mut shrubs = LandscapeShrubBuffers::new(3, 3);
    shrubs.cell_offsets.pop(); // Corrupted length != num_cells + 1

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        shrubs.shrub_range_for_cell(0);
    }));
    assert!(result.is_err(), "Corrupted cell_offsets length must panic");
}

#[test]
fn test_csr_shrub_range_invalid_start_greater_than_end() {
    let mut shrubs = LandscapeShrubBuffers::new(2, 4);
    shrubs.cell_offsets = vec![0, 3, 2]; // start (3) > end (2) for cell 1

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        shrubs.shrub_range_for_cell(1);
    }));
    assert!(result.is_err(), "CSR start offset > end offset must panic");
}

#[test]
fn test_csr_shrub_range_end_exceeds_num_shrubs() {
    let mut shrubs = LandscapeShrubBuffers::new(2, 4);
    shrubs.cell_offsets = vec![0, 2, 5]; // end (5) > num_shrubs (4) for cell 1

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        shrubs.shrub_range_for_cell(1);
    }));
    assert!(result.is_err(), "CSR end offset > num_shrubs must panic");
}

// =========================================================================
// 3. Species ID Indexing & Extreme Value Fallback Tests
// =========================================================================

#[test]
fn test_species_id_extreme_and_fallback_indexing() {
    let mut cells = LandscapeCellBuffers::new(3);
    cells.herb_biomass = vec![100.0, 200.0, 300.0];
    cells.herb_cover = vec![20.0, 30.0, 40.0];
    cells.herb_height = vec![30.0, 40.0, 50.0];
    cells.shrub_cover = vec![10.0, 15.0, 20.0];
    cells.shrub_height = vec![50.0, 60.0, 70.0];

    let mut shrubs = LandscapeShrubBuffers::new(3, 4);
    shrubs.height = vec![40.0, 60.0, 80.0, 100.0];
    shrubs.cover = vec![10.0, 20.0, 30.0, 40.0];
    shrubs.species_id = vec![
        0,          // Valid index
        9999,       // Out of bounds extreme ID
        u32::MAX,   // Maximum u32 ID
        1,          // Valid index
    ];

    let mut refs = ReferenceData::default();
    refs.shrub_eq_cf1 = vec![2.0, 3.0];
    refs.shrub_eq_cf2 = vec![1.2, 1.3];

    // Must not panic on invalid or extreme species IDs
    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);

    // Species 0 used cf1=2.0, cf2=1.2
    assert!(shrubs.stems_per_acre[0] > 0.0);
    // Species 9999 used fallback cf1=1.0, cf2=1.5
    assert!(shrubs.stems_per_acre[1] > 0.0);
    // Species u32::MAX used fallback cf1=1.0, cf2=1.5
    assert!(shrubs.stems_per_acre[2] > 0.0);
    // Species 1 used cf1=3.0, cf2=1.3
    assert!(shrubs.stems_per_acre[3] > 0.0);

    for stems in &shrubs.stems_per_acre {
        assert!(stems.is_finite(), "Calculated stems per acre must be finite");
    }
}

// =========================================================================
// 4. Async DataLoader Concurrency & Pipeline Lifecycle Tests
// =========================================================================

struct MockStreamingDataLoader {
    tile_count: usize,
    cells_per_tile: usize,
    producer_cancelled: Arc<AtomicBool>,
    produced_count: Arc<AtomicUsize>,
    fail_at_tile: Option<usize>,
}

#[async_trait]
impl AsyncDataLoader for MockStreamingDataLoader {
    async fn stream_landscape_tiles(
        &self,
        sender: mpsc::Sender<LandscapeTileSoA>,
        cancel: CancellationToken,
    ) -> Result<(), DataLoaderError> {
        for t in 0..self.tile_count {
            if cancel.is_cancelled() {
                self.producer_cancelled.store(true, Ordering::SeqCst);
                return Err(DataLoaderError::Cancelled);
            }

            if let Some(fail_idx) = self.fail_at_tile {
                if t == fail_idx {
                    return Err(DataLoaderError::Validation(format!("Simulated failure at tile {}", t)));
                }
            }

            let mut buf = LandscapeCellBuffers::new(self.cells_per_tile);
            for i in 0..self.cells_per_tile {
                buf.herb_biomass[i] = (t * 100 + i) as f64;
                buf.herb_height[i] = 20.0;
            }

            let tile = LandscapeTileSoA {
                tile_id: t as u64,
                x_offset: 0,
                y_offset: t * 10,
                width: self.cells_per_tile,
                height: 1,
                buffers: buf,
            };

            tokio::select! {
                _ = cancel.cancelled() => {
                    self.producer_cancelled.store(true, Ordering::SeqCst);
                    return Err(DataLoaderError::Cancelled);
                }
                res = sender.send(tile) => {
                    if res.is_err() {
                        // Channel closed by consumer
                        return Ok(());
                    }
                    self.produced_count.fetch_add(1, Ordering::SeqCst);
                }
            }
        }
        Ok(())
    }

    fn load_reference_data(&self) -> Result<ReferenceData, DataLoaderError> {
        Ok(ReferenceData::default())
    }
}

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn test_async_pipeline_normal_execution() {
    let produced = Arc::new(AtomicUsize::new(0));
    let cancelled = Arc::new(AtomicBool::new(false));

    let loader = Arc::new(MockStreamingDataLoader {
        tile_count: 5,
        cells_per_tile: 10,
        producer_cancelled: Arc::clone(&cancelled),
        produced_count: Arc::clone(&produced),
        fail_at_tile: None,
    });

    let consumed_count = Arc::new(AtomicUsize::new(0));
    let consumed_clone = Arc::clone(&consumed_count);

    let res = run_producer_consumer_pipeline(loader, 4, move |tile| {
        consumed_clone.fetch_add(1, Ordering::SeqCst);
        assert_eq!(tile.buffers.herb_biomass.len(), 10);
        Ok(())
    }).await;

    assert!(res.is_ok(), "Pipeline should complete with Ok");
    assert_eq!(produced.load(Ordering::SeqCst), 5);
    assert_eq!(consumed_count.load(Ordering::SeqCst), 5);
    assert!(!cancelled.load(Ordering::SeqCst));
}

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn test_async_pipeline_consumer_cancellation_cooperative() {
    let produced = Arc::new(AtomicUsize::new(0));
    let cancelled = Arc::new(AtomicBool::new(false));

    let loader = Arc::new(MockStreamingDataLoader {
        tile_count: 50,
        cells_per_tile: 10,
        producer_cancelled: Arc::clone(&cancelled),
        produced_count: Arc::clone(&produced),
        fail_at_tile: None,
    });

    let consumed = Arc::new(AtomicUsize::new(0));
    let consumed_clone = Arc::clone(&consumed);

    let res = run_producer_consumer_pipeline(loader, 2, move |_tile| {
        let count = consumed_clone.fetch_add(1, Ordering::SeqCst);
        if count >= 3 {
            return Err(DataLoaderError::Validation("Consumer aborted after 3 tiles".into()));
        }
        Ok(())
    }).await;

    // Pipeline should return the consumer error
    assert!(res.is_err(), "Pipeline should return Err when consumer fails");
    match res.unwrap_err() {
        DataLoaderError::Validation(msg) => {
            assert!(msg.contains("Consumer aborted"));
        }
        other => panic!("Expected Validation error, got: {:?}", other),
    }

    // Producer cancellation token must have fired
    assert!(cancelled.load(Ordering::SeqCst), "Producer must have received cancellation token");
    assert!(produced.load(Ordering::SeqCst) < 50, "Producer must not produce all 50 tiles");
}

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn test_async_pipeline_producer_error_propagation() {
    let produced = Arc::new(AtomicUsize::new(0));
    let cancelled = Arc::new(AtomicBool::new(false));

    let loader = Arc::new(MockStreamingDataLoader {
        tile_count: 10,
        cells_per_tile: 5,
        producer_cancelled: Arc::clone(&cancelled),
        produced_count: Arc::clone(&produced),
        fail_at_tile: Some(2), // Fails at tile 2
    });

    let consumed = Arc::new(AtomicUsize::new(0));
    let consumed_clone = Arc::clone(&consumed);

    let res = run_producer_consumer_pipeline(loader, 4, move |_tile| {
        consumed_clone.fetch_add(1, Ordering::SeqCst);
        Ok(())
    }).await;

    assert!(res.is_err(), "Pipeline should fail when producer encounters error");
    match res.unwrap_err() {
        DataLoaderError::Validation(msg) => {
            assert!(msg.contains("Simulated failure at tile 2"));
        }
        other => panic!("Expected Validation error, got: {:?}", other),
    }
}

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn test_async_pipeline_bounded_channel_backpressure() {
    let produced = Arc::new(AtomicUsize::new(0));
    let cancelled = Arc::new(AtomicBool::new(false));

    // 20 tiles, but channel capacity is only 2
    let loader = Arc::new(MockStreamingDataLoader {
        tile_count: 20,
        cells_per_tile: 10,
        producer_cancelled: Arc::clone(&cancelled),
        produced_count: Arc::clone(&produced),
        fail_at_tile: None,
    });

    let consumed = Arc::new(AtomicUsize::new(0));
    let consumed_clone = Arc::clone(&consumed);

    let res = run_producer_consumer_pipeline(loader, 2, move |_tile| {
        consumed_clone.fetch_add(1, Ordering::SeqCst);
        // Simulate compute load
        std::thread::sleep(Duration::from_millis(5));
        Ok(())
    }).await;

    assert!(res.is_ok());
    assert_eq!(consumed.load(Ordering::SeqCst), 20);
    assert_eq!(produced.load(Ordering::SeqCst), 20);
}

// =========================================================================
// 5. Additional Concurrency & Channel Lifecycle Regression Tests
// =========================================================================

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn test_async_pipeline_channel_capacity_zero_returns_validation_error() {
    let loader = Arc::new(MockStreamingDataLoader {
        tile_count: 5,
        cells_per_tile: 10,
        producer_cancelled: Arc::new(AtomicBool::new(false)),
        produced_count: Arc::new(AtomicUsize::new(0)),
        fail_at_tile: None,
    });

    let res = run_producer_consumer_pipeline(loader, 0, |_| Ok(())).await;

    assert!(res.is_err(), "Pipeline with channel_capacity=0 must return Err");
    match res.unwrap_err() {
        DataLoaderError::Validation(msg) => {
            assert!(
                msg.contains("channel_capacity must be greater than zero"),
                "Error message must specify channel_capacity requirement, got: {}",
                msg
            );
        }
        other => panic!("Expected DataLoaderError::Validation, got: {:?}", other),
    }
}

struct UncooperativeProducer {
    total_tiles: usize,
    produced_count: Arc<AtomicUsize>,
}

#[async_trait]
impl AsyncDataLoader for UncooperativeProducer {
    async fn stream_landscape_tiles(
        &self,
        sender: mpsc::Sender<LandscapeTileSoA>,
        _cancel: CancellationToken,
    ) -> Result<(), DataLoaderError> {
        for t in 0..self.total_tiles {
            let tile = LandscapeTileSoA {
                tile_id: t as u64,
                x_offset: 0,
                y_offset: t * 10,
                width: 5,
                height: 1,
                buffers: LandscapeCellBuffers::new(5),
            };

            // Uncooperative: does NOT select on cancel token!
            // When channel is full, blocks indefinitely on sender.send().await
            // unless the channel receiver is explicitly dropped by the consumer.
            sender
                .send(tile)
                .await
                .map_err(|e| DataLoaderError::Channel(e.to_string()))?;
            self.produced_count.fetch_add(1, Ordering::SeqCst);
        }
        Ok(())
    }

    fn load_reference_data(&self) -> Result<ReferenceData, DataLoaderError> {
        Ok(ReferenceData::default())
    }
}

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn test_async_pipeline_deadlock_prevention_on_uncooperative_producer() {
    let produced = Arc::new(AtomicUsize::new(0));

    // 10 tiles, bounded capacity 1.
    // The uncooperative producer will buffer 1 tile, then block on sender.send() for the 2nd tile.
    let loader = Arc::new(UncooperativeProducer {
        total_tiles: 10,
        produced_count: Arc::clone(&produced),
    });

    // Timeout guard: if deadlock occurs, timeout after 2 seconds instead of hanging the runner
    let test_future = async {
        run_producer_consumer_pipeline(loader, 1, |_tile| {
            // Consumer immediately fails on the very first tile
            Err(DataLoaderError::Validation(
                "Consumer aborted intentionally".into(),
            ))
        })
        .await
    };

    let result = tokio::time::timeout(Duration::from_secs(2), test_future).await;

    assert!(
        result.is_ok(),
        "Pipeline deadlocked! Producer was blocked on send() and rx was not dropped."
    );
    let pipeline_res = result.unwrap();

    assert!(
        pipeline_res.is_err(),
        "Pipeline should fail with consumer error"
    );
    match pipeline_res.unwrap_err() {
        DataLoaderError::Validation(msg) => {
            assert!(msg.contains("Consumer aborted intentionally"));
        }
        other => panic!("Expected consumer Validation error, got: {:?}", other),
    }
}
