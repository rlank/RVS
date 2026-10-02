//! Safe bidirectional CXX FFI bridge between C++20 and Rust.
//!
//! Maps C++20 `std::span` and numeric primitives directly to Rust slices and scalars
//! without intermediate copies or allocations.
//! Exposes dual naming conventions (`rust_*` and `rvs_*`) and unmangled C-ABI symbols.

use rayon::prelude::*;

#[allow(clippy::module_inception)]
#[cxx::bridge(namespace = "RVS::Rust")]
pub mod ffi {
    extern "Rust" {
        // =====================================================================
        // Biomass Routines (B01 - B22)
        // =====================================================================
        fn rust_eq_165(cf1: f64, cf2: f64, cover: f64) -> f64;
        fn rvs_eq_165(cf1: f64, cf2: f64, cover: f64) -> f64;
        fn rust_eq_201(cf1: f64, cf2: f64, cover: f64) -> f64;
        fn rvs_eq_201(cf1: f64, cf2: f64, cover: f64) -> f64;
        fn rust_eq_202(cf1: f64, cf2: f64, cover: f64) -> f64;
        fn rvs_eq_202(cf1: f64, cf2: f64, cover: f64) -> f64;
        fn rust_eq_636(cf1: f64, cf2: f64, length: f64, width: f64, height: f64) -> f64;
        fn rvs_eq_636(cf1: f64, cf2: f64, length: f64, width: f64, height: f64) -> f64;
        fn rust_eq_636_2(cf1: f64, cf2: f64, vol: f64) -> f64;
        fn rvs_eq_636_2(cf1: f64, cf2: f64, vol: f64) -> f64;
        fn rust_eq_743(cf1: f64, cf2: f64, cover: f64, height: f64) -> f64;
        fn rvs_eq_743(cf1: f64, cf2: f64, cover: f64, height: f64) -> f64;
        fn rust_eq_998(cf1: f64, cf2: f64, width: f64) -> f64;
        fn rvs_eq_998(cf1: f64, cf2: f64, width: f64) -> f64;
        fn rust_eq_999(cf1: f64, cf2: f64, cf3: f64, length: f64, width: f64) -> f64;
        fn rvs_eq_999(cf1: f64, cf2: f64, cf3: f64, length: f64, width: f64) -> f64;
        fn rust_eq_1000(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64;
        fn rvs_eq_1000(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64;
        fn rust_eq_1002(cf1: f64, cf2: f64, vol: f64) -> f64;
        fn rvs_eq_1002(cf1: f64, cf2: f64, vol: f64) -> f64;
        fn rust_eq_1008(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64;
        fn rvs_eq_1008(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64;
        fn rust_eq_1012(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64;
        fn rvs_eq_1012(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64;
        fn rust_eq_1058(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64;
        fn rvs_eq_1058(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64;
        fn rust_eq_basic_bat(cf1: f64, cf2: f64, length: f64, width: f64) -> f64;
        fn rvs_eq_basic_bat(cf1: f64, cf2: f64, length: f64, width: f64) -> f64;
        fn rust_eq_1153(cf1: f64, cf2: f64, cf3: f64, l: f64, w: f64, h: f64) -> f64;
        fn rvs_eq_1153(cf1: f64, cf2: f64, cf3: f64, l: f64, w: f64, h: f64) -> f64;
        fn rust_eq_1160(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rvs_eq_1160(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rust_eq_1161(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rvs_eq_1161(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rust_eq_pch(cf1: f64, cf2: f64, height: f64) -> f64;
        fn rvs_eq_pch(cf1: f64, cf2: f64, height: f64) -> f64;
        fn rust_calc_crown_width(crown_area: f64) -> f64;
        fn rvs_calc_crown_width(crown_area: f64) -> f64;
        fn rust_calc_stems_per_acre_pure(single_stem_area: f64, cover: f64, height: f64) -> f64;
        fn rvs_calc_stems_per_acre_pure(single_stem_area: f64, cover: f64, height: f64) -> f64;
        fn rust_calc_herb_holdover(p0: f64, p1: f64, p2: f64) -> f64;
        fn rvs_calc_herb_holdover(p0: f64, p1: f64, p2: f64) -> f64;
        fn rust_calc_attenuation(herb_b: f64, hc: f64, hh: f64, sc: f64, sh: f64) -> f64;
        fn rvs_calc_attenuation(herb_b: f64, hc: f64, hh: f64, sc: f64, sh: f64) -> f64;

        // =====================================================================
        // Fuels Routines (F01 - F12)
        // =====================================================================
        fn rust_fuels_eq_1(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rvs_fuels_eq_1(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rust_fuels_eq_2(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rvs_fuels_eq_2(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rust_fuels_eq_13(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rvs_fuels_eq_13(cf1: f64, cf2: f64, p1: f64) -> f64;
        fn rust_fuels_eq_15(cf1: f64, cf2: f64, p1: f64, p2: f64) -> f64;
        fn rvs_fuels_eq_15(cf1: f64, cf2: f64, p1: f64, p2: f64) -> f64;
        fn rust_fuels_eq_17(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64;
        fn rvs_fuels_eq_17(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64;
        fn rust_fuels_eq_25(cf1: f64, cf2: f64, cf3: f64, cf4: f64, p1: f64, p2: f64, p3: f64) -> f64;
        fn rvs_fuels_eq_25(cf1: f64, cf2: f64, cf3: f64, cf4: f64, p1: f64, p2: f64, p3: f64) -> f64;
        fn rust_fuels_eq_33(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64;
        fn rvs_fuels_eq_33(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64;
        fn rust_calc_fuel_component_wb(shrub_biomass: f64, proportion: f64) -> f64;
        fn rvs_calc_fuel_component_wb(shrub_biomass: f64, proportion: f64) -> f64;
        fn rust_calc_fuel_component_fl(shrub_biomass: f64, proportion: f64) -> f64;
        fn rvs_calc_fuel_component_fl(shrub_biomass: f64, proportion: f64) -> f64;
        fn rust_calc_total_1hr_fuel(wb: f64, fl: f64, herb: f64) -> f64;
        fn rvs_calc_total_1hr_fuel(wb: f64, fl: f64, herb: f64) -> f64;
        fn rust_calc_herb_loading(herb_biomass: f64, cure_rate: f64) -> f64;
        fn rvs_calc_herb_loading(herb_biomass: f64, cure_rate: f64) -> f64;

        // =====================================================================
        // Succession Routines (S01 - S06)
        // =====================================================================
        fn rust_calc_growth_rate(current_val: f64, annual_rate: f64) -> f64;
        fn rvs_calc_growth_rate(current_val: f64, annual_rate: f64) -> f64;
        fn rust_calc_plot_age(cover: f64, min_cover: f64, growth_rate: f64, start_age: f64) -> f64;
        fn rvs_calc_plot_age(cover: f64, min_cover: f64, growth_rate: f64, start_age: f64) -> f64;
        fn rust_evaluate_herb_growth(b0: f64, b1: f64, b2: f64, b3: f64, ppt: f64, ndvi: f64) -> f64;
        fn rvs_evaluate_herb_growth(b0: f64, b1: f64, b2: f64, b3: f64, ppt: f64, ndvi: f64) -> f64;
        fn rust_calc_s2b_quadratic_form(x: &[f64], sigma: &[f64], dim: usize) -> f64;
        fn rvs_calc_s2b_quadratic_form(x: &[f64], sigma: &[f64], dim: usize) -> f64;
        fn rust_calc_herb_biomass_production(mean_prod: f64, s2b: f64, z_score: f64) -> f64;
        fn rvs_calc_herb_biomass_production(mean_prod: f64, s2b: f64, z_score: f64) -> f64;
        fn rust_scale_shrub_height(current_ht: f64, delta_ht: f64) -> f64;
        fn rvs_scale_shrub_height(current_ht: f64, delta_ht: f64) -> f64;
        fn rust_scale_shrub_cover(current_cov: f64, delta_cov: f64) -> f64;
        fn rvs_scale_shrub_cover(current_cov: f64, delta_cov: f64) -> f64;

        // =====================================================================
        // Disturbance Routines (D01 - D06)
        // =====================================================================
        fn rust_calc_graze_biomass_reduction(pre_biomass: f64, utilization_pct: f64) -> f64;
        fn rvs_calc_graze_biomass_reduction(pre_biomass: f64, utilization_pct: f64) -> f64;
        fn rust_calc_graze_height_reduction(pre_height: f64, utilization_pct: f64) -> f64;
        fn rvs_calc_graze_height_reduction(pre_height: f64, utilization_pct: f64) -> f64;
        fn rust_calc_mow_biomass_residual(pre_biomass: f64, pre_height: f64, mow_height: f64) -> f64;
        fn rvs_calc_mow_biomass_residual(pre_biomass: f64, pre_height: f64, mow_height: f64) -> f64;
        fn rust_calc_fire_biomass_consumption(biomass: f64, severity_k: f64, fire_intensity: f64) -> f64;
        fn rvs_calc_fire_biomass_consumption(biomass: f64, severity_k: f64, fire_intensity: f64) -> f64;
        fn rust_calc_fire_shrub_mortality(scorch_pct: f64, alpha: f64, beta: f64) -> f64;
        fn rvs_calc_fire_shrub_mortality(scorch_pct: f64, alpha: f64, beta: f64) -> f64;
        fn rust_calc_fire_herb_post_regrowth(resprout_pot: f64, mortality_pct: f64) -> f64;
        fn rvs_calc_fire_herb_post_regrowth(resprout_pot: f64, mortality_pct: f64) -> f64;
        fn rust_calc_graze_removal_amount(intake_lbs_per_day: f64, num_animals: f64, days: f64, area_acres: f64) -> f64;
        fn rvs_calc_graze_removal_amount(intake_lbs_per_day: f64, num_animals: f64, days: f64, area_acres: f64) -> f64;

        // =====================================================================
        // SoA Batch Endpoints (Features 9, 10, 11, 12)
        // =====================================================================
        fn rust_batch_evaluate_biomass(
            herb_biomass: &mut [f64],
            herb_cover: &[f64],
            herb_height: &[f64],
            shrub_cover: &[f64],
            shrub_height: &[f64],
            shrub_stems: &mut [f64],
            shrub_heights: &[f64],
            shrub_covers: &[f64],
            shrub_species: &[u32],
            ref_cf1: &[f64],
            ref_cf2: &[f64],
        ) -> i32;
        fn rvs_batch_evaluate_biomass(
            herb_biomass: &mut [f64],
            herb_cover: &[f64],
            herb_height: &[f64],
            shrub_cover: &[f64],
            shrub_height: &[f64],
            shrub_stems: &mut [f64],
            shrub_heights: &[f64],
            shrub_covers: &[f64],
            shrub_species: &[u32],
            ref_cf1: &[f64],
            ref_cf2: &[f64],
        ) -> i32;

        fn rust_batch_evaluate_fuels(
            out_1hr_fuel: &mut [f64],
            cells_herb_biomass: &[f64],
            cure_rate: f64,
        ) -> i32;
        fn rvs_batch_evaluate_fuels(
            out_1hr_fuel: &mut [f64],
            cells_herb_biomass: &[f64],
            cure_rate: f64,
        ) -> i32;

        fn rust_batch_evaluate_succession(
            shrub_height: &mut [f64],
            shrub_cover: &mut [f64],
            delta_height: &[f64],
            delta_cover: &[f64],
        ) -> i32;
        fn rvs_batch_evaluate_succession(
            shrub_height: &mut [f64],
            shrub_cover: &mut [f64],
            delta_height: &[f64],
            delta_cover: &[f64],
        ) -> i32;

        fn rust_batch_evaluate_succession_scalar(
            shrub_height: &mut [f64],
            shrub_cover: &mut [f64],
            delta_shrub_ht: f64,
            delta_shrub_cov: f64,
        ) -> i32;
        fn rvs_batch_evaluate_succession_scalar(
            shrub_height: &mut [f64],
            shrub_cover: &mut [f64],
            delta_shrub_ht: f64,
            delta_shrub_cov: f64,
        ) -> i32;

        fn rust_batch_evaluate_disturbance_graze(
            herb_biomass: &mut [f64],
            herb_height: &mut [f64],
            utilization_pct: f64,
        ) -> i32;
        fn rvs_batch_evaluate_disturbance_graze(
            herb_biomass: &mut [f64],
            herb_height: &mut [f64],
            utilization_pct: f64,
        ) -> i32;

        // Legacy slice attenuation endpoint
        fn rust_batch_calc_attenuation(
            herb_b: &mut [f64],
            hc: &[f64],
            hh: &[f64],
            sc: &[f64],
            sh: &[f64],
        );
        fn rvs_batch_calc_attenuation(
            herb_b: &mut [f64],
            hc: &[f64],
            hh: &[f64],
            sc: &[f64],
            sh: &[f64],
        );
    }
}

// =============================================================================
// Biomass Implementations (#[no_mangle] pub extern "C" for C ABI)
// =============================================================================

#[no_mangle]
pub extern "C" fn rust_eq_165(cf1: f64, cf2: f64, cover: f64) -> f64 { crate::biomass::eq_165(cf1, cf2, cover) }
#[no_mangle]
pub extern "C" fn rvs_eq_165(cf1: f64, cf2: f64, cover: f64) -> f64 { rust_eq_165(cf1, cf2, cover) }

#[no_mangle]
pub extern "C" fn rust_eq_201(cf1: f64, cf2: f64, cover: f64) -> f64 { crate::biomass::eq_201(cf1, cf2, cover) }
#[no_mangle]
pub extern "C" fn rvs_eq_201(cf1: f64, cf2: f64, cover: f64) -> f64 { rust_eq_201(cf1, cf2, cover) }

#[no_mangle]
pub extern "C" fn rust_eq_202(cf1: f64, cf2: f64, cover: f64) -> f64 { crate::biomass::eq_202(cf1, cf2, cover) }
#[no_mangle]
pub extern "C" fn rvs_eq_202(cf1: f64, cf2: f64, cover: f64) -> f64 { rust_eq_202(cf1, cf2, cover) }

#[no_mangle]
pub extern "C" fn rust_eq_636(cf1: f64, cf2: f64, length: f64, width: f64, height: f64) -> f64 { crate::biomass::eq_636(cf1, cf2, length, width, height) }
#[no_mangle]
pub extern "C" fn rvs_eq_636(cf1: f64, cf2: f64, length: f64, width: f64, height: f64) -> f64 { rust_eq_636(cf1, cf2, length, width, height) }

#[no_mangle]
pub extern "C" fn rust_eq_636_2(cf1: f64, cf2: f64, vol: f64) -> f64 { crate::biomass::eq_636_2(cf1, cf2, vol) }
#[no_mangle]
pub extern "C" fn rvs_eq_636_2(cf1: f64, cf2: f64, vol: f64) -> f64 { rust_eq_636_2(cf1, cf2, vol) }

#[no_mangle]
pub extern "C" fn rust_eq_743(cf1: f64, cf2: f64, cover: f64, height: f64) -> f64 { crate::biomass::eq_743(cf1, cf2, cover, height) }
#[no_mangle]
pub extern "C" fn rvs_eq_743(cf1: f64, cf2: f64, cover: f64, height: f64) -> f64 { rust_eq_743(cf1, cf2, cover, height) }

#[no_mangle]
pub extern "C" fn rust_eq_998(cf1: f64, cf2: f64, width: f64) -> f64 { crate::biomass::eq_998(cf1, cf2, width) }
#[no_mangle]
pub extern "C" fn rvs_eq_998(cf1: f64, cf2: f64, width: f64) -> f64 { rust_eq_998(cf1, cf2, width) }

#[no_mangle]
pub extern "C" fn rust_eq_999(cf1: f64, cf2: f64, cf3: f64, length: f64, width: f64) -> f64 { crate::biomass::eq_999(cf1, cf2, cf3, length, width) }
#[no_mangle]
pub extern "C" fn rvs_eq_999(cf1: f64, cf2: f64, cf3: f64, length: f64, width: f64) -> f64 { rust_eq_999(cf1, cf2, cf3, length, width) }

#[no_mangle]
pub extern "C" fn rust_eq_1000(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64 { crate::biomass::eq_1000(cf1, cf2, cf3, cf4, l, w, h) }
#[no_mangle]
pub extern "C" fn rvs_eq_1000(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64 { rust_eq_1000(cf1, cf2, cf3, cf4, l, w, h) }

#[no_mangle]
pub extern "C" fn rust_eq_1002(cf1: f64, cf2: f64, vol: f64) -> f64 { crate::biomass::eq_1002(cf1, cf2, vol) }
#[no_mangle]
pub extern "C" fn rvs_eq_1002(cf1: f64, cf2: f64, vol: f64) -> f64 { rust_eq_1002(cf1, cf2, vol) }

#[no_mangle]
pub extern "C" fn rust_eq_1008(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64 { crate::biomass::eq_1008(cf1, cf2, cf3, cf4, l, w, h) }
#[no_mangle]
pub extern "C" fn rvs_eq_1008(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64 { rust_eq_1008(cf1, cf2, cf3, cf4, l, w, h) }

#[no_mangle]
pub extern "C" fn rust_eq_1012(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64 { crate::biomass::eq_1012(cf1, cf2, cf3, p1, p2) }
#[no_mangle]
pub extern "C" fn rvs_eq_1012(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64 { rust_eq_1012(cf1, cf2, cf3, p1, p2) }

#[no_mangle]
pub extern "C" fn rust_eq_1058(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64 { crate::biomass::eq_1058(cf1, cf2, cf3, p1) }
#[no_mangle]
pub extern "C" fn rvs_eq_1058(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64 { rust_eq_1058(cf1, cf2, cf3, p1) }

#[no_mangle]
pub extern "C" fn rust_eq_basic_bat(cf1: f64, cf2: f64, length: f64, width: f64) -> f64 { crate::biomass::eq_basic_bat(cf1, cf2, length, width) }
#[no_mangle]
pub extern "C" fn rvs_eq_basic_bat(cf1: f64, cf2: f64, length: f64, width: f64) -> f64 { rust_eq_basic_bat(cf1, cf2, length, width) }

#[no_mangle]
pub extern "C" fn rust_eq_1153(cf1: f64, cf2: f64, cf3: f64, l: f64, w: f64, h: f64) -> f64 { crate::biomass::eq_1153(cf1, cf2, cf3, l, w, h) }
#[no_mangle]
pub extern "C" fn rvs_eq_1153(cf1: f64, cf2: f64, cf3: f64, l: f64, w: f64, h: f64) -> f64 { rust_eq_1153(cf1, cf2, cf3, l, w, h) }

#[no_mangle]
pub extern "C" fn rust_eq_1160(cf1: f64, cf2: f64, p1: f64) -> f64 { crate::biomass::eq_1160(cf1, cf2, p1) }
#[no_mangle]
pub extern "C" fn rvs_eq_1160(cf1: f64, cf2: f64, p1: f64) -> f64 { rust_eq_1160(cf1, cf2, p1) }

#[no_mangle]
pub extern "C" fn rust_eq_1161(cf1: f64, cf2: f64, p1: f64) -> f64 { crate::biomass::eq_1161(cf1, cf2, p1) }
#[no_mangle]
pub extern "C" fn rvs_eq_1161(cf1: f64, cf2: f64, p1: f64) -> f64 { rust_eq_1161(cf1, cf2, p1) }

#[no_mangle]
pub extern "C" fn rust_eq_pch(cf1: f64, cf2: f64, height: f64) -> f64 { crate::biomass::eq_pch(cf1, cf2, height) }
#[no_mangle]
pub extern "C" fn rvs_eq_pch(cf1: f64, cf2: f64, height: f64) -> f64 { rust_eq_pch(cf1, cf2, height) }

#[no_mangle]
pub extern "C" fn rust_calc_crown_width(crown_area: f64) -> f64 { crate::biomass::calc_crown_width(crown_area) }
#[no_mangle]
pub extern "C" fn rvs_calc_crown_width(crown_area: f64) -> f64 { rust_calc_crown_width(crown_area) }

#[no_mangle]
pub extern "C" fn rust_calc_stems_per_acre_pure(single_stem_area: f64, cover: f64, height: f64) -> f64 {
    crate::biomass::calc_stems_per_acre_pure(single_stem_area, cover, height)
}
#[no_mangle]
pub extern "C" fn rvs_calc_stems_per_acre_pure(single_stem_area: f64, cover: f64, height: f64) -> f64 {
    rust_calc_stems_per_acre_pure(single_stem_area, cover, height)
}

#[no_mangle]
pub extern "C" fn rust_calc_herb_holdover(p0: f64, p1: f64, p2: f64) -> f64 { crate::biomass::calc_herb_holdover(p0, p1, p2) }
#[no_mangle]
pub extern "C" fn rvs_calc_herb_holdover(p0: f64, p1: f64, p2: f64) -> f64 { rust_calc_herb_holdover(p0, p1, p2) }

#[no_mangle]
pub extern "C" fn rust_calc_attenuation(herb_b: f64, hc: f64, hh: f64, sc: f64, sh: f64) -> f64 {
    crate::biomass::calc_attenuation(herb_b, hc, hh, sc, sh)
}
#[no_mangle]
pub extern "C" fn rvs_calc_attenuation(herb_b: f64, hc: f64, hh: f64, sc: f64, sh: f64) -> f64 {
    rust_calc_attenuation(herb_b, hc, hh, sc, sh)
}

// =============================================================================
// Fuels Implementations (#[no_mangle] pub extern "C" for C ABI)
// =============================================================================

#[no_mangle]
pub extern "C" fn rust_fuels_eq_1(cf1: f64, cf2: f64, p1: f64) -> f64 { crate::fuels::fuels_eq_1(cf1, cf2, p1) }
#[no_mangle]
pub extern "C" fn rvs_fuels_eq_1(cf1: f64, cf2: f64, p1: f64) -> f64 { rust_fuels_eq_1(cf1, cf2, p1) }

#[no_mangle]
pub extern "C" fn rust_fuels_eq_2(cf1: f64, cf2: f64, p1: f64) -> f64 { crate::fuels::fuels_eq_2(cf1, cf2, p1) }
#[no_mangle]
pub extern "C" fn rvs_fuels_eq_2(cf1: f64, cf2: f64, p1: f64) -> f64 { rust_fuels_eq_2(cf1, cf2, p1) }

#[no_mangle]
pub extern "C" fn rust_fuels_eq_13(cf1: f64, cf2: f64, p1: f64) -> f64 { crate::fuels::fuels_eq_13(cf1, cf2, p1) }
#[no_mangle]
pub extern "C" fn rvs_fuels_eq_13(cf1: f64, cf2: f64, p1: f64) -> f64 { rust_fuels_eq_13(cf1, cf2, p1) }

#[no_mangle]
pub extern "C" fn rust_fuels_eq_15(cf1: f64, cf2: f64, p1: f64, p2: f64) -> f64 { crate::fuels::fuels_eq_15(cf1, cf2, p1, p2) }
#[no_mangle]
pub extern "C" fn rvs_fuels_eq_15(cf1: f64, cf2: f64, p1: f64, p2: f64) -> f64 { rust_fuels_eq_15(cf1, cf2, p1, p2) }

#[no_mangle]
pub extern "C" fn rust_fuels_eq_17(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64 { crate::fuels::fuels_eq_17(cf1, cf2, cf3, p1, p2) }
#[no_mangle]
pub extern "C" fn rvs_fuels_eq_17(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64 { rust_fuels_eq_17(cf1, cf2, cf3, p1, p2) }

#[no_mangle]
pub extern "C" fn rust_fuels_eq_25(cf1: f64, cf2: f64, cf3: f64, cf4: f64, p1: f64, p2: f64, p3: f64) -> f64 {
    crate::fuels::fuels_eq_25(cf1, cf2, cf3, cf4, p1, p2, p3)
}
#[no_mangle]
pub extern "C" fn rvs_fuels_eq_25(cf1: f64, cf2: f64, cf3: f64, cf4: f64, p1: f64, p2: f64, p3: f64) -> f64 {
    rust_fuels_eq_25(cf1, cf2, cf3, cf4, p1, p2, p3)
}

#[no_mangle]
pub extern "C" fn rust_fuels_eq_33(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64 { crate::fuels::fuels_eq_33(cf1, cf2, cf3, p1) }
#[no_mangle]
pub extern "C" fn rvs_fuels_eq_33(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64 { rust_fuels_eq_33(cf1, cf2, cf3, p1) }

#[no_mangle]
pub extern "C" fn rust_calc_fuel_component_wb(shrub_biomass: f64, proportion: f64) -> f64 {
    crate::fuels::calc_fuel_component_wb(shrub_biomass, proportion)
}
#[no_mangle]
pub extern "C" fn rvs_calc_fuel_component_wb(shrub_biomass: f64, proportion: f64) -> f64 {
    rust_calc_fuel_component_wb(shrub_biomass, proportion)
}

#[no_mangle]
pub extern "C" fn rust_calc_fuel_component_fl(shrub_biomass: f64, proportion: f64) -> f64 {
    crate::fuels::calc_fuel_component_fl(shrub_biomass, proportion)
}
#[no_mangle]
pub extern "C" fn rvs_calc_fuel_component_fl(shrub_biomass: f64, proportion: f64) -> f64 {
    rust_calc_fuel_component_fl(shrub_biomass, proportion)
}

#[no_mangle]
pub extern "C" fn rust_calc_total_1hr_fuel(wb: f64, fl: f64, herb: f64) -> f64 { crate::fuels::calc_total_1hr_fuel(wb, fl, herb) }
#[no_mangle]
pub extern "C" fn rvs_calc_total_1hr_fuel(wb: f64, fl: f64, herb: f64) -> f64 { rust_calc_total_1hr_fuel(wb, fl, herb) }

#[no_mangle]
pub extern "C" fn rust_calc_herb_loading(herb_biomass: f64, cure_rate: f64) -> f64 { crate::fuels::calc_herb_loading(herb_biomass, cure_rate) }
#[no_mangle]
pub extern "C" fn rvs_calc_herb_loading(herb_biomass: f64, cure_rate: f64) -> f64 { rust_calc_herb_loading(herb_biomass, cure_rate) }

// =============================================================================
// Succession Implementations (#[no_mangle] pub extern "C" for C ABI)
// =============================================================================

#[no_mangle]
pub extern "C" fn rust_calc_growth_rate(current_val: f64, annual_rate: f64) -> f64 { crate::succession::calc_growth_rate(current_val, annual_rate) }
#[no_mangle]
pub extern "C" fn rvs_calc_growth_rate(current_val: f64, annual_rate: f64) -> f64 { rust_calc_growth_rate(current_val, annual_rate) }

#[no_mangle]
pub extern "C" fn rust_calc_plot_age(cover: f64, min_cover: f64, growth_rate: f64, start_age: f64) -> f64 {
    crate::succession::calc_plot_age(cover, min_cover, growth_rate, start_age)
}
#[no_mangle]
pub extern "C" fn rvs_calc_plot_age(cover: f64, min_cover: f64, growth_rate: f64, start_age: f64) -> f64 {
    rust_calc_plot_age(cover, min_cover, growth_rate, start_age)
}

#[no_mangle]
pub extern "C" fn rust_evaluate_herb_growth(b0: f64, b1: f64, b2: f64, b3: f64, ppt: f64, ndvi: f64) -> f64 {
    crate::succession::evaluate_herb_growth(b0, b1, b2, b3, ppt, ndvi)
}
#[no_mangle]
pub extern "C" fn rvs_evaluate_herb_growth(b0: f64, b1: f64, b2: f64, b3: f64, ppt: f64, ndvi: f64) -> f64 {
    rust_evaluate_herb_growth(b0, b1, b2, b3, ppt, ndvi)
}

pub fn rust_calc_s2b_quadratic_form(x: &[f64], sigma: &[f64], dim: usize) -> f64 {
    crate::succession::calc_s2b_quadratic_form(x, sigma, dim)
}
pub fn rvs_calc_s2b_quadratic_form(x: &[f64], sigma: &[f64], dim: usize) -> f64 {
    rust_calc_s2b_quadratic_form(x, sigma, dim)
}

#[no_mangle]
pub extern "C" fn rust_calc_herb_biomass_production(mean_prod: f64, s2b: f64, z_score: f64) -> f64 {
    crate::succession::calc_herb_biomass_production(mean_prod, s2b, z_score)
}
#[no_mangle]
pub extern "C" fn rvs_calc_herb_biomass_production(mean_prod: f64, s2b: f64, z_score: f64) -> f64 {
    rust_calc_herb_biomass_production(mean_prod, s2b, z_score)
}

#[no_mangle]
pub extern "C" fn rust_scale_shrub_height(current_ht: f64, delta_ht: f64) -> f64 { crate::succession::scale_shrub_height(current_ht, delta_ht) }
#[no_mangle]
pub extern "C" fn rvs_scale_shrub_height(current_ht: f64, delta_ht: f64) -> f64 { rust_scale_shrub_height(current_ht, delta_ht) }

#[no_mangle]
pub extern "C" fn rust_scale_shrub_cover(current_cov: f64, delta_cov: f64) -> f64 { crate::succession::scale_shrub_cover(current_cov, delta_cov) }
#[no_mangle]
pub extern "C" fn rvs_scale_shrub_cover(current_cov: f64, delta_cov: f64) -> f64 { rust_scale_shrub_cover(current_cov, delta_cov) }

// =============================================================================
// Disturbance Implementations (#[no_mangle] pub extern "C" for C ABI)
// =============================================================================

#[no_mangle]
pub extern "C" fn rust_calc_graze_biomass_reduction(pre_biomass: f64, utilization_pct: f64) -> f64 {
    crate::disturbance::calc_graze_biomass_reduction(pre_biomass, utilization_pct)
}
#[no_mangle]
pub extern "C" fn rvs_calc_graze_biomass_reduction(pre_biomass: f64, utilization_pct: f64) -> f64 {
    rust_calc_graze_biomass_reduction(pre_biomass, utilization_pct)
}

#[no_mangle]
pub extern "C" fn rust_calc_graze_height_reduction(pre_height: f64, utilization_pct: f64) -> f64 {
    crate::disturbance::calc_graze_height_reduction(pre_height, utilization_pct)
}
#[no_mangle]
pub extern "C" fn rvs_calc_graze_height_reduction(pre_height: f64, utilization_pct: f64) -> f64 {
    rust_calc_graze_height_reduction(pre_height, utilization_pct)
}

#[no_mangle]
pub extern "C" fn rust_calc_mow_biomass_residual(pre_biomass: f64, pre_height: f64, mow_height: f64) -> f64 {
    crate::disturbance::calc_mow_biomass_residual(pre_biomass, pre_height, mow_height)
}
#[no_mangle]
pub extern "C" fn rvs_calc_mow_biomass_residual(pre_biomass: f64, pre_height: f64, mow_height: f64) -> f64 {
    rust_calc_mow_biomass_residual(pre_biomass, pre_height, mow_height)
}

#[no_mangle]
pub extern "C" fn rust_calc_fire_biomass_consumption(biomass: f64, severity_k: f64, fire_intensity: f64) -> f64 {
    crate::disturbance::calc_fire_biomass_consumption(biomass, severity_k, fire_intensity)
}
#[no_mangle]
pub extern "C" fn rvs_calc_fire_biomass_consumption(biomass: f64, severity_k: f64, fire_intensity: f64) -> f64 {
    rust_calc_fire_biomass_consumption(biomass, severity_k, fire_intensity)
}

#[no_mangle]
pub extern "C" fn rust_calc_fire_shrub_mortality(scorch_pct: f64, alpha: f64, beta: f64) -> f64 {
    crate::disturbance::calc_fire_shrub_mortality(scorch_pct, alpha, beta)
}
#[no_mangle]
pub extern "C" fn rvs_calc_fire_shrub_mortality(scorch_pct: f64, alpha: f64, beta: f64) -> f64 {
    rust_calc_fire_shrub_mortality(scorch_pct, alpha, beta)
}

#[no_mangle]
pub extern "C" fn rust_calc_fire_herb_post_regrowth(resprout_pot: f64, mortality_pct: f64) -> f64 {
    crate::disturbance::calc_fire_herb_post_regrowth(resprout_pot, mortality_pct)
}
#[no_mangle]
pub extern "C" fn rvs_calc_fire_herb_post_regrowth(resprout_pot: f64, mortality_pct: f64) -> f64 {
    rust_calc_fire_herb_post_regrowth(resprout_pot, mortality_pct)
}

#[no_mangle]
pub extern "C" fn rust_calc_graze_removal_amount(intake_lbs_per_day: f64, num_animals: f64, days: f64, area_acres: f64) -> f64 {
    crate::disturbance::calc_graze_removal_amount(intake_lbs_per_day, num_animals, days, area_acres)
}
#[no_mangle]
pub extern "C" fn rvs_calc_graze_removal_amount(intake_lbs_per_day: f64, num_animals: f64, days: f64, area_acres: f64) -> f64 {
    rust_calc_graze_removal_amount(intake_lbs_per_day, num_animals, days, area_acres)
}

// =============================================================================
// SoA Batch Implementations (Panic-Safe, Zero-Copy, Non-Panicking Error Codes)
// =============================================================================

/// Batch evaluation of biomass: canopy competition attenuation and shrub stems per acre.
/// Returns 0 on success, -1 on cell buffer length mismatch, -2 on shrub buffer length mismatch, -99 on caught panic.
pub fn rust_batch_evaluate_biomass(
    herb_biomass: &mut [f64],
    herb_cover: &[f64],
    herb_height: &[f64],
    shrub_cover: &[f64],
    shrub_height: &[f64],
    shrub_stems: &mut [f64],
    shrub_heights: &[f64],
    shrub_covers: &[f64],
    shrub_species: &[u32],
    ref_cf1: &[f64],
    ref_cf2: &[f64],
) -> i32 {
    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        let n = herb_biomass.len();
        if herb_cover.len() != n || herb_height.len() != n || shrub_cover.len() != n || shrub_height.len() != n {
            return -1;
        }

        let m = shrub_stems.len();
        if shrub_heights.len() != m || shrub_covers.len() != m || shrub_species.len() != m {
            return -2;
        }

        if n > 0 {
            herb_biomass
                .par_iter_mut()
                .with_min_len(1024)
                .zip(herb_cover.par_iter().with_min_len(1024))
                .zip(herb_height.par_iter().with_min_len(1024))
                .zip(shrub_cover.par_iter().with_min_len(1024))
                .zip(shrub_height.par_iter().with_min_len(1024))
                .for_each(|((((hb, &hc), &hh), &sc), &sh)| {
                    *hb = crate::biomass::calc_attenuation(*hb, hc, hh, sc, sh);
                });
        }

        if m > 0 {
            shrub_stems
                .par_iter_mut()
                .with_min_len(1024)
                .zip(shrub_heights.par_iter().with_min_len(1024))
                .zip(shrub_covers.par_iter().with_min_len(1024))
                .zip(shrub_species.par_iter().with_min_len(1024))
                .for_each(|(((stems, &h), &c), &sp)| {
                    let sp_idx = sp as usize;
                    let cf1 = ref_cf1.get(sp_idx).copied().unwrap_or(1.0);
                    let cf2 = ref_cf2.get(sp_idx).copied().unwrap_or(1.5);
                    let stem_area = crate::biomass::eq_pch(cf1, cf2, h);
                    *stems = crate::biomass::calc_stems_per_acre_pure(stem_area, c, h);
                });
        }

        0
    }));

    match result {
        Ok(code) => code,
        Err(_) => -99,
    }
}

pub fn rvs_batch_evaluate_biomass(
    herb_biomass: &mut [f64],
    herb_cover: &[f64],
    herb_height: &[f64],
    shrub_cover: &[f64],
    shrub_height: &[f64],
    shrub_stems: &mut [f64],
    shrub_heights: &[f64],
    shrub_covers: &[f64],
    shrub_species: &[u32],
    ref_cf1: &[f64],
    ref_cf2: &[f64],
) -> i32 {
    rust_batch_evaluate_biomass(
        herb_biomass,
        herb_cover,
        herb_height,
        shrub_cover,
        shrub_height,
        shrub_stems,
        shrub_heights,
        shrub_covers,
        shrub_species,
        ref_cf1,
        ref_cf2,
    )
}

/// Batch evaluation of fine fuels across landscape cells using Rayon.
/// Returns 0 on success, -1 on buffer length mismatch, -99 on caught panic.
pub fn rust_batch_evaluate_fuels(
    out_1hr_fuel: &mut [f64],
    cells_herb_biomass: &[f64],
    cure_rate: f64,
) -> i32 {
    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        let n = out_1hr_fuel.len();
        if cells_herb_biomass.len() != n {
            return -1;
        }
        if n == 0 {
            return 0;
        }

        if !cure_rate.is_finite() {
            out_1hr_fuel.par_iter_mut().with_min_len(2048).for_each(|f| *f = 0.0);
            return 0;
        }

        let cure_clamped = cure_rate.clamp(0.0, 1.0);

        out_1hr_fuel
            .par_iter_mut()
            .with_min_len(2048)
            .zip(cells_herb_biomass.par_iter().with_min_len(2048))
            .for_each(|(fuel_1hr, &herb_b)| {
                if !herb_b.is_finite() {
                    *fuel_1hr = 0.0;
                } else {
                    *fuel_1hr = herb_b.max(0.0) * cure_clamped;
                }
            });

        0
    }));

    match result {
        Ok(code) => code,
        Err(_) => -99,
    }
}

pub fn rvs_batch_evaluate_fuels(
    out_1hr_fuel: &mut [f64],
    cells_herb_biomass: &[f64],
    cure_rate: f64,
) -> i32 {
    rust_batch_evaluate_fuels(out_1hr_fuel, cells_herb_biomass, cure_rate)
}

/// Batch succession evaluation with cell-varying deltas across landscape cells using Rayon.
/// Returns 0 on success, -1 on buffer length mismatch, -99 on caught panic.
pub fn rust_batch_evaluate_succession(
    shrub_height: &mut [f64],
    shrub_cover: &mut [f64],
    delta_height: &[f64],
    delta_cover: &[f64],
) -> i32 {
    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        let n = shrub_height.len();
        if shrub_cover.len() != n || delta_height.len() != n || delta_cover.len() != n {
            return -1;
        }
        if n == 0 {
            return 0;
        }

        shrub_height
            .par_iter_mut()
            .with_min_len(2048)
            .zip(shrub_cover.par_iter_mut().with_min_len(2048))
            .zip(delta_height.par_iter().with_min_len(2048))
            .zip(delta_cover.par_iter().with_min_len(2048))
            .for_each(|(((h, c), &dh), &dc)| {
                *h = crate::succession::scale_shrub_height(*h, dh);
                *c = crate::succession::scale_shrub_cover(*c, dc);
            });

        0
    }));

    match result {
        Ok(code) => code,
        Err(_) => -99,
    }
}

pub fn rvs_batch_evaluate_succession(
    shrub_height: &mut [f64],
    shrub_cover: &mut [f64],
    delta_height: &[f64],
    delta_cover: &[f64],
) -> i32 {
    rust_batch_evaluate_succession(shrub_height, shrub_cover, delta_height, delta_cover)
}

/// Batch succession evaluation with uniform scalar deltas across landscape cells using Rayon.
/// Returns 0 on success, -1 on buffer length mismatch, -99 on caught panic.
pub fn rust_batch_evaluate_succession_scalar(
    shrub_height: &mut [f64],
    shrub_cover: &mut [f64],
    delta_shrub_ht: f64,
    delta_shrub_cov: f64,
) -> i32 {
    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        let n = shrub_height.len();
        if shrub_cover.len() != n {
            return -1;
        }
        if n == 0 {
            return 0;
        }

        shrub_height
            .par_iter_mut()
            .with_min_len(2048)
            .zip(shrub_cover.par_iter_mut().with_min_len(2048))
            .for_each(|(h, c)| {
                *h = crate::succession::scale_shrub_height(*h, delta_shrub_ht);
                *c = crate::succession::scale_shrub_cover(*c, delta_shrub_cov);
            });

        0
    }));

    match result {
        Ok(code) => code,
        Err(_) => -99,
    }
}

pub fn rvs_batch_evaluate_succession_scalar(
    shrub_height: &mut [f64],
    shrub_cover: &mut [f64],
    delta_shrub_ht: f64,
    delta_shrub_cov: f64,
) -> i32 {
    rust_batch_evaluate_succession_scalar(shrub_height, shrub_cover, delta_shrub_ht, delta_shrub_cov)
}

/// Batch disturbance evaluation applying grazing utilization across all cells using Rayon.
/// Returns 0 on success, -1 on buffer length mismatch, -99 on caught panic.
pub fn rust_batch_evaluate_disturbance_graze(
    herb_biomass: &mut [f64],
    herb_height: &mut [f64],
    utilization_pct: f64,
) -> i32 {
    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        let n = herb_biomass.len();
        if herb_height.len() != n {
            return -1;
        }
        if n == 0 {
            return 0;
        }

        if !utilization_pct.is_finite() {
            herb_biomass.par_iter_mut().with_min_len(1024).for_each(|b| *b = 0.0);
            herb_height.par_iter_mut().with_min_len(1024).for_each(|h| *h = 0.0);
            return 0;
        }

        let u = utilization_pct.clamp(0.0, 100.0) / 100.0;
        let biomass_factor = (1.0 - u).clamp(0.0, 1.0);
        let height_factor = biomass_factor.powf(0.7);

        herb_biomass
            .par_iter_mut()
            .with_min_len(1024)
            .zip(herb_height.par_iter_mut().with_min_len(1024))
            .for_each(|(b, h)| {
                *b = if !b.is_finite() || *b <= 0.0 {
                    0.0
                } else {
                    (*b * biomass_factor).max(0.0)
                };
                *h = if !h.is_finite() || *h <= 0.0 {
                    0.0
                } else {
                    (*h * height_factor).max(0.0)
                };
            });

        0
    }));

    match result {
        Ok(code) => code,
        Err(_) => -99,
    }
}

pub fn rvs_batch_evaluate_disturbance_graze(
    herb_biomass: &mut [f64],
    herb_height: &mut [f64],
    utilization_pct: f64,
) -> i32 {
    rust_batch_evaluate_disturbance_graze(herb_biomass, herb_height, utilization_pct)
}

/// Legacy batch attenuation calculation over slices (non-panicking, bounds-checked).
pub fn rust_batch_calc_attenuation(
    herb_b: &mut [f64],
    hc: &[f64],
    hh: &[f64],
    sc: &[f64],
    sh: &[f64],
) {
    let n = herb_b.len();
    if hc.len() != n || hh.len() != n || sc.len() != n || sh.len() != n {
        return;
    }
    herb_b
        .par_iter_mut()
        .with_min_len(1024)
        .zip(hc.par_iter().with_min_len(1024))
        .zip(hh.par_iter().with_min_len(1024))
        .zip(sc.par_iter().with_min_len(1024))
        .zip(sh.par_iter().with_min_len(1024))
        .for_each(|((((b, &h_c), &h_h), &s_c), &s_h)| {
            *b = crate::biomass::calc_attenuation(*b, h_c, h_h, s_c, s_h);
        });
}

pub fn rvs_batch_calc_attenuation(
    herb_b: &mut [f64],
    hc: &[f64],
    hh: &[f64],
    sc: &[f64],
    sh: &[f64],
) {
    rust_batch_calc_attenuation(herb_b, hc, hh, sc, sh);
}

// =============================================================================
// C-ABI Export Wrappers (Unmangled C Symbols for Dynamic Linkers & C Consumers)
// =============================================================================

/// C-ABI exports providing unmangled `rust_*` and `rvs_*` symbols in `.symtab` / `.dynsym`.
///
/// These wrappers accept raw pointer and length pairs, validate null pointers and alignment,
/// reconstruct safe Rust slices, and guard against panics using `std::panic::catch_unwind`.
pub mod c_abi {
    #[inline]
    unsafe fn make_slice<'a, T>(ptr: *const T, len: usize) -> Option<&'a [T]> {
        if len == 0 {
            Some(&[])
        } else if ptr.is_null() || (ptr as usize) % std::mem::align_of::<T>() != 0 {
            None
        } else {
            Some(unsafe { std::slice::from_raw_parts(ptr, len) })
        }
    }

    #[inline]
    unsafe fn make_slice_mut<'a, T>(ptr: *mut T, len: usize) -> Option<&'a mut [T]> {
        if len == 0 {
            Some(&mut [])
        } else if ptr.is_null() || (ptr as usize) % std::mem::align_of::<T>() != 0 {
            None
        } else {
            Some(unsafe { std::slice::from_raw_parts_mut(ptr, len) })
        }
    }

    // -------------------------------------------------------------------------
    // S03: calc_s2b_quadratic_form C-ABI Exports
    // -------------------------------------------------------------------------

    /// Evaluates multi-normal variance quadratic form: x^T * Sigma * x.
    ///
    /// # Safety
    /// - `x_ptr` must point to at least `x_len` valid `f64` elements if `x_len > 0`.
    /// - `sigma_ptr` must point to at least `sigma_len` valid `f64` elements if `sigma_len > 0`.
    /// - Returns `0.0` safely on null pointers, insufficient lengths (`x_len < dim` or
    ///   `sigma_len < dim * dim`), unaligned memory, non-finite values, or panic.
    #[no_mangle]
    pub unsafe extern "C" fn rust_calc_s2b_quadratic_form(
        x_ptr: *const f64,
        x_len: usize,
        sigma_ptr: *const f64,
        sigma_len: usize,
        dim: usize,
    ) -> f64 {
        if dim == 0 || x_len < dim || sigma_len < dim.saturating_mul(dim) {
            return 0.0;
        }
        let x = match unsafe { make_slice(x_ptr, x_len) } {
            Some(s) => s,
            None => return 0.0,
        };
        let sigma = match unsafe { make_slice(sigma_ptr, sigma_len) } {
            Some(s) => s,
            None => return 0.0,
        };

        let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            crate::succession::calc_s2b_quadratic_form(x, sigma, dim)
        }));

        match result {
            Ok(val) => val,
            Err(_) => 0.0,
        }
    }

    /// Dual symbol alias forwarding to `rust_calc_s2b_quadratic_form`.
    #[no_mangle]
    pub unsafe extern "C" fn rvs_calc_s2b_quadratic_form(
        x_ptr: *const f64,
        x_len: usize,
        sigma_ptr: *const f64,
        sigma_len: usize,
        dim: usize,
    ) -> f64 {
        unsafe { rust_calc_s2b_quadratic_form(x_ptr, x_len, sigma_ptr, sigma_len, dim) }
    }

    // -------------------------------------------------------------------------
    // B01-B22: batch_evaluate_biomass C-ABI Exports
    // -------------------------------------------------------------------------

    /// Evaluates biomass canopy competition attenuation and shrub stems per acre across slices.
    ///
    /// Accepts raw pointer and length pairs for all 11 slices.
    /// Returns:
    /// - `0`: Success.
    /// - `-1`: Cell buffer length mismatch (`herb_cover`, `herb_height`, `shrub_cover`, or `shrub_height` != `herb_biomass`).
    /// - `-2`: Shrub buffer length mismatch (`shrub_heights`, `shrub_covers`, or `shrub_species` != `shrub_stems`).
    /// - `-3`: Invalid / null pointer or unaligned pointer for non-empty slice.
    /// - `-99`: Caught panic (noexcept boundary guarantee).
    #[no_mangle]
    pub unsafe extern "C" fn rust_batch_evaluate_biomass(
        herb_biomass_ptr: *mut f64,
        herb_biomass_len: usize,
        herb_cover_ptr: *const f64,
        herb_cover_len: usize,
        herb_height_ptr: *const f64,
        herb_height_len: usize,
        shrub_cover_ptr: *const f64,
        shrub_cover_len: usize,
        shrub_height_ptr: *const f64,
        shrub_height_len: usize,
        shrub_stems_ptr: *mut f64,
        shrub_stems_len: usize,
        shrub_heights_ptr: *const f64,
        shrub_heights_len: usize,
        shrub_covers_ptr: *const f64,
        shrub_covers_len: usize,
        shrub_species_ptr: *const u32,
        shrub_species_len: usize,
        ref_cf1_ptr: *const f64,
        ref_cf1_len: usize,
        ref_cf2_ptr: *const f64,
        ref_cf2_len: usize,
    ) -> i32 {
        let herb_biomass = match unsafe { make_slice_mut(herb_biomass_ptr, herb_biomass_len) } {
            Some(s) => s,
            None => return -3,
        };
        let herb_cover = match unsafe { make_slice(herb_cover_ptr, herb_cover_len) } {
            Some(s) => s,
            None => return -3,
        };
        let herb_height = match unsafe { make_slice(herb_height_ptr, herb_height_len) } {
            Some(s) => s,
            None => return -3,
        };
        let shrub_cover = match unsafe { make_slice(shrub_cover_ptr, shrub_cover_len) } {
            Some(s) => s,
            None => return -3,
        };
        let shrub_height = match unsafe { make_slice(shrub_height_ptr, shrub_height_len) } {
            Some(s) => s,
            None => return -3,
        };
        let shrub_stems = match unsafe { make_slice_mut(shrub_stems_ptr, shrub_stems_len) } {
            Some(s) => s,
            None => return -3,
        };
        let shrub_heights = match unsafe { make_slice(shrub_heights_ptr, shrub_heights_len) } {
            Some(s) => s,
            None => return -3,
        };
        let shrub_covers = match unsafe { make_slice(shrub_covers_ptr, shrub_covers_len) } {
            Some(s) => s,
            None => return -3,
        };
        let shrub_species = match unsafe { make_slice(shrub_species_ptr, shrub_species_len) } {
            Some(s) => s,
            None => return -3,
        };
        let ref_cf1 = match unsafe { make_slice(ref_cf1_ptr, ref_cf1_len) } {
            Some(s) => s,
            None => return -3,
        };
        let ref_cf2 = match unsafe { make_slice(ref_cf2_ptr, ref_cf2_len) } {
            Some(s) => s,
            None => return -3,
        };

        super::rust_batch_evaluate_biomass(
            herb_biomass,
            herb_cover,
            herb_height,
            shrub_cover,
            shrub_height,
            shrub_stems,
            shrub_heights,
            shrub_covers,
            shrub_species,
            ref_cf1,
            ref_cf2,
        )
    }

    /// Dual symbol alias forwarding to `rust_batch_evaluate_biomass`.
    #[no_mangle]
    pub unsafe extern "C" fn rvs_batch_evaluate_biomass(
        herb_biomass_ptr: *mut f64,
        herb_biomass_len: usize,
        herb_cover_ptr: *const f64,
        herb_cover_len: usize,
        herb_height_ptr: *const f64,
        herb_height_len: usize,
        shrub_cover_ptr: *const f64,
        shrub_cover_len: usize,
        shrub_height_ptr: *const f64,
        shrub_height_len: usize,
        shrub_stems_ptr: *mut f64,
        shrub_stems_len: usize,
        shrub_heights_ptr: *const f64,
        shrub_heights_len: usize,
        shrub_covers_ptr: *const f64,
        shrub_covers_len: usize,
        shrub_species_ptr: *const u32,
        shrub_species_len: usize,
        ref_cf1_ptr: *const f64,
        ref_cf1_len: usize,
        ref_cf2_ptr: *const f64,
        ref_cf2_len: usize,
    ) -> i32 {
        unsafe {
            rust_batch_evaluate_biomass(
                herb_biomass_ptr,
                herb_biomass_len,
                herb_cover_ptr,
                herb_cover_len,
                herb_height_ptr,
                herb_height_len,
                shrub_cover_ptr,
                shrub_cover_len,
                shrub_height_ptr,
                shrub_height_len,
                shrub_stems_ptr,
                shrub_stems_len,
                shrub_heights_ptr,
                shrub_heights_len,
                shrub_covers_ptr,
                shrub_covers_len,
                shrub_species_ptr,
                shrub_species_len,
                ref_cf1_ptr,
                ref_cf1_len,
                ref_cf2_ptr,
                ref_cf2_len,
            )
        }
    }

    // -------------------------------------------------------------------------
    // Additional Batch C-ABI Endpoints (Complete Coverage)
    // -------------------------------------------------------------------------

    #[no_mangle]
    pub unsafe extern "C" fn rust_batch_evaluate_fuels(
        out_1hr_fuel_ptr: *mut f64,
        out_1hr_fuel_len: usize,
        cells_herb_biomass_ptr: *const f64,
        cells_herb_biomass_len: usize,
        cure_rate: f64,
    ) -> i32 {
        let out_1hr_fuel = match unsafe { make_slice_mut(out_1hr_fuel_ptr, out_1hr_fuel_len) } {
            Some(s) => s,
            None => return -3,
        };
        let cells_herb_biomass = match unsafe { make_slice(cells_herb_biomass_ptr, cells_herb_biomass_len) } {
            Some(s) => s,
            None => return -3,
        };
        super::rust_batch_evaluate_fuels(out_1hr_fuel, cells_herb_biomass, cure_rate)
    }

    #[no_mangle]
    pub unsafe extern "C" fn rvs_batch_evaluate_fuels(
        out_1hr_fuel_ptr: *mut f64,
        out_1hr_fuel_len: usize,
        cells_herb_biomass_ptr: *const f64,
        cells_herb_biomass_len: usize,
        cure_rate: f64,
    ) -> i32 {
        unsafe { rust_batch_evaluate_fuels(out_1hr_fuel_ptr, out_1hr_fuel_len, cells_herb_biomass_ptr, cells_herb_biomass_len, cure_rate) }
    }

    #[no_mangle]
    pub unsafe extern "C" fn rust_batch_evaluate_succession(
        shrub_height_ptr: *mut f64,
        shrub_height_len: usize,
        shrub_cover_ptr: *mut f64,
        shrub_cover_len: usize,
        delta_height_ptr: *const f64,
        delta_height_len: usize,
        delta_cover_ptr: *const f64,
        delta_cover_len: usize,
    ) -> i32 {
        let shrub_height = match unsafe { make_slice_mut(shrub_height_ptr, shrub_height_len) } {
            Some(s) => s,
            None => return -3,
        };
        let shrub_cover = match unsafe { make_slice_mut(shrub_cover_ptr, shrub_cover_len) } {
            Some(s) => s,
            None => return -3,
        };
        let delta_height = match unsafe { make_slice(delta_height_ptr, delta_height_len) } {
            Some(s) => s,
            None => return -3,
        };
        let delta_cover = match unsafe { make_slice(delta_cover_ptr, delta_cover_len) } {
            Some(s) => s,
            None => return -3,
        };
        super::rust_batch_evaluate_succession(shrub_height, shrub_cover, delta_height, delta_cover)
    }

    #[no_mangle]
    pub unsafe extern "C" fn rvs_batch_evaluate_succession(
        shrub_height_ptr: *mut f64,
        shrub_height_len: usize,
        shrub_cover_ptr: *mut f64,
        shrub_cover_len: usize,
        delta_height_ptr: *const f64,
        delta_height_len: usize,
        delta_cover_ptr: *const f64,
        delta_cover_len: usize,
    ) -> i32 {
        unsafe {
            rust_batch_evaluate_succession(
                shrub_height_ptr, shrub_height_len, shrub_cover_ptr, shrub_cover_len,
                delta_height_ptr, delta_height_len, delta_cover_ptr, delta_cover_len,
            )
        }
    }

    #[no_mangle]
    pub unsafe extern "C" fn rust_batch_evaluate_disturbance_graze(
        herb_biomass_ptr: *mut f64,
        herb_biomass_len: usize,
        herb_height_ptr: *mut f64,
        herb_height_len: usize,
        utilization_pct: f64,
    ) -> i32 {
        let herb_biomass = match unsafe { make_slice_mut(herb_biomass_ptr, herb_biomass_len) } {
            Some(s) => s,
            None => return -3,
        };
        let herb_height = match unsafe { make_slice_mut(herb_height_ptr, herb_height_len) } {
            Some(s) => s,
            None => return -3,
        };
        super::rust_batch_evaluate_disturbance_graze(herb_biomass, herb_height, utilization_pct)
    }

    #[no_mangle]
    pub unsafe extern "C" fn rvs_batch_evaluate_disturbance_graze(
        herb_biomass_ptr: *mut f64,
        herb_biomass_len: usize,
        herb_height_ptr: *mut f64,
        herb_height_len: usize,
        utilization_pct: f64,
    ) -> i32 {
        unsafe { rust_batch_evaluate_disturbance_graze(herb_biomass_ptr, herb_biomass_len, herb_height_ptr, herb_height_len, utilization_pct) }
    }

    // -------------------------------------------------------------------------
    // Unit Tests for C-ABI Export Wrappers
    // -------------------------------------------------------------------------
    #[cfg(test)]
    mod tests {
        use super::*;

        #[test]
        fn test_c_abi_export_wrappers() {
            // 1. S03: rust_calc_s2b_quadratic_form
            let x = [1.0, 2.0];
            let sigma = [0.1, 0.02, 0.02, 0.1];
            let val = unsafe { rust_calc_s2b_quadratic_form(x.as_ptr(), x.len(), sigma.as_ptr(), sigma.len(), 2) };
            let val_rvs = unsafe { rvs_calc_s2b_quadratic_form(x.as_ptr(), x.len(), sigma.as_ptr(), sigma.len(), 2) };
            assert_eq!(val, val_rvs);
            assert!(val > 0.0);

            // Null pointer handling
            let val_null = unsafe { rust_calc_s2b_quadratic_form(std::ptr::null(), 0, sigma.as_ptr(), sigma.len(), 2) };
            assert_eq!(val_null, 0.0);

            // Dimension 0 handling
            let val_dim0 = unsafe { rust_calc_s2b_quadratic_form(x.as_ptr(), x.len(), sigma.as_ptr(), sigma.len(), 0) };
            assert_eq!(val_dim0, 0.0);

            // 2. B01-B22: rust_batch_evaluate_biomass
            let mut herb_biomass = vec![200.0; 10];
            let herb_cover = vec![30.0; 10];
            let herb_height = vec![25.0; 10];
            let shrub_cover = vec![20.0; 10];
            let shrub_height = vec![50.0; 10];

            let mut shrub_stems = vec![0.0; 25];
            let shrub_heights = vec![45.0; 25];
            let shrub_covers = vec![15.0; 25];
            let shrub_species = vec![0u32; 25];
            let ref_cf1 = vec![0.85];
            let ref_cf2 = vec![1.42];

            let status = unsafe {
                rust_batch_evaluate_biomass(
                    herb_biomass.as_mut_ptr(), herb_biomass.len(),
                    herb_cover.as_ptr(), herb_cover.len(),
                    herb_height.as_ptr(), herb_height.len(),
                    shrub_cover.as_ptr(), shrub_cover.len(),
                    shrub_height.as_ptr(), shrub_height.len(),
                    shrub_stems.as_mut_ptr(), shrub_stems.len(),
                    shrub_heights.as_ptr(), shrub_heights.len(),
                    shrub_covers.as_ptr(), shrub_covers.len(),
                    shrub_species.as_ptr(), shrub_species.len(),
                    ref_cf1.as_ptr(), ref_cf1.len(),
                    ref_cf2.as_ptr(), ref_cf2.len(),
                )
            };
            assert_eq!(status, 0);
            assert!(herb_biomass[0] > 0.0);
            assert!(shrub_stems[0] > 0.0);

            // Null pointer non-empty buffer validation
            let null_status = unsafe {
                rust_batch_evaluate_biomass(
                    std::ptr::null_mut(), 10,
                    herb_cover.as_ptr(), herb_cover.len(),
                    herb_height.as_ptr(), herb_height.len(),
                    shrub_cover.as_ptr(), shrub_cover.len(),
                    shrub_height.as_ptr(), shrub_height.len(),
                    shrub_stems.as_mut_ptr(), shrub_stems.len(),
                    shrub_heights.as_ptr(), shrub_heights.len(),
                    shrub_covers.as_ptr(), shrub_covers.len(),
                    shrub_species.as_ptr(), shrub_species.len(),
                    ref_cf1.as_ptr(), ref_cf1.len(),
                    ref_cf2.as_ptr(), ref_cf2.len(),
                )
            };
            assert_eq!(null_status, -3);
        }
    }
}

