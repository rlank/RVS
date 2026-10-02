//! # rvs_core: High-Performance Mathematical Simulation Engine for RVS
//!
//! `rvs_core` provides pure mathematical kernels, Structure-of-Arrays (SoA) batch
//! parallelization, asynchronous spatial tile streaming, and zero-copy CXX FFI bindings
//! for the Rangeland Vegetation Simulator.
//!
//! ## Subroutine Inventory
//! - [`biomass`]: 22 subroutines (B01–B22) covering allometric equations and drivers.
//! - [`fuels`]: 12 subroutines (F01–F12) covering allometric fuel loading and partitioning.
//! - [`succession`]: 6 subroutines (S01–S06, plus plot age calculation).
//! - [`disturbance`]: 6 subroutines (D01–D06, plus graze removal calculation).
//! - [`soa`]: Structure-of-Arrays memory buffers for cache locality.
//! - [`batch`]: Multithreaded Rayon batch evaluation kernels.
//! - [`data_loader`]: Asynchronous tile streaming & producer-consumer pipeline.
//! - [`ffi`]: Zero-copy CXX FFI bindings mapping `std::span` to Rust slices.

pub mod biomass;
pub mod fuels;
pub mod succession;
pub mod disturbance;
pub mod soa;
pub mod batch;
pub mod data_loader;
pub mod ffi;

/// Convenient prelude re-exporting key types and kernels
pub mod prelude {
    pub use crate::biomass::*;
    pub use crate::fuels::*;
    pub use crate::succession::*;
    pub use crate::disturbance::*;
    pub use crate::soa::*;
    pub use crate::batch::*;
    pub use crate::data_loader::*;
}
