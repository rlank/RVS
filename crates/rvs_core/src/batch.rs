//! Rayon parallel batch execution kernels for landscape simulation grids.
//!
//! Evaluates ecological transformations across large landscape extents using
//! multithreaded Rayon parallel iterators, zero-copy contiguous slices, and
//! lock-free operations.

use rayon::prelude::*;
use crate::soa::{LandscapeCellBuffers, LandscapeCellBuffersMutView, LandscapeShrubBuffers};
use crate::biomass::*;
use crate::fuels::*;
use crate::succession::*;
use crate::disturbance::*;

/// In-memory reference coefficients catalog eliminating runtime database queries.
#[derive(Debug, Clone, Default)]
pub struct ReferenceData {
    pub shrub_eq_cf1: Vec<f64>,
    pub shrub_eq_cf2: Vec<f64>,
}

/// Batch biomass evaluation across all cells and shrub sub-canopy layers using Rayon.
pub fn batch_evaluate_biomass(
    cells: &mut LandscapeCellBuffersMutView<'_>,
    shrubs: &mut LandscapeShrubBuffers,
    refs: &ReferenceData,
) {
    let n = cells.num_cells;
    assert_eq!(
        cells.shrub_cover.len(),
        n,
        "cells.shrub_cover buffer length ({}) must match num_cells ({})",
        cells.shrub_cover.len(),
        n
    );
    assert_eq!(
        cells.shrub_height.len(),
        n,
        "cells.shrub_height buffer length ({}) must match num_cells ({})",
        cells.shrub_height.len(),
        n
    );
    assert_eq!(
        cells.herb_cover.len(),
        n,
        "cells.herb_cover buffer length ({}) must match num_cells ({})",
        cells.herb_cover.len(),
        n
    );
    assert_eq!(
        cells.herb_height.len(),
        n,
        "cells.herb_height buffer length ({}) must match num_cells ({})",
        cells.herb_height.len(),
        n
    );
    assert_eq!(
        cells.herb_biomass.len(),
        n,
        "cells.herb_biomass buffer length ({}) must match num_cells ({})",
        cells.herb_biomass.len(),
        n
    );

    assert_eq!(
        shrubs.num_cells,
        n,
        "shrubs.num_cells ({}) must match cells.num_cells ({})",
        shrubs.num_cells,
        n
    );
    assert_eq!(
        shrubs.cell_offsets.len(),
        shrubs.num_cells + 1,
        "shrubs.cell_offsets buffer length ({}) must match shrubs.num_cells + 1 ({})",
        shrubs.cell_offsets.len(),
        shrubs.num_cells + 1
    );
    assert_eq!(
        shrubs.height.len(),
        shrubs.num_shrubs,
        "shrubs.height buffer length ({}) must match num_shrubs ({})",
        shrubs.height.len(),
        shrubs.num_shrubs
    );
    assert_eq!(
        shrubs.cover.len(),
        shrubs.num_shrubs,
        "shrubs.cover buffer length ({}) must match num_shrubs ({})",
        shrubs.cover.len(),
        shrubs.num_shrubs
    );
    assert_eq!(
        shrubs.biomass.len(),
        shrubs.num_shrubs,
        "shrubs.biomass buffer length ({}) must match num_shrubs ({})",
        shrubs.biomass.len(),
        shrubs.num_shrubs
    );
    assert_eq!(
        shrubs.stems_per_acre.len(),
        shrubs.num_shrubs,
        "shrubs.stems_per_acre buffer length ({}) must match num_shrubs ({})",
        shrubs.stems_per_acre.len(),
        shrubs.num_shrubs
    );
    assert_eq!(
        shrubs.species_id.len(),
        shrubs.num_shrubs,
        "shrubs.species_id buffer length ({}) must match num_shrubs ({})",
        shrubs.species_id.len(),
        shrubs.num_shrubs
    );

    // Parallel cell-level canopy competition attenuation
    cells.herb_biomass
        .par_iter_mut()
        .zip(cells.herb_cover.par_iter())
        .zip(cells.herb_height.par_iter())
        .zip(cells.shrub_cover.par_iter())
        .zip(cells.shrub_height.par_iter())
        .for_each(|((((hb, &hc), &hh), &sc), &sh)| {
            *hb = calc_attenuation(*hb, hc, hh, sc, sh);
        });

    // Parallel shrub stems per acre calculation
    shrubs.stems_per_acre
        .par_iter_mut()
        .zip(shrubs.height.par_iter())
        .zip(shrubs.cover.par_iter())
        .zip(shrubs.species_id.par_iter())
        .for_each(|(((stems, &h), &c), &sp)| {
            let sp_idx = sp as usize;
            let cf1 = refs.shrub_eq_cf1.get(sp_idx).copied().unwrap_or(1.0);
            let cf2 = refs.shrub_eq_cf2.get(sp_idx).copied().unwrap_or(1.5);
            let stem_area = eq_pch(cf1, cf2, h);
            *stems = calc_stems_per_acre_pure(stem_area, c, h);
        });
}

/// Batch fuels evaluation across cell buffers using Rayon.
pub fn batch_evaluate_fuels(
    cells: &LandscapeCellBuffers,
    out_1hr_fuel: &mut [f64],
    cure_rate: f64,
) {
    let n = cells.len();
    assert_eq!(
        out_1hr_fuel.len(),
        n,
        "out_1hr_fuel buffer length ({}) must match cells.len() ({})",
        out_1hr_fuel.len(),
        n
    );
    assert_eq!(
        cells.herb_biomass.len(),
        n,
        "cells.herb_biomass buffer length ({}) must match cells.len() ({})",
        cells.herb_biomass.len(),
        n
    );

    out_1hr_fuel
        .par_iter_mut()
        .zip(cells.herb_biomass.par_iter())
        .for_each(|(fuel_1hr, &herb_b)| {
            let herb_fine = calc_herb_loading(herb_b, cure_rate);
            *fuel_1hr = calc_total_1hr_fuel(0.0, 0.0, herb_fine);
        });
}

/// Batch succession evaluation advancing vegetative height and cover using Rayon.
pub fn batch_evaluate_succession(
    cells: &mut LandscapeCellBuffersMutView<'_>,
    delta_shrub_ht: f64,
    delta_shrub_cov: f64,
) {
    let n = cells.num_cells;
    assert_eq!(
        cells.shrub_height.len(),
        n,
        "cells.shrub_height buffer length ({}) must match num_cells ({})",
        cells.shrub_height.len(),
        n
    );
    assert_eq!(
        cells.shrub_cover.len(),
        n,
        "cells.shrub_cover buffer length ({}) must match num_cells ({})",
        cells.shrub_cover.len(),
        n
    );

    cells.shrub_height
        .par_iter_mut()
        .zip(cells.shrub_cover.par_iter_mut())
        .for_each(|(h, c)| {
            *h = scale_shrub_height(*h, delta_shrub_ht);
            *c = scale_shrub_cover(*c, delta_shrub_cov);
        });
}

/// Batch disturbance evaluation applying grazing utilization across all cells using Rayon.
pub fn batch_evaluate_disturbance_graze(
    cells: &mut LandscapeCellBuffersMutView<'_>,
    utilization_pct: f64,
) {
    let n = cells.num_cells;
    assert_eq!(
        cells.herb_biomass.len(),
        n,
        "cells.herb_biomass buffer length ({}) must match num_cells ({})",
        cells.herb_biomass.len(),
        n
    );
    assert_eq!(
        cells.herb_height.len(),
        n,
        "cells.herb_height buffer length ({}) must match num_cells ({})",
        cells.herb_height.len(),
        n
    );

    cells.herb_biomass
        .par_iter_mut()
        .zip(cells.herb_height.par_iter_mut())
        .for_each(|(b, h)| {
            *b = calc_graze_biomass_reduction(*b, utilization_pct);
            *h = calc_graze_height_reduction(*h, utilization_pct);
        });
}
