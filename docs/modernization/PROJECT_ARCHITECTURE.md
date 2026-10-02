# Project: RVS Production Modernization

## Architecture
The modernized Rangeland Vegetation Simulator (RVS) simulation engine decouples pure computational subroutines, in-memory contiguous data representations, and input/output persistence into distinct, highly optimized architectural layers:

1. **Pure Mathematical Kernel (`crates/rvs_core`)**:
   - 46 pure, zero-allocation mathematical subroutines across Biomass (22: B01–B22), Fuels (12: F01–F12), Succession (6: S01–S06), and Disturbance (6: D01–D06).
   - Functions are `pub`, `#[inline]`, pure, free of heap allocation, and guarded against non-positive input domain errors.
2. **Structure-of-Arrays (SoA) Batch Engine & Rayon Multithreading**:
   - Canonical contiguous slice buffers (`&[f64]`) for cell-level attributes (N plots) and Segmented CSR slice buffers for sub-canopy shrub attributes (M shrubs across N plots).
   - Multicore parallel batch iteration using Rayon (`par_iter_mut`), enabling SIMD auto-vectorization without lock contention.
3. **Zero-Copy FFI Bridge (CXX / cbindgen)**:
   - Exposes Rust mathematical routines and batch SoA evaluation endpoints to C++20.
   - Maps C++20 `std::span<const double>` / `std::span<double>` directly to Rust `&[f64]` / `&mut [f64]` without intermediate memory allocations or buffer copies.
   - Static archive `librvs_core.a` linked directly into `librvs.so`.
4. **Input-Agnostic DataLoader & Tokio Async Streaming Pipeline**:
   - `IDataLoader` / `DataLoader` abstraction decoupling simulation business logic from direct database handles.
   - Concurrency pattern per `rust-async-patterns` skill:
     - **Async DataLoader & Tile Streaming**: Uses Tokio, async traits (`async-trait`), and bounded channels (`tokio::sync::mpsc`) for concurrent prefetching/streaming of spatial raster tiles (GeoTIFF/COG) and vector layers without blocking CPU compute threads.
     - **Producer-Consumer Pipeline**: Decouples asynchronous I/O ingestion from synchronous Rayon batch compute via bounded channels and semaphores (`tokio::sync::Semaphore`) for backpressure and memory-efficient streaming across large landscape extents.
     - **Structured Concurrency & Diagnostics**: Employs `tokio::task::JoinSet` for task lifecycle management, `tokio_util::sync::CancellationToken` for graceful cancellation/simulation interrupts, and `tracing` for async observability and telemetry.
   - In-memory pre-compiled reference catalog (`ReferenceData`) replacing all runtime SQLite queries.
   - Multi-format ingestion backends:
     - Legacy SQLite database (`rvs_demo.db`)
     - Spatial multi-band raster data (GeoTIFF / gridded landscape layers: NDVI, precipitation, elevation, cover, height) and spatial vector/columnar tables (Parquet/CSV).
   - Numerical equivalence verification ensuring identical inputs produce identical simulation results across both pipelines.
5. **Automated Verification & Property Testing Harness**:
   - Rust unit tests for all 46 subroutines and `proptest` property-based tests for stochastic input domain stability.
   - Requirement-driven 4-tier E2E opaque-box test suite + Tier 5 adversarial coverage hardening.

---

## Feature Inventory
Every feature identified during the Survey phase and user directives is enumerated below with its assigned milestone:

| # | Feature | Description | Milestone | Source |
|---|---------|-------------|-----------|--------|
| 1 | Biomass Equations B01–B18 | 18 pure allometric tree/shrub biomass equations (eq_165, eq_201, eq_202, eq_636, eq_636_2, eq_743, eq_998, eq_999, eq_1000, eq_1002, eq_1008, eq_1012, eq_1058, eq_basicBAT, eq_1153, eq_1160, eq_1161, eq_PCH) | M1 | rust_candidates.md, librvs |
| 2 | Biomass Driver Routines B19–B22 | 4 pure biomass calculation routines: calc_crown_width, calc_stems_per_acre, calc_herb_holdover, calc_attenuation | M1 | rust_candidates.md, librvs |
| 3 | Fuels Equations F01–F07 | 7 pure fuel loading equations (fuels_eq_1, fuels_eq_2, fuels_eq_13, fuels_eq_15, fuels_eq_17, fuels_eq_25/42, fuels_eq_33/46) | M1 | rust_candidates.md, librvs |
| 4 | Fuels Driver Routines F08–F12 | 5 pure fuel calculation routines: calc_fuel_component_wb, calc_fuel_component_fl, calc_total_1hr_fuel, calc_fuel_percentages, calc_herb_loading | M1 | rust_candidates.md, librvs |
| 5 | Succession Routines S01–S06 | 6 pure succession routines: calc_growth_rate, evaluate_herb_growth, calc_s2b_quadratic_form, calc_herb_biomass_production, scale_shrub_height, scale_shrub_cover | M1 | rust_candidates.md, librvs |
| 6 | Disturbance Routines D01–D06 | 6 pure disturbance routines: calc_graze_biomass_reduction, calc_graze_height_reduction, calc_mow_biomass_residual, calc_fire_biomass_consumption, calc_fire_shrub_mortality, calc_fire_herb_post_regrowth | M1 | rust_candidates.md, librvs |
| 7 | Non-Positive Domain Guards | Strict guards returning 0.0 or clamped values for ln/log10/sqrt with non-positive inputs and division-by-zero guards | M1 | ORIGINAL_REQUEST.md, survey |
| 8 | Property-Based Testing Suite | `proptest` test suite covering numerical stability, non-panicking, and monotonicity across stochastic domains | M1 | ORIGINAL_REQUEST.md § R5 |
| 9 | Contiguous SoA Buffer Layout | Design of `LandscapeCellBuffers` (length N) for plot-level layers (herb cover, herb height, NDVI, PPT, fuel loads) | M2 | ORIGINAL_REQUEST.md § R2 |
| 10 | Segmented CSR Shrub Buffer | Design of `LandscapeShrubBuffers` with `cell_offsets` for variable shrub sub-canopies (M shrubs across N cells) | M2 | ORIGINAL_REQUEST.md § R2 |
| 11 | Rayon Batch Iteration Kernels | Multithreaded batch evaluation kernels using `par_iter_mut` for biomass, fuels, succession, and disturbance | M2 | ORIGINAL_REQUEST.md § R2 |
| 12 | CXX / cbindgen FFI Bridge | Direct FFI interface mapping C++20 `std::span` and contiguous vectors to Rust slices (`&[f64]`, `&mut [f64]`) without copying | M3 | ORIGINAL_REQUEST.md § R4 |
| 13 | Rust Static Archive Build | Cargo configuration building `librvs_core.a` static archive with CXX C++ bridging artifacts | M3 | ORIGINAL_REQUEST.md § R6 |
| 14 | librvs Linkage Integration | Makefile / build integration linking `librvs_core.a` into shared library `build/librvs.so` | M3 | ORIGINAL_REQUEST.md § R6 |
| 15 | In-Memory ReferenceData Catalog | Pre-loaded in-memory reference tables (equations, coefficients, crosswalks) eliminating runtime SQLite queries | M4 | ORIGINAL_REQUEST.md § R3 |
| 16 | Abstract DataLoader Interface | `IDataLoader` / `DataLoader` interface decoupling simulation drivers from storage schemas | M4 | ORIGINAL_REQUEST.md § R3 |
| 17 | Tokio Async Streaming & Prefetching | Bounded `mpsc` channels and semaphore backpressure for async raster tile and vector prefetching (`rust-async-patterns`) | M4 | User Directive 2026-09-17 |
| 18 | Producer-Consumer Rayon Bridge | Decouples async I/O ingestion from synchronous Rayon batch processing via bounded channels | M4 | User Directive 2026-09-17 |
| 19 | Structured Concurrency & Diagnostics | `JoinSet`, `CancellationToken`, and `tracing` observability across the ingestion pipeline | M4 | User Directive 2026-09-17 |
| 20 | SQLite DataLoader Backend | Ingests `rvs_demo.db` (`Plots_FCCS`, `Shrubs_FCCS`, `Dist_year7fire`) into canonical SoA buffers | M4 | ORIGINAL_REQUEST.md § R3 |
| 21 | Spatial Raster DataLoader Backend | Ingests multi-band GeoTIFF / gridded raster layers into canonical SoA buffers | M4 | ORIGINAL_REQUEST.md § R3 |
| 22 | Spatial Vector / Columnar Backend | Ingests columnar Parquet / CSV / GeoPackage tables into canonical SoA buffers | M4 | ORIGINAL_REQUEST.md § R3 |
| 23 | In-Memory Grid Simulation Driver | C++/Rust simulation pipeline executing on abstract in-memory SoA grid buffers | M4 | ORIGINAL_REQUEST.md § R3 |
| 24 | SQLite vs Spatial Equivalence Test | Automated test verifying identical inputs produce identical simulation results across both pipelines | M4 | ORIGINAL_REQUEST.md § AC |
| 25 | 20-Year Baseline Verification | Full 20-year simulation execution on `rvs_demo.db` with ecological continuity and exit code 0 | M5 | ORIGINAL_REQUEST.md § AC |
| 26 | E2E Test Suite Execution (100% Pass) | Verification that all test tiers (Tiers 1–4) in `TEST_READY.md` pass 100% | M5 | Project Pattern Phase 1 |
| 27 | Adversarial Coverage Hardening | White-box adversarial testing (Tier 5) with Challengers ensuring robust edge-case coverage | M5 | Project Pattern Phase 2 |

---

## Milestones

| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| M1 | Rust Mathematical Crate (`crates/rvs_core`) & Property Testing | Implement all 46 pure mathematical subroutines (B01–B22, F01–F12, S01–S06, D01–D06), domain guards, unit tests, and `proptest` property-based tests (Features 1–8) | none | DONE |
| M2 | SoA Batch Engine & Multithreading | Design contiguous SoA slice layouts (`LandscapeCellBuffers`, Segmented CSR `LandscapeShrubBuffers`) and Rayon parallel batch execution kernels (Features 9–11) | M1 | DONE |
| M3 | Zero-Copy FFI Bridge & C++20 Integration | Implement CXX FFI bindings mapping `std::span` to Rust slices, compile `librvs_core.a`, and link into `build/librvs.so` (Features 12–14) | M2 | PLANNED |
| M4 | Input-Agnostic DataLoader & Async Spatial Ingestion | Decouple core from SQLite via `IDataLoader`, implement Tokio async tile prefetching, producer-consumer Rayon bridge, `JoinSet`/`CancellationToken`/`tracing`, SQLite and Spatial Raster (GeoTIFF) / Vector backends, in-memory reference catalog, and verify SQLite vs Raster equivalence (Features 15–24) | M2, M3 | PLANNED |
| M5 | Final Milestone: 100% E2E Verification & Hardening | Phase 1: Pass 100% of E2E test suite (`TEST_READY.md` Tiers 1–4). Phase 2: Adversarial coverage hardening (Tier 5). Verify 20-year ecological continuity against `rvs_demo.db` (Features 25–27) | M4, TEST_READY.md | PLANNED |

---

## Interface Contracts

### 1. Mathematical Crate (`crates/rvs_core`)
- Module structure:
  - `crates/rvs_core/src/biomass.rs`: B01–B22
  - `crates/rvs_core/src/fuels.rs`: F01–F12
  - `crates/rvs_core/src/succession.rs`: S01–S06
  - `crates/rvs_core/src/disturbance.rs`: D01–D06
- Common contract:
  - `pub fn <routine_name>(...) -> f64`
  - All routines pure, zero allocations, marked `#[inline]`.
  - Non-positive inputs for logarithmic and root functions return `0.0`.
  - Zero divisors guarded with safe epsilon or fallback value `0.0`.

### 2. SoA Batch Engine
- `LandscapeCellBuffers`:
  - `shrub_cover: &[f64]`, `shrub_height: &[f64]`, `herb_cover: &[f64]`, `herb_height: &[f64]`, `herb_biomass: &[f64]`, `ndvi: &[f64]`, `precip: &[f64]`, `elevation: &[f64]`.
- `LandscapeShrubBuffers`:
  - `cell_offsets: &[usize]` (CSR format index of length $N+1$).
  - `species_id: &[u32]`, `cover: &[f64]`, `height: &[f64]`, `biomass: &[f64]`, `stems_per_acre: &[f64]`.
- Batch functions:
  - `pub fn batch_evaluate_biomass(cells: &mut LandscapeCellBuffers, shrubs: &mut LandscapeShrubBuffers, refs: &ReferenceData)`
  - `pub fn batch_evaluate_fuels(cells: &mut LandscapeCellBuffers, shrubs: &LandscapeShrubBuffers, refs: &ReferenceData)`
  - `pub fn batch_evaluate_succession(cells: &mut LandscapeCellBuffers, shrubs: &mut LandscapeShrubBuffers, refs: &ReferenceData, year: u32)`
  - `pub fn batch_evaluate_disturbance(cells: &mut LandscapeCellBuffers, shrubs: &mut LandscapeShrubBuffers, dist: &DisturbanceSchedule)`

### 3. FFI Bridge (CXX Bridge Contract)
- `crates/rvs_core/src/ffi.rs` exposing:
  - `rust::Slice<const f64>` <-> `std::span<const double>`
  - `rust::Slice<f64>` <-> `std::span<double>`
  - Function signatures mirrored between C++ and Rust without intermediate serialization or buffer copying.

### 4. Async DataLoader Abstraction (`rust-async-patterns`)
- In Rust:
  ```rust
  #[async_trait::async_trait]
  pub trait AsyncDataLoader: Send + Sync {
      async fn stream_landscape_tiles(
          &self,
          sender: tokio::sync::mpsc::Sender<LandscapeTileSoA>,
          cancel: tokio_util::sync::CancellationToken,
      ) -> Result<(), DataLoaderError>;

      fn load_reference_data(&self) -> Result<ReferenceData, DataLoaderError>;
  }
  ```
- Producer-Consumer bridge:
  - Background Tokio task prefetches tiles into bounded `tokio::sync::mpsc::channel(capacity)`.
  - Worker thread consumer takes tiles and passes slices to Rayon batch kernels.
- In C++20:
  ```cpp
  class IDataLoader {
  public:
      virtual ~IDataLoader() = default;
      virtual LandscapeGridSoA load_landscape() = 0;
      virtual ReferenceData load_reference_data() = 0;
      virtual void write_landscape(int year, const LandscapeGridSoA& grid) = 0;
  };
  ```

---

## Code Layout

```
/home/robbl/repos/rvs-legacy/
├── crates/
│   └── rvs_core/                     # Rust mathematical crate
│       ├── Cargo.toml
│       ├── build.rs                  # CXX bridge code generation
│       ├── src/
│       │   ├── lib.rs                # Crate root and module exports
│       │   ├── biomass.rs            # B01–B22
│       │   ├── fuels.rs              # F01–F12
│       │   ├── succession.rs         # S01–S06
│       │   ├── disturbance.rs        # D01–D06
│       │   ├── soa.rs                # Structure-of-Arrays slice layouts
│       │   ├── batch.rs              # Rayon parallel batch execution
│       │   ├── data_loader.rs        # Async DataLoader & tile prefetching
│       │   └── ffi.rs                # CXX bidirectional FFI definitions
│       └── tests/
│           ├── unit_tests.rs         # Deterministic unit tests for 46 routines
│           └── property_tests.rs     # proptest stochastic property tests
├── librvs/                           # Modernized C++20 codebase
│   ├── makefile                      # Integrated build pipeline linking librvs_core.a
│   ├── main.cpp                      # Simulation driver supporting SQLite and Spatial
│   ├── Biomass/                      # C++ Biomass module & FFI integration
│   ├── Fuels/                        # C++ Fuels module & FFI integration
│   ├── Succession/                   # C++ Succession module & FFI integration
│   ├── Disturbance/                  # C++ Disturbance module & FFI integration
│   └── DataManagement/               # DataLoader C++ implementations & SoA buffers
│       ├── IDataLoader.h
│       ├── SQLiteDataLoader.h / .cpp
│       ├── SpatialRasterDataLoader.h / .cpp
│       └── LandscapeGridSoA.h
├── tests/                            # Requirement-driven E2E test suite
│   ├── e2e_runner.py / .sh           # Opaque-box E2E test harness
│   ├── test_tier1_features/          # Tier 1 tests (>=5 per feature)
│   ├── test_tier2_boundaries/        # Tier 2 boundary tests
│   ├── test_tier3_combinations/      # Tier 3 cross-feature combinations
│   ├── test_tier4_applications/      # Tier 4 real-world workloads
│   └── test_tier5_adversarial/       # Tier 5 white-box coverage hardening
├── TEST_INFRA.md                     # E2E test suite index and philosophy
├── TEST_READY.md                     # Test suite ready signal
└── rvs_demo.db                       # Demonstration SQLite database
```
