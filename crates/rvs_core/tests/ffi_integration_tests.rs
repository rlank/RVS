//! Integration tests for safe CXX bidirectional FFI bridge routines.
//!
//! Verifies dual-symbol export parity (rust_* and rvs_*), zero-copy slice interfaces,
//! non-panicking error codes on buffer mismatches, and IEEE-754 domain safety.

use rvs_core::ffi::*;

#[test]
fn test_biomass_ffi_scalar_parity() {
    // Check dual exports for biomass routines
    let cf1 = 2.5;
    let cf2 = 1.8;
    let cover = 35.0;

    let r_165 = rust_eq_165(cf1, cf2, cover);
    let v_165 = rvs_eq_165(cf1, cf2, cover);
    assert_eq!(r_165, v_165);
    assert!(r_165 > 0.0);

    let r_201 = rust_eq_201(cf1, cf2, cover);
    let v_201 = rvs_eq_201(cf1, cf2, cover);
    assert_eq!(r_201, v_201);
    assert!(r_201 > 0.0);

    let r_202 = rust_eq_202(cf1, cf2, cover);
    let v_202 = rvs_eq_202(cf1, cf2, cover);
    assert_eq!(r_202, v_202);
    assert!(r_202 > 0.0);

    let r_636 = rust_eq_636(1.0, 1.2, 50.0, 40.0, 30.0);
    let v_636 = rvs_eq_636(1.0, 1.2, 50.0, 40.0, 30.0);
    assert_eq!(r_636, v_636);

    let r_636_2 = rust_eq_636_2(1.0, 1.2, 60000.0);
    let v_636_2 = rvs_eq_636_2(1.0, 1.2, 60000.0);
    assert_eq!(r_636_2, v_636_2);

    let r_743 = rust_eq_743(1.5, 0.8, 25.0, 45.0);
    let v_743 = rvs_eq_743(1.5, 0.8, 25.0, 45.0);
    assert_eq!(r_743, v_743);

    let r_cw = rust_calc_crown_width(500.0);
    let v_cw = rvs_calc_crown_width(500.0);
    assert_eq!(r_cw, v_cw);
    assert!(r_cw > 0.0);

    let r_stems = rust_calc_stems_per_acre_pure(500.0, 25.0, 60.0);
    let v_stems = rvs_calc_stems_per_acre_pure(500.0, 25.0, 60.0);
    assert_eq!(r_stems, v_stems);
    assert!(r_stems > 0.0);

    let r_hold = rust_calc_herb_holdover(100.0, 80.0, 60.0);
    let v_hold = rvs_calc_herb_holdover(100.0, 80.0, 60.0);
    assert_eq!(r_hold, v_hold);
    assert_eq!(r_hold, 0.3 * 100.0 + 0.1 * 80.0 + 0.05 * 60.0);

    let r_att = rust_calc_attenuation(250.0, 30.0, 40.0, 20.0, 50.0);
    let v_att = rvs_calc_attenuation(250.0, 30.0, 40.0, 20.0, 50.0);
    assert_eq!(r_att, v_att);
    assert!(r_att > 0.0);
}

#[test]
fn test_fuels_ffi_scalar_parity() {
    let r_f1 = rust_fuels_eq_1(0.5, 1.2, 30.0);
    let v_f1 = rvs_fuels_eq_1(0.5, 1.2, 30.0);
    assert_eq!(r_f1, v_f1);

    let r_f2 = rust_fuels_eq_2(0.5, 1.2, 30.0);
    let v_f2 = rvs_fuels_eq_2(0.5, 1.2, 30.0);
    assert_eq!(r_f2, v_f2);

    let r_f13 = rust_fuels_eq_13(0.5, 0.1, 20.0);
    let v_f13 = rvs_fuels_eq_13(0.5, 0.1, 20.0);
    assert_eq!(r_f13, v_f13);

    let r_wb = rust_calc_fuel_component_wb(150.0, 0.8);
    let v_wb = rvs_calc_fuel_component_wb(150.0, 0.8);
    assert_eq!(r_wb, v_wb);

    let r_fl = rust_calc_fuel_component_fl(150.0, 0.8);
    let v_fl = rvs_calc_fuel_component_fl(150.0, 0.8);
    assert_eq!(r_fl, v_fl);

    let r_tot = rust_calc_total_1hr_fuel(60.0, 40.0, 50.0);
    let v_tot = rvs_calc_total_1hr_fuel(60.0, 40.0, 50.0);
    assert_eq!(r_tot, v_tot);
    assert_eq!(r_tot, 150.0);

    let r_cure = rust_calc_herb_loading(200.0, 0.75);
    let v_cure = rvs_calc_herb_loading(200.0, 0.75);
    assert_eq!(r_cure, v_cure);
    assert_eq!(r_cure, 150.0);
}

#[test]
fn test_succession_and_disturbance_ffi_scalar_parity() {
    let r_gr = rust_calc_growth_rate(100.0, 0.05);
    let v_gr = rvs_calc_growth_rate(100.0, 0.05);
    assert_eq!(r_gr, v_gr);

    let r_age = rust_calc_plot_age(30.0, 5.0, 2.0, 1.0);
    let v_age = rvs_calc_plot_age(30.0, 5.0, 2.0, 1.0);
    assert_eq!(r_age, v_age);

    let x = [1.0, 5.5, 0.6];
    let sigma = [
        0.05, 0.01, 0.005,
        0.01, 0.04, 0.002,
        0.005, 0.002, 0.03,
    ];
    let r_s2b = rust_calc_s2b_quadratic_form(&x, &sigma, 3);
    let v_s2b = rvs_calc_s2b_quadratic_form(&x, &sigma, 3);
    assert_eq!(r_s2b, v_s2b);
    assert!(r_s2b > 0.0);

    let r_prod = rust_calc_herb_biomass_production(150.0, 0.05, 1.96);
    let v_prod = rvs_calc_herb_biomass_production(150.0, 0.05, 1.96);
    assert_eq!(r_prod, v_prod);

    let r_sh = rust_scale_shrub_height(50.0, 5.0);
    let v_sh = rvs_scale_shrub_height(50.0, 5.0);
    assert_eq!(r_sh, v_sh);

    let r_sc = rust_scale_shrub_cover(25.0, 3.0);
    let v_sc = rvs_scale_shrub_cover(25.0, 3.0);
    assert_eq!(r_sc, v_sc);

    let r_gb = rust_calc_graze_biomass_reduction(200.0, 40.0);
    let v_gb = rvs_calc_graze_biomass_reduction(200.0, 40.0);
    assert_eq!(r_gb, v_gb);

    let r_gh = rust_calc_graze_height_reduction(40.0, 40.0);
    let v_gh = rvs_calc_graze_height_reduction(40.0, 40.0);
    assert_eq!(r_gh, v_gh);

    let r_fire = rust_calc_fire_biomass_consumption(300.0, 0.02, 50.0);
    let v_fire = rvs_calc_fire_biomass_consumption(300.0, 0.02, 50.0);
    assert_eq!(r_fire, v_fire);

    let r_mort = rust_calc_fire_shrub_mortality(80.0, 0.05, 0.08);
    let v_mort = rvs_calc_fire_shrub_mortality(80.0, 0.05, 0.08);
    assert_eq!(r_mort, v_mort);
}

#[test]
fn test_batch_evaluate_biomass_ffi_execution() {
    let mut herb_biomass = vec![200.0; 100];
    let herb_cover = vec![30.0; 100];
    let herb_height = vec![25.0; 100];
    let shrub_cover = vec![20.0; 100];
    let shrub_height = vec![50.0; 100];

    let mut shrub_stems = vec![0.0; 250];
    let shrub_heights = vec![45.0; 250];
    let shrub_covers = vec![15.0; 250];
    let shrub_species = vec![0u32; 250];
    let ref_cf1 = vec![0.85];
    let ref_cf2 = vec![1.42];

    let status = rust_batch_evaluate_biomass(
        &mut herb_biomass,
        &herb_cover,
        &herb_height,
        &shrub_cover,
        &shrub_height,
        &mut shrub_stems,
        &shrub_heights,
        &shrub_covers,
        &shrub_species,
        &ref_cf1,
        &ref_cf2,
    );

    assert_eq!(status, 0);
    assert!(herb_biomass[0] > 0.0);
    assert!(shrub_stems[0] > 0.0);

    // Dimension mismatch testing (must return negative code, NEVER panic)
    let bad_shrub_heights = vec![45.0; 200]; // Expected 250
    let err_status = rust_batch_evaluate_biomass(
        &mut herb_biomass,
        &herb_cover,
        &herb_height,
        &shrub_cover,
        &shrub_height,
        &mut shrub_stems,
        &bad_shrub_heights,
        &shrub_covers,
        &shrub_species,
        &ref_cf1,
        &ref_cf2,
    );
    assert_eq!(err_status, -2);

    let bad_herb_cover = vec![30.0; 50]; // Expected 100
    let err_status_cell = rust_batch_evaluate_biomass(
        &mut herb_biomass,
        &bad_herb_cover,
        &herb_height,
        &shrub_cover,
        &shrub_height,
        &mut shrub_stems,
        &shrub_heights,
        &shrub_covers,
        &shrub_species,
        &ref_cf1,
        &ref_cf2,
    );
    assert_eq!(err_status_cell, -1);
}

#[test]
fn test_batch_evaluate_fuels_ffi_execution() {
    let cells_herb_biomass = vec![250.0; 50];
    let mut out_1hr_fuel = vec![0.0; 50];

    let status = rust_batch_evaluate_fuels(&mut out_1hr_fuel, &cells_herb_biomass, 0.8);
    assert_eq!(status, 0);
    assert_eq!(out_1hr_fuel[0], 200.0);

    // Dimension mismatch
    let bad_biomass = vec![250.0; 40];
    let err_status = rust_batch_evaluate_fuels(&mut out_1hr_fuel, &bad_biomass, 0.8);
    assert_eq!(err_status, -1);

    // Non-finite cure rate safety
    let status_nan = rust_batch_evaluate_fuels(&mut out_1hr_fuel, &cells_herb_biomass, f64::NAN);
    assert_eq!(status_nan, 0);
    assert_eq!(out_1hr_fuel[0], 0.0);
}

#[test]
fn test_batch_evaluate_succession_ffi_execution() {
    let mut shrub_height = vec![50.0; 60];
    let mut shrub_cover = vec![20.0; 60];
    let delta_height = vec![2.0; 60];
    let delta_cover = vec![1.5; 60];

    let status = rust_batch_evaluate_succession(
        &mut shrub_height,
        &mut shrub_cover,
        &delta_height,
        &delta_cover,
    );
    assert_eq!(status, 0);
    assert_eq!(shrub_height[0], 52.0);
    assert_eq!(shrub_cover[0], 21.5);

    // Scalar overload
    let status_scalar = rust_batch_evaluate_succession_scalar(
        &mut shrub_height,
        &mut shrub_cover,
        1.0,
        0.5,
    );
    assert_eq!(status_scalar, 0);
    assert_eq!(shrub_height[0], 53.0);
    assert_eq!(shrub_cover[0], 22.0);

    // Mismatch check
    let bad_delta = vec![1.0; 10];
    let err = rust_batch_evaluate_succession(
        &mut shrub_height,
        &mut shrub_cover,
        &bad_delta,
        &delta_cover,
    );
    assert_eq!(err, -1);
}

#[test]
fn test_batch_evaluate_disturbance_graze_ffi_execution() {
    let mut herb_biomass = vec![300.0; 80];
    let mut herb_height = vec![40.0; 80];

    let status = rust_batch_evaluate_disturbance_graze(
        &mut herb_biomass,
        &mut herb_height,
        50.0,
    );
    assert_eq!(status, 0);
    assert!(herb_biomass[0] < 300.0);
    assert!(herb_height[0] < 40.0);

    // Mismatch check
    let mut bad_height = vec![40.0; 70];
    let err = rust_batch_evaluate_disturbance_graze(
        &mut herb_biomass,
        &mut bad_height,
        50.0,
    );
    assert_eq!(err, -1);

    // Non-finite utilization check
    let status_nan = rust_batch_evaluate_disturbance_graze(
        &mut herb_biomass,
        &mut herb_height,
        f64::NAN,
    );
    assert_eq!(status_nan, 0);
    assert_eq!(herb_biomass[0], 0.0);
    assert_eq!(herb_height[0], 0.0);
}

#[test]
fn test_batch_calc_attenuation_ffi_bounds_safety() {
    let mut herb_b = vec![200.0; 20];
    let hc = vec![30.0; 20];
    let hh = vec![25.0; 20];
    let sc = vec![20.0; 20];
    let sh = vec![50.0; 20];

    rust_batch_calc_attenuation(&mut herb_b, &hc, &hh, &sc, &sh);
    assert!(herb_b[0] > 0.0);

    // Mismatched length safely ignored without panic
    let bad_hc = vec![30.0; 10];
    rust_batch_calc_attenuation(&mut herb_b, &bad_hc, &hh, &sc, &sh);
    // Should not panic, herb_b intact
    assert!(herb_b[0] > 0.0);
}

#[test]
fn test_noexcept_domain_safety() {
    // Negative cover in log models returns 0.0 safely
    assert_eq!(rust_eq_202(1.0, 1.0, -999.0), 0.0);
    assert_eq!(rust_eq_636(1.0, 1.0, -10.0, 10.0, 10.0), 0.0);
    assert_eq!(rust_fuels_eq_2(1.0, 1.0, -5.0), 0.0);

    // NaN / Inf returns 0.0 safely
    assert_eq!(rust_eq_165(f64::NAN, 1.0, 10.0), 0.0);
    assert_eq!(rust_eq_165(1.0, f64::INFINITY, 10.0), 0.0);
    assert_eq!(rust_fuels_eq_1(1.0, 1.0, f64::NAN), 0.0);
    assert_eq!(rust_calc_crown_width(f64::NAN), 0.0);
    assert_eq!(rust_calc_stems_per_acre_pure(f64::NAN, 10.0, 10.0), 0.0);
}

#[test]
fn test_batch_empty_buffers_all_endpoints() {
    // 1. rust_batch_evaluate_biomass with 0 cells and 0 shrubs
    let mut herb_biomass: Vec<f64> = vec![];
    let herb_cover: Vec<f64> = vec![];
    let herb_height: Vec<f64> = vec![];
    let shrub_cover: Vec<f64> = vec![];
    let shrub_height: Vec<f64> = vec![];
    let mut shrub_stems: Vec<f64> = vec![];
    let shrub_heights: Vec<f64> = vec![];
    let shrub_covers: Vec<f64> = vec![];
    let shrub_species: Vec<u32> = vec![];
    let ref_cf1: Vec<f64> = vec![];
    let ref_cf2: Vec<f64> = vec![];

    let res = rust_batch_evaluate_biomass(
        &mut herb_biomass, &herb_cover, &herb_height, &shrub_cover, &shrub_height,
        &mut shrub_stems, &shrub_heights, &shrub_covers, &shrub_species,
        &ref_cf1, &ref_cf2,
    );
    assert_eq!(res, 0);

    let res_rvs = rvs_batch_evaluate_biomass(
        &mut herb_biomass, &herb_cover, &herb_height, &shrub_cover, &shrub_height,
        &mut shrub_stems, &shrub_heights, &shrub_covers, &shrub_species,
        &ref_cf1, &ref_cf2,
    );
    assert_eq!(res_rvs, 0);

    // 0 cells, but 10 shrubs
    let mut shrub_stems_10 = vec![0.0; 10];
    let shrub_heights_10 = vec![45.0; 10];
    let shrub_covers_10 = vec![25.0; 10];
    let shrub_species_10 = vec![0u32; 10];
    let cf1 = vec![0.8];
    let cf2 = vec![1.5];
    let res_cells0_shrubs10 = rust_batch_evaluate_biomass(
        &mut herb_biomass, &herb_cover, &herb_height, &shrub_cover, &shrub_height,
        &mut shrub_stems_10, &shrub_heights_10, &shrub_covers_10, &shrub_species_10,
        &cf1, &cf2,
    );
    assert_eq!(res_cells0_shrubs10, 0);
    assert!(shrub_stems_10[0] > 0.0);

    // 10 cells, but 0 shrubs
    let mut herb_biomass_10 = vec![200.0; 10];
    let herb_cover_10 = vec![30.0; 10];
    let herb_height_10 = vec![20.0; 10];
    let shrub_cover_10 = vec![15.0; 10];
    let shrub_height_10 = vec![40.0; 10];
    let res_cells10_shrubs0 = rust_batch_evaluate_biomass(
        &mut herb_biomass_10, &herb_cover_10, &herb_height_10, &shrub_cover_10, &shrub_height_10,
        &mut shrub_stems, &shrub_heights, &shrub_covers, &shrub_species,
        &cf1, &cf2,
    );
    assert_eq!(res_cells10_shrubs0, 0);
    assert!(herb_biomass_10[0] > 0.0);

    // 2. rust_batch_evaluate_fuels with 0 cells
    let mut fuels_out: Vec<f64> = vec![];
    let fuels_cells: Vec<f64> = vec![];
    assert_eq!(rust_batch_evaluate_fuels(&mut fuels_out, &fuels_cells, 0.75), 0);
    assert_eq!(rvs_batch_evaluate_fuels(&mut fuels_out, &fuels_cells, 0.75), 0);

    // 3. rust_batch_evaluate_succession with 0 shrubs
    let mut succ_h: Vec<f64> = vec![];
    let mut succ_c: Vec<f64> = vec![];
    let succ_dh: Vec<f64> = vec![];
    let succ_dc: Vec<f64> = vec![];
    assert_eq!(rust_batch_evaluate_succession(&mut succ_h, &mut succ_c, &succ_dh, &succ_dc), 0);
    assert_eq!(rvs_batch_evaluate_succession(&mut succ_h, &mut succ_c, &succ_dh, &succ_dc), 0);
    assert_eq!(rust_batch_evaluate_succession_scalar(&mut succ_h, &mut succ_c, 1.0, 0.5), 0);
    assert_eq!(rvs_batch_evaluate_succession_scalar(&mut succ_h, &mut succ_c, 1.0, 0.5), 0);

    // 4. rust_batch_evaluate_disturbance_graze with 0 cells
    let mut graze_b: Vec<f64> = vec![];
    let mut graze_h: Vec<f64> = vec![];
    assert_eq!(rust_batch_evaluate_disturbance_graze(&mut graze_b, &mut graze_h, 50.0), 0);
    assert_eq!(rvs_batch_evaluate_disturbance_graze(&mut graze_b, &mut graze_h, 50.0), 0);

    // 5. rust_batch_calc_attenuation with 0 cells
    rust_batch_calc_attenuation(&mut herb_biomass, &herb_cover, &herb_height, &shrub_cover, &shrub_height);
    rvs_batch_calc_attenuation(&mut herb_biomass, &herb_cover, &herb_height, &shrub_cover, &shrub_height);
}

#[test]
fn test_batch_exhaustive_mismatch_matrix() {
    let mut hb = vec![200.0; 10];
    let hc = vec![30.0; 10];
    let hh = vec![20.0; 10];
    let sc = vec![15.0; 10];
    let sh = vec![40.0; 10];

    let mut ss = vec![0.0; 6];
    let ssh = vec![45.0; 6];
    let ssc = vec![20.0; 6];
    let ssp = vec![0u32; 6];
    let cf1 = vec![0.8];
    let cf2 = vec![1.5];

    // Biomass cell mismatches -> must return -1
    let bad_hc = vec![30.0; 9];
    assert_eq!(rust_batch_evaluate_biomass(&mut hb, &bad_hc, &hh, &sc, &sh, &mut ss, &ssh, &ssc, &ssp, &cf1, &cf2), -1);
    assert_eq!(rvs_batch_evaluate_biomass(&mut hb, &bad_hc, &hh, &sc, &sh, &mut ss, &ssh, &ssc, &ssp, &cf1, &cf2), -1);

    let bad_hh = vec![20.0; 8];
    assert_eq!(rust_batch_evaluate_biomass(&mut hb, &hc, &bad_hh, &sc, &sh, &mut ss, &ssh, &ssc, &ssp, &cf1, &cf2), -1);

    let bad_sc = vec![15.0; 7];
    assert_eq!(rust_batch_evaluate_biomass(&mut hb, &hc, &hh, &bad_sc, &sh, &mut ss, &ssh, &ssc, &ssp, &cf1, &cf2), -1);

    let bad_sh = vec![40.0; 5];
    assert_eq!(rust_batch_evaluate_biomass(&mut hb, &hc, &hh, &sc, &bad_sh, &mut ss, &ssh, &ssc, &ssp, &cf1, &cf2), -1);

    // Biomass shrub mismatches -> must return -2
    let bad_ssh = vec![45.0; 5];
    assert_eq!(rust_batch_evaluate_biomass(&mut hb, &hc, &hh, &sc, &sh, &mut ss, &bad_ssh, &ssc, &ssp, &cf1, &cf2), -2);
    assert_eq!(rvs_batch_evaluate_biomass(&mut hb, &hc, &hh, &sc, &sh, &mut ss, &bad_ssh, &ssc, &ssp, &cf1, &cf2), -2);

    let bad_ssc = vec![20.0; 4];
    assert_eq!(rust_batch_evaluate_biomass(&mut hb, &hc, &hh, &sc, &sh, &mut ss, &ssh, &bad_ssc, &ssp, &cf1, &cf2), -2);

    let bad_ssp = vec![0u32; 3];
    assert_eq!(rust_batch_evaluate_biomass(&mut hb, &hc, &hh, &sc, &sh, &mut ss, &ssh, &ssc, &bad_ssp, &cf1, &cf2), -2);

    // Fuels mismatch -> must return -1
    let mut out_fuel = vec![0.0; 10];
    let bad_fuel_in = vec![100.0; 9];
    assert_eq!(rust_batch_evaluate_fuels(&mut out_fuel, &bad_fuel_in, 0.8), -1);
    assert_eq!(rvs_batch_evaluate_fuels(&mut out_fuel, &bad_fuel_in, 0.8), -1);

    // Succession mismatches -> must return -1
    let mut succ_h = vec![50.0; 10];
    let mut succ_c = vec![20.0; 10];
    let succ_dh = vec![2.0; 10];
    let succ_dc = vec![1.0; 10];

    let mut bad_succ_c = vec![20.0; 9];
    assert_eq!(rust_batch_evaluate_succession(&mut succ_h, &mut bad_succ_c, &succ_dh, &succ_dc), -1);
    assert_eq!(rvs_batch_evaluate_succession(&mut succ_h, &mut bad_succ_c, &succ_dh, &succ_dc), -1);

    let bad_succ_dh = vec![2.0; 8];
    assert_eq!(rust_batch_evaluate_succession(&mut succ_h, &mut succ_c, &bad_succ_dh, &succ_dc), -1);

    let bad_succ_dc = vec![1.0; 7];
    assert_eq!(rust_batch_evaluate_succession(&mut succ_h, &mut succ_c, &succ_dh, &bad_succ_dc), -1);

    // Succession scalar mismatch -> must return -1
    let mut bad_succ_c2 = vec![20.0; 5];
    assert_eq!(rust_batch_evaluate_succession_scalar(&mut succ_h, &mut bad_succ_c2, 1.0, 0.5), -1);
    assert_eq!(rvs_batch_evaluate_succession_scalar(&mut succ_h, &mut bad_succ_c2, 1.0, 0.5), -1);

    // Disturbance graze mismatch -> must return -1
    let mut graze_b = vec![250.0; 10];
    let mut bad_graze_h = vec![30.0; 8];
    assert_eq!(rust_batch_evaluate_disturbance_graze(&mut graze_b, &mut bad_graze_h, 40.0), -1);
    assert_eq!(rvs_batch_evaluate_disturbance_graze(&mut graze_b, &mut bad_graze_h, 40.0), -1);
}

#[test]
fn test_batch_floating_point_anomalies_and_sanitization() {
    // 1. Fuels anomalies
    let mut fuel_out = vec![0.0; 4];
    let fuel_cells = vec![100.0, -50.0, f64::NAN, f64::INFINITY];

    // Negative cure rate -> clamped to 0.0
    assert_eq!(rust_batch_evaluate_fuels(&mut fuel_out, &fuel_cells, -0.5), 0);
    assert_eq!(fuel_out, vec![0.0, 0.0, 0.0, 0.0]);

    // Excess cure rate (> 1.0) -> clamped to 1.0
    assert_eq!(rust_batch_evaluate_fuels(&mut fuel_out, &fuel_cells, 2.5), 0);
    assert_eq!(fuel_out[0], 100.0); // 100 * 1.0
    assert_eq!(fuel_out[1], 0.0);   // negative clamped
    assert_eq!(fuel_out[2], 0.0);   // NaN sanitized
    assert_eq!(fuel_out[3], 0.0);   // Inf sanitized

    // NaN cure rate -> all 0.0
    assert_eq!(rust_batch_evaluate_fuels(&mut fuel_out, &fuel_cells, f64::NAN), 0);
    assert_eq!(fuel_out, vec![0.0, 0.0, 0.0, 0.0]);

    // Inf cure rate -> all 0.0
    assert_eq!(rust_batch_evaluate_fuels(&mut fuel_out, &fuel_cells, f64::INFINITY), 0);
    assert_eq!(fuel_out, vec![0.0, 0.0, 0.0, 0.0]);

    // 2. Grazing disturbance anomalies
    let mut graze_b = vec![200.0, -10.0, f64::NAN, f64::INFINITY];
    let mut graze_h = vec![40.0, -5.0, f64::NAN, f64::INFINITY];

    // Negative utilization -> clamped to 0.0 (no reduction)
    assert_eq!(rust_batch_evaluate_disturbance_graze(&mut graze_b, &mut graze_h, -50.0), 0);
    assert_eq!(graze_b[0], 200.0);
    assert_eq!(graze_b[1], 0.0);
    assert_eq!(graze_b[2], 0.0);
    assert_eq!(graze_b[3], 0.0);
    assert_eq!(graze_h[0], 40.0);
    assert_eq!(graze_h[1], 0.0);
    assert_eq!(graze_h[2], 0.0);
    assert_eq!(graze_h[3], 0.0);

    // Excess utilization (> 100.0) -> clamped to 100.0 (100% defoliation -> 0.0)
    let mut graze_b2 = vec![200.0; 4];
    let mut graze_h2 = vec![40.0; 4];
    assert_eq!(rust_batch_evaluate_disturbance_graze(&mut graze_b2, &mut graze_h2, 150.0), 0);
    assert_eq!(graze_b2, vec![0.0, 0.0, 0.0, 0.0]);
    assert_eq!(graze_h2, vec![0.0, 0.0, 0.0, 0.0]);

    // NaN utilization -> all 0.0
    let mut graze_b3 = vec![200.0; 4];
    let mut graze_h3 = vec![40.0; 4];
    assert_eq!(rust_batch_evaluate_disturbance_graze(&mut graze_b3, &mut graze_h3, f64::NAN), 0);
    assert_eq!(graze_b3, vec![0.0, 0.0, 0.0, 0.0]);
    assert_eq!(graze_h3, vec![0.0, 0.0, 0.0, 0.0]);

    // Inf utilization -> all 0.0
    let mut graze_b4 = vec![200.0; 4];
    let mut graze_h4 = vec![40.0; 4];
    assert_eq!(rust_batch_evaluate_disturbance_graze(&mut graze_b4, &mut graze_h4, f64::INFINITY), 0);
    assert_eq!(graze_b4, vec![0.0, 0.0, 0.0, 0.0]);
    assert_eq!(graze_h4, vec![0.0, 0.0, 0.0, 0.0]);

    // 3. Species ID out of bounds / empty reference tables in Biomass
    let mut hb = vec![200.0; 2];
    let hc = vec![30.0; 2];
    let hh = vec![20.0; 2];
    let sc = vec![15.0; 2];
    let sh = vec![40.0; 2];
    let mut ss = vec![0.0; 2];
    let ssh = vec![45.0; 2];
    let ssc = vec![20.0; 2];
    let ssp = vec![99999u32; 2]; // Out of bounds
    let empty_cf1: Vec<f64> = vec![];
    let empty_cf2: Vec<f64> = vec![];

    let res_oob = rust_batch_evaluate_biomass(
        &mut hb, &hc, &hh, &sc, &sh,
        &mut ss, &ssh, &ssc, &ssp,
        &empty_cf1, &empty_cf2,
    );
    assert_eq!(res_oob, 0);
    assert!(ss[0] > 0.0);
    assert!(ss[1] > 0.0);
}

#[test]
fn test_dual_symbol_exhaustive_all_routines_parity() {
    // Biomass subroutines B01 - B18
    assert_eq!(rust_eq_165(1.5, 2.0, 30.0), rvs_eq_165(1.5, 2.0, 30.0));
    assert_eq!(rust_eq_201(1.5, 0.05, 30.0), rvs_eq_201(1.5, 0.05, 30.0));
    assert_eq!(rust_eq_202(1.5, 0.8, 30.0), rvs_eq_202(1.5, 0.8, 30.0));
    assert_eq!(rust_eq_636(1.0, 1.2, 50.0, 40.0, 30.0), rvs_eq_636(1.0, 1.2, 50.0, 40.0, 30.0));
    assert_eq!(rust_eq_636_2(1.0, 1.2, 60000.0), rvs_eq_636_2(1.0, 1.2, 60000.0));
    assert_eq!(rust_eq_743(1.5, 0.8, 25.0, 45.0), rvs_eq_743(1.5, 0.8, 25.0, 45.0));
    assert_eq!(rust_eq_998(0.5, 1.5, 30.0), rvs_eq_998(0.5, 1.5, 30.0));
    assert_eq!(rust_eq_999(0.5, 1.2, 0.8, 40.0, 30.0), rvs_eq_999(0.5, 1.2, 0.8, 40.0, 30.0));
    assert_eq!(rust_eq_1000(0.5, 1.1, 0.9, 0.8, 40.0, 30.0, 50.0), rvs_eq_1000(0.5, 1.1, 0.9, 0.8, 40.0, 30.0, 50.0));
    assert_eq!(rust_eq_1002(10.0, 2.5, 500.0), rvs_eq_1002(10.0, 2.5, 500.0));
    assert_eq!(rust_eq_1008(0.5, 0.8, 0.7, 0.6, 40.0, 30.0, 50.0), rvs_eq_1008(0.5, 0.8, 0.7, 0.6, 40.0, 30.0, 50.0));
    assert_eq!(rust_eq_1012(0.5, 0.8, 0.7, 40.0, 30.0), rvs_eq_1012(0.5, 0.8, 0.7, 40.0, 30.0));
    assert_eq!(rust_eq_1058(2.0, 0.5, 1.2, 30.0), rvs_eq_1058(2.0, 0.5, 1.2, 30.0));
    assert_eq!(rust_eq_basic_bat(1.2, 0.9, 40.0, 30.0), rvs_eq_basic_bat(1.2, 0.9, 40.0, 30.0));
    assert_eq!(rust_eq_1153(0.5, 1.2, 0.8, 40.0, 30.0, 50.0), rvs_eq_1153(0.5, 1.2, 0.8, 40.0, 30.0, 50.0));
    assert_eq!(rust_eq_1160(5.0, 2.5, 20.0), rvs_eq_1160(5.0, 2.5, 20.0));
    assert_eq!(rust_eq_1161(10.0, 5.0, 20.0), rvs_eq_1161(10.0, 5.0, 20.0));
    assert_eq!(rust_eq_pch(0.8, 1.4, 45.0), rvs_eq_pch(0.8, 1.4, 45.0));

    // Biomass drivers B19 - B22
    assert_eq!(rust_calc_crown_width(500.0), rvs_calc_crown_width(500.0));
    assert_eq!(rust_calc_stems_per_acre_pure(500.0, 25.0, 60.0), rvs_calc_stems_per_acre_pure(500.0, 25.0, 60.0));
    assert_eq!(rust_calc_herb_holdover(100.0, 80.0, 60.0), rvs_calc_herb_holdover(100.0, 80.0, 60.0));
    assert_eq!(rust_calc_attenuation(250.0, 30.0, 40.0, 20.0, 50.0), rvs_calc_attenuation(250.0, 30.0, 40.0, 20.0, 50.0));

    // Fuels F01 - F12
    assert_eq!(rust_fuels_eq_1(0.5, 1.2, 30.0), rvs_fuels_eq_1(0.5, 1.2, 30.0));
    assert_eq!(rust_fuels_eq_2(0.5, 1.2, 30.0), rvs_fuels_eq_2(0.5, 1.2, 30.0));
    assert_eq!(rust_fuels_eq_13(0.5, 0.1, 20.0), rvs_fuels_eq_13(0.5, 0.1, 20.0));
    assert_eq!(rust_fuels_eq_15(0.5, 0.1, 20.0, 30.0), rvs_fuels_eq_15(0.5, 0.1, 20.0, 30.0));
    assert_eq!(rust_fuels_eq_17(0.5, 0.1, 0.2, 20.0, 30.0), rvs_fuels_eq_17(0.5, 0.1, 0.2, 20.0, 30.0));
    assert_eq!(rust_fuels_eq_25(0.5, 0.1, 0.2, 0.3, 20.0, 30.0, 40.0), rvs_fuels_eq_25(0.5, 0.1, 0.2, 0.3, 20.0, 30.0, 40.0));
    assert_eq!(rust_fuels_eq_33(0.5, 0.1, 0.2, 20.0), rvs_fuels_eq_33(0.5, 0.1, 0.2, 20.0));
    assert_eq!(rust_calc_fuel_component_wb(150.0, 0.8), rvs_calc_fuel_component_wb(150.0, 0.8));
    assert_eq!(rust_calc_fuel_component_fl(150.0, 0.8), rvs_calc_fuel_component_fl(150.0, 0.8));
    assert_eq!(rust_calc_total_1hr_fuel(60.0, 40.0, 50.0), rvs_calc_total_1hr_fuel(60.0, 40.0, 50.0));
    assert_eq!(rust_calc_herb_loading(200.0, 0.75), rvs_calc_herb_loading(200.0, 0.75));

    // Succession S01 - S06
    assert_eq!(rust_calc_growth_rate(100.0, 0.05), rvs_calc_growth_rate(100.0, 0.05));
    assert_eq!(rust_calc_plot_age(30.0, 5.0, 2.0, 1.0), rvs_calc_plot_age(30.0, 5.0, 2.0, 1.0));
    assert_eq!(rust_evaluate_herb_growth(1.0, 0.5, 0.2, 0.1, 400.0, 0.6), rvs_evaluate_herb_growth(1.0, 0.5, 0.2, 0.1, 400.0, 0.6));
    let x = [1.0, 2.0];
    let sigma = [0.1, 0.02, 0.02, 0.1];
    assert_eq!(rust_calc_s2b_quadratic_form(&x, &sigma, 2), rvs_calc_s2b_quadratic_form(&x, &sigma, 2));
    assert_eq!(rust_calc_herb_biomass_production(150.0, 0.05, 1.96), rvs_calc_herb_biomass_production(150.0, 0.05, 1.96));
    assert_eq!(rust_scale_shrub_height(50.0, 5.0), rvs_scale_shrub_height(50.0, 5.0));
    assert_eq!(rust_scale_shrub_cover(25.0, 3.0), rvs_scale_shrub_cover(25.0, 3.0));

    // Disturbance D01 - D06
    assert_eq!(rust_calc_graze_biomass_reduction(200.0, 40.0), rvs_calc_graze_biomass_reduction(200.0, 40.0));
    assert_eq!(rust_calc_graze_height_reduction(40.0, 40.0), rvs_calc_graze_height_reduction(40.0, 40.0));
    assert_eq!(rust_calc_mow_biomass_residual(200.0, 30.0, 10.0), rvs_calc_mow_biomass_residual(200.0, 30.0, 10.0));
    assert_eq!(rust_calc_fire_biomass_consumption(300.0, 0.02, 50.0), rvs_calc_fire_biomass_consumption(300.0, 0.02, 50.0));
    assert_eq!(rust_calc_fire_shrub_mortality(80.0, 0.05, 0.08), rvs_calc_fire_shrub_mortality(80.0, 0.05, 0.08));
    assert_eq!(rust_calc_fire_herb_post_regrowth(0.8, 40.0), rvs_calc_fire_herb_post_regrowth(0.8, 40.0));
    assert_eq!(rust_calc_graze_removal_amount(26.0, 50.0, 30.0, 100.0), rvs_calc_graze_removal_amount(26.0, 50.0, 30.0, 100.0));
}
