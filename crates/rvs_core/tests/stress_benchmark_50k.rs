//! Large-Scale Stress Test & Performance Benchmark (50,000+ Cells)
//!
//! Verifies:
//! 1. Zero heap allocations during batch loop execution
//! 2. Measurable Rayon multi-core speedup (1 thread vs N threads)
//! 3. Bit-identical numerical determinism across thread counts
//! 4. Memory stability across 50,000 cells and 150,000 shrubs

use std::alloc::{GlobalAlloc, Layout, System};
use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
use std::sync::Mutex;
use std::time::Instant;

use rvs_core::soa::{LandscapeCellBuffers, LandscapeShrubBuffers};
use rvs_core::batch::{
    batch_evaluate_biomass,
    batch_evaluate_fuels,
    batch_evaluate_succession,
    batch_evaluate_disturbance_graze,
    ReferenceData,
};

// Mutex to ensure tests in this binary do not interfere with allocation tracking
static TEST_SERIALIZER: Mutex<()> = Mutex::new(());

// Global Tracking Allocator for Zero-Allocation Proof
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

fn generate_test_landscape(
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

    shrubs.validate().expect("Generated landscape must satisfy CSR invariants");

    let refs = ReferenceData {
        shrub_eq_cf1: vec![1.5, 2.0, 2.5, 1.8, 2.2],
        shrub_eq_cf2: vec![1.1, 1.3, 1.2, 1.4, 1.15],
        ..Default::default()
    };

    (cells, shrubs, refs)
}

#[test]
fn test_large_scale_zero_allocation_and_determinism_50k() {
    let _lock = TEST_SERIALIZER.lock().unwrap();

    const NUM_CELLS: usize = 50_000;
    const SHRUBS_PER_CELL: usize = 3;

    let (mut cells_seq, mut shrubs_seq, refs) = generate_test_landscape(NUM_CELLS, SHRUBS_PER_CELL);
    let (mut cells_par, mut shrubs_par, _) = generate_test_landscape(NUM_CELLS, SHRUBS_PER_CELL);
    let mut fuels_seq = vec![0.0; NUM_CELLS];
    let mut fuels_par = vec![0.0; NUM_CELLS];

    // 1. Threadpools
    let pool_1 = rayon::ThreadPoolBuilder::new().num_threads(1).build().unwrap();
    let num_threads = std::thread::available_parallelism().map(|n| n.get()).unwrap_or(4);
    let pool_n = rayon::ThreadPoolBuilder::new().num_threads(num_threads).build().unwrap();

    // 2. Sequential Run (1 thread)
    pool_1.install(|| {
        batch_evaluate_biomass(&mut cells_seq.view_mut(), &mut shrubs_seq, &refs);
        batch_evaluate_fuels(&cells_seq, &mut fuels_seq, 0.7);
        batch_evaluate_succession(&mut cells_seq.view_mut(), 1.2, 0.5);
        batch_evaluate_disturbance_graze(&mut cells_seq.view_mut(), 25.0);
    });

    // Warmup pool_n to ensure Rayon worker threads and thread-local state are ready
    pool_n.install(|| {
        let mut dummy_cells = LandscapeCellBuffers::new(64);
        let mut dummy_shrubs = LandscapeShrubBuffers::new(64, 64);
        let mut dummy_fuels = vec![0.0; 64];
        dummy_shrubs.cell_offsets[64] = 64;
        batch_evaluate_biomass(&mut dummy_cells.view_mut(), &mut dummy_shrubs, &refs);
        batch_evaluate_fuels(&dummy_cells, &mut dummy_fuels, 0.5);
        batch_evaluate_succession(&mut dummy_cells.view_mut(), 1.0, 1.0);
        batch_evaluate_disturbance_graze(&mut dummy_cells.view_mut(), 20.0);
    });

    // 3. Parallel Run with Zero-Allocation Tracking
    pool_n.install(|| {
        ALLOC_COUNT.store(0, Ordering::SeqCst);
        TRACKING_ACTIVE.store(true, Ordering::SeqCst);

        batch_evaluate_biomass(&mut cells_par.view_mut(), &mut shrubs_par, &refs);
        batch_evaluate_fuels(&cells_par, &mut fuels_par, 0.7);
        batch_evaluate_succession(&mut cells_par.view_mut(), 1.2, 0.5);
        batch_evaluate_disturbance_graze(&mut cells_par.view_mut(), 25.0);

        TRACKING_ACTIVE.store(false, Ordering::SeqCst);
    });

    let allocations = ALLOC_COUNT.load(Ordering::SeqCst);

    // ZERO ALLOCATION ASSERTION
    assert_eq!(
        allocations, 0,
        "Batch evaluation must produce ZERO heap allocations, found: {}",
        allocations
    );

    // DETERMINISM ASSERTION
    assert_eq!(
        cells_seq.herb_biomass, cells_par.herb_biomass,
        "Herb biomass mismatch between 1 and N threads"
    );
    assert_eq!(
        cells_seq.herb_height, cells_par.herb_height,
        "Herb height mismatch between 1 and N threads"
    );
    assert_eq!(
        cells_seq.shrub_height, cells_par.shrub_height,
        "Shrub height mismatch between 1 and N threads"
    );
    assert_eq!(
        cells_seq.shrub_cover, cells_par.shrub_cover,
        "Shrub cover mismatch between 1 and N threads"
    );
    assert_eq!(
        fuels_seq, fuels_par,
        "Fuels output mismatch between 1 and N threads"
    );
    assert_eq!(
        shrubs_seq.stems_per_acre, shrubs_par.stems_per_acre,
        "Shrub stems mismatch between 1 and N threads"
    );
}

#[test]
fn test_large_scale_rayon_speedup_50k() {
    let _lock = TEST_SERIALIZER.lock().unwrap();

    const NUM_CELLS: usize = 50_000;
    const SHRUBS_PER_CELL: usize = 3;
    const ITERATIONS: usize = 20;

    let num_threads = std::thread::available_parallelism().map(|n| n.get()).unwrap_or(1);
    if num_threads < 2 {
        println!("Skipping speedup test: system has only 1 hardware thread");
        return;
    }

    let (mut cells_1, mut shrubs_1, refs) = generate_test_landscape(NUM_CELLS, SHRUBS_PER_CELL);
    let (mut cells_n, mut shrubs_n, _) = generate_test_landscape(NUM_CELLS, SHRUBS_PER_CELL);
    let mut fuels_1 = vec![0.0; NUM_CELLS];
    let mut fuels_n = vec![0.0; NUM_CELLS];

    let pool_1 = rayon::ThreadPoolBuilder::new().num_threads(1).build().unwrap();
    let pool_n = rayon::ThreadPoolBuilder::new().num_threads(num_threads).build().unwrap();

    // Warmup
    pool_1.install(|| batch_evaluate_biomass(&mut cells_1.view_mut(), &mut shrubs_1, &refs));
    pool_n.install(|| batch_evaluate_biomass(&mut cells_n.view_mut(), &mut shrubs_n, &refs));

    // Time single thread
    let start_1 = Instant::now();
    pool_1.install(|| {
        for _ in 0..ITERATIONS {
            batch_evaluate_biomass(&mut cells_1.view_mut(), &mut shrubs_1, &refs);
            batch_evaluate_fuels(&cells_1, &mut fuels_1, 0.7);
            batch_evaluate_succession(&mut cells_1.view_mut(), 1.0, 0.5);
            batch_evaluate_disturbance_graze(&mut cells_1.view_mut(), 20.0);
        }
    });
    let dur_1 = start_1.elapsed();

    // Time multi thread
    let start_n = Instant::now();
    pool_n.install(|| {
        for _ in 0..ITERATIONS {
            batch_evaluate_biomass(&mut cells_n.view_mut(), &mut shrubs_n, &refs);
            batch_evaluate_fuels(&cells_n, &mut fuels_n, 0.7);
            batch_evaluate_succession(&mut cells_n.view_mut(), 1.0, 0.5);
            batch_evaluate_disturbance_graze(&mut cells_n.view_mut(), 20.0);
        }
    });
    let dur_n = start_n.elapsed();

    let speedup = dur_1.as_secs_f64() / dur_n.as_secs_f64();
    println!(
        "Rayon 50k Cells Benchmark ({} iterations): 1 thread = {:?}, {} threads = {:?}, Speedup = {:.2}x",
        ITERATIONS, dur_1, num_threads, dur_n, speedup
    );

    assert!(
        speedup >= 1.25,
        "Expected at least 1.25x speedup across {} threads, observed: {:.2}x",
        num_threads, speedup
    );
}
