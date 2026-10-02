//! Deterministic unit test suite covering 100% of the 46 pure mathematical
//! subroutines across Biomass (B01–B22), Fuels (F01–F12), Succession (S01–S06),
//! and Disturbance (D01–D06).

use std::f64::consts::PI;
use rvs_core::biomass::*;
use rvs_core::fuels::*;
use rvs_core::succession::*;
use rvs_core::disturbance::*;

const EPSILON: f64 = 1e-6;

// =========================================================================
// Module 1: Biomass Unit Tests (B01–B22)
// =========================================================================

#[test]
fn test_b01_eq_165_linear_cover() {
    // Nominal
    let res = eq_165(10.0, 2.5, 20.0);
    assert!((res - 60.0).abs() < EPSILON);

    // Edge case: zero cover -> intercept
    assert!((eq_165(15.0, 3.0, 0.0) - 15.0).abs() < EPSILON);

    // Domain guard: negative cover clamped to 0.0 -> intercept
    assert!((eq_165(10.0, 2.5, -5.0) - 10.0).abs() < EPSILON);

    // NaN guard
    assert_eq!(eq_165(f64::NAN, 1.0, 10.0), 0.0);
    assert_eq!(eq_165(10.0, 1.0, f64::NAN), 0.0);
}

#[test]
fn test_b02_eq_201_exponential_cover() {
    // Nominal: exp(0.5 + 0.05 * 10) = exp(1.0)
    let res = eq_201(0.5, 0.05, 10.0);
    assert!((res - std::f64::consts::E).abs() < EPSILON);

    // Edge case: zero cover -> exp(cf1)
    let res_zero = eq_201(1.0, 0.5, 0.0);
    assert!((res_zero - 1.0f64.exp()).abs() < EPSILON);

    // Domain guard: negative cover clamped to 0.0
    let res_neg = eq_201(1.0, 0.5, -10.0);
    assert!((res_neg - 1.0f64.exp()).abs() < EPSILON);

    // Overflow protection
    let res_overflow = eq_201(500.0, 10.0, 100.0);
    assert_eq!(res_overflow, f64::MAX);
}

#[test]
fn test_b03_eq_202_power_law_cover() {
    // Nominal: exp(1.0 + 2.0 * ln(5)) = exp(1.0) * 25
    let res = eq_202(1.0, 2.0, 5.0);
    let expected = 1.0f64.exp() * 25.0;
    assert!((res - expected).abs() < 1e-4);

    // Domain guard: cover <= 0.0 returns 0.0
    assert_eq!(eq_202(1.0, 2.0, 0.0), 0.0);
    assert_eq!(eq_202(1.0, 2.0, -10.0), 0.0);

    // NaN guard
    assert_eq!(eq_202(1.0, 2.0, f64::NAN), 0.0);
}

#[test]
fn test_b04_eq_636_ellipsoidal_volume() {
    // Nominal: 100 * 80 * 50 = 400,000 cm3 = 400 dm3
    let res = eq_636(1.0, 0.8, 100.0, 80.0, 50.0);
    let expected = (1.0 + 0.8 * (400.0f64).ln()).exp();
    assert!((res - expected).abs() < EPSILON);

    // Domain guards: any dimension <= 0.0 returns 0.0
    assert_eq!(eq_636(1.0, 0.8, 0.0, 80.0, 50.0), 0.0);
    assert_eq!(eq_636(1.0, 0.8, 100.0, -5.0, 50.0), 0.0);
    assert_eq!(eq_636(1.0, 0.8, 100.0, 80.0, 0.0), 0.0);
}

#[test]
fn test_b05_eq_636_2_direct_volume() {
    // Nominal: 400,000 cm3 -> 400 dm3
    let res = eq_636_2(1.0, 0.8, 400_000.0);
    let expected = (1.0 + 0.8 * (400.0f64).ln()).exp();
    assert!((res - expected).abs() < EPSILON);

    // Volume = 1000 cm3 -> 1 dm3, ln(1) = 0 -> exp(cf1)
    let res_1000 = eq_636_2(2.0, 1.5, 1000.0);
    assert!((res_1000 - 2.0f64.exp()).abs() < EPSILON);

    // Domain guards
    assert_eq!(eq_636_2(1.0, 0.8, 0.0), 0.0);
    assert_eq!(eq_636_2(1.0, 0.8, -500.0), 0.0);
}

#[test]
fn test_b06_eq_743_canopy_interaction() {
    // Nominal
    let res = eq_743(5.0, 0.02, 30.0, 40.0);
    assert!((res - (5.0 + 0.02 * 1200.0)).abs() < EPSILON);

    // Zero inputs -> intercept
    assert!((eq_743(5.0, 0.02, 0.0, 40.0) - 5.0).abs() < EPSILON);
    assert!((eq_743(5.0, 0.02, 30.0, 0.0) - 5.0).abs() < EPSILON);

    // Negative inputs clamped to 0.0
    assert!((eq_743(5.0, 0.02, -10.0, 40.0) - 5.0).abs() < EPSILON);
}

#[test]
fn test_b07_eq_998_shrub_width_log10() {
    // Nominal: 10^(1.2 + 0.9 * log10(50))
    let res = eq_998(1.2, 0.9, 50.0);
    let expected = 10.0f64.powf(1.2 + 0.9 * 50.0f64.log10());
    assert!((res - expected).abs() < EPSILON);

    // Width = 1.0 -> log10(1) = 0 -> 10^cf1
    assert!((eq_998(2.0, 1.5, 1.0) - 100.0).abs() < EPSILON);

    // Domain guard: width <= 0.0
    assert_eq!(eq_998(1.2, 0.9, 0.0), 0.0);
    assert_eq!(eq_998(1.2, 0.9, -5.0), 0.0);
}

#[test]
fn test_b08_eq_999_bivariate_log10() {
    // Nominal
    let res = eq_999(0.5, 0.6, 0.4, 40.0, 30.0);
    let expected = 10.0f64.powf(0.5 + 0.6 * 40.0f64.log10() + 0.4 * 30.0f64.log10());
    assert!((res - expected).abs() < EPSILON);

    // Domain guard: either dimension <= 0.0
    assert_eq!(eq_999(0.5, 0.6, 0.4, 0.0, 30.0), 0.0);
    assert_eq!(eq_999(0.5, 0.6, 0.4, 40.0, -1.0), 0.0);
}

#[test]
fn test_b09_eq_1000_trivariate_log10() {
    // Nominal
    let res = eq_1000(0.2, 0.3, 0.3, 0.4, 40.0, 30.0, 50.0);
    let expected = 10.0f64.powf(0.2 + 0.3 * 40.0f64.log10() + 0.3 * 30.0f64.log10() + 0.4 * 50.0f64.log10());
    assert!((res - expected).abs() < EPSILON);

    // Domain guard
    assert_eq!(eq_1000(0.2, 0.3, 0.3, 0.4, 0.0, 30.0, 50.0), 0.0);
    assert_eq!(eq_1000(0.2, 0.3, 0.3, 0.4, 40.0, 0.0, 50.0), 0.0);
    assert_eq!(eq_1000(0.2, 0.3, 0.3, 0.4, 40.0, 30.0, -5.0), 0.0);
}

#[test]
fn test_b10_eq_1002_scaled_volume() {
    // Nominal: 2.0 + 0.5 * (10000 / 1000) = 7.0
    let res = eq_1002(2.0, 0.5, 10000.0);
    assert!((res - 7.0).abs() < EPSILON);

    // Zero volume -> cf1
    assert!((eq_1002(2.0, 0.5, 0.0) - 2.0).abs() < EPSILON);

    // Negative volume clamped to 0 -> cf1
    assert!((eq_1002(2.0, 0.5, -500.0) - 2.0).abs() < EPSILON);
}

#[test]
fn test_b11_eq_1008_trivariate_ln_exp() {
    // Nominal
    let res = eq_1008(0.1, 0.2, 0.3, 0.4, 10.0, 15.0, 20.0);
    let expected = (0.1 + 0.2 * 10.0f64.ln() + 0.3 * 15.0f64.ln() + 0.4 * 20.0f64.ln()).exp();
    assert!((res - expected).abs() < EPSILON);

    // Domain guard: any dimension <= 0.0
    assert_eq!(eq_1008(0.1, 0.2, 0.3, 0.4, 0.0, 15.0, 20.0), 0.0);
    assert_eq!(eq_1008(0.1, 0.2, 0.3, 0.4, 10.0, -1.0, 20.0), 0.0);
}

#[test]
fn test_b12_eq_1012_bivariate_ln_exp() {
    // Nominal
    let res = eq_1012(0.5, 0.8, 0.4, 25.0, 35.0);
    let expected = (0.5 + 0.8 * 25.0f64.ln() + 0.4 * 35.0f64.ln()).exp();
    assert!((res - expected).abs() < EPSILON);

    // Domain guard
    assert_eq!(eq_1012(0.5, 0.8, 0.4, 0.0, 35.0), 0.0);
    assert_eq!(eq_1012(0.5, 0.8, 0.4, 25.0, -5.0), 0.0);
}

#[test]
fn test_b13_eq_1058_poly_sqrt() {
    // Nominal: 10 + 2 * 16 + 5 * sqrt(16) = 10 + 32 + 20 = 62
    let res = eq_1058(10.0, 2.0, 5.0, 16.0);
    assert!((res - 62.0).abs() < EPSILON);

    // Zero input -> cf1
    assert!((eq_1058(10.0, 2.0, 5.0, 0.0) - 10.0).abs() < EPSILON);

    // Negative input clamped to 0 -> cf1
    assert!((eq_1058(10.0, 2.0, 5.0, -4.0) - 10.0).abs() < EPSILON);
}

#[test]
fn test_b14_eq_basic_bat() {
    // Nominal: exp(0.5 + 1.2 * ln(30 + 20))
    let res = eq_basic_bat(0.5, 1.2, 30.0, 20.0);
    let expected = (0.5 + 1.2 * 50.0f64.ln()).exp();
    assert!((res - expected).abs() < EPSILON);

    // Domain guard: length <= 0 or width <= 0 or sum <= 0
    assert_eq!(eq_basic_bat(0.5, 1.2, 0.0, 20.0), 0.0);
    assert_eq!(eq_basic_bat(0.5, 1.2, 30.0, -5.0), 0.0);
}

#[test]
fn test_b15_eq_1153_area_height_log10() {
    // Nominal: 10^(0.3 + 0.5 * log10(200) + 0.4 * log10(15))
    let res = eq_1153(0.3, 0.5, 0.4, 20.0, 10.0, 15.0);
    let expected = 10.0f64.powf(0.3 + 0.5 * 200.0f64.log10() + 0.4 * 15.0f64.log10());
    assert!((res - expected).abs() < EPSILON);

    // Domain guard
    assert_eq!(eq_1153(0.3, 0.5, 0.4, 0.0, 10.0, 15.0), 0.0);
    assert_eq!(eq_1153(0.3, 0.5, 0.4, 20.0, 10.0, 0.0), 0.0);
}

#[test]
fn test_b16_eq_1160_linear_param() {
    // Nominal: 15.0 + 3.5 * 10.0 = 50.0
    let res = eq_1160(15.0, 3.5, 10.0);
    assert!((res - 50.0).abs() < EPSILON);

    // Negative clamped
    assert!((eq_1160(15.0, 3.5, -10.0) - 15.0).abs() < EPSILON);
}

#[test]
fn test_b17_eq_1161_scaled_ratio() {
    // Nominal: 12.0 * (8.0 / 4.0) = 24.0
    let res = eq_1161(12.0, 4.0, 8.0);
    assert!((res - 24.0).abs() < EPSILON);

    // Division by zero guard
    assert_eq!(eq_1161(12.0, 0.0, 8.0), 0.0);

    // Negative input clamped
    assert_eq!(eq_1161(12.0, 4.0, -8.0), 0.0);
}

#[test]
fn test_b18_eq_pch_crown_area() {
    // Nominal: 10^(1.0 + 1.5 * log10(100)) = 10^(1.0 + 3.0) = 10000
    let res = eq_pch(1.0, 1.5, 100.0);
    assert!((res - 10000.0).abs() < EPSILON);

    // Domain guard: height <= 0
    assert_eq!(eq_pch(1.0, 1.5, 0.0), 0.0);
    assert_eq!(eq_pch(1.0, 1.5, -10.0), 0.0);
}

#[test]
fn test_b19_calc_crown_width() {
    // Nominal: 2 * sqrt(10000 / PI) ≈ 112.8379167
    let res = calc_crown_width(10000.0);
    let expected = (10000.0 / PI).sqrt() * 2.0;
    assert!((res - expected).abs() < EPSILON);

    // Domain guard: area <= 0
    assert_eq!(calc_crown_width(0.0), 0.0);
    assert_eq!(calc_crown_width(-100.0), 0.0);
}

#[test]
fn test_b20_calc_stems_per_acre_pure() {
    // Nominal: single_stem_area = 10000 cm2, cover = 25%, height = 10 cm
    // expanded = 4046.8564224 / 1.0 = 4046.8564224
    // stems = 4046.8564224 * 0.25 = 1011.7141056
    // reduction = exp(0.0809 * 10) * 0.0053 ≈ 2.24566 * 0.0053 ≈ 0.0119020
    // reduction <= 0.453595949 -> stems *= reduction ≈ 12.0414
    let res = calc_stems_per_acre_pure(10000.0, 25.0, 10.0);
    assert!(res > 11.0 && res < 13.0);

    // Zero cover or zero stem area
    assert_eq!(calc_stems_per_acre_pure(0.0, 25.0, 10.0), 0.0);
    assert_eq!(calc_stems_per_acre_pure(10000.0, 0.0, 10.0), 0.0);
    assert_eq!(calc_stems_per_acre_pure(-100.0, 25.0, 10.0), 0.0);
}

#[test]
fn test_b21_calc_herb_holdover() {
    // Nominal: 100 * 0.3 + 80 * 0.1 + 60 * 0.05 = 30 + 8 + 3 = 41
    let res = calc_herb_holdover(100.0, 80.0, 60.0);
    assert!((res - 41.0).abs() < EPSILON);

    // Slice variant
    let arr = [100.0, 80.0, 60.0];
    assert!((calc_herb_holdover_slice(&arr) - 41.0).abs() < EPSILON);

    // Negative terms clamped
    assert!((calc_herb_holdover(-50.0, 80.0, 60.0) - 11.0).abs() < EPSILON);
}

#[test]
fn test_b22_calc_attenuation() {
    // Nominal: hc*hh = 1200, sc*sh = 1000 -> ratio = 1000/1200 = 0.8333
    // Attenuated = 50 * (1000/1200) ≈ 41.6667
    let res = calc_attenuation(50.0, 30.0, 40.0, 20.0, 50.0);
    assert!((res - (50.0 * (1000.0 / 1200.0))).abs() < EPSILON);

    // Shrub index zero -> returns unattenuated herb_b
    assert!((calc_attenuation(50.0, 30.0, 40.0, 0.0, 50.0) - 50.0).abs() < EPSILON);
    assert!((calc_attenuation(50.0, 30.0, 40.0, 20.0, 0.0) - 50.0).abs() < EPSILON);

    // Herb index zero -> returns herb_b
    assert!((calc_attenuation(50.0, 0.0, 40.0, 20.0, 50.0) - 50.0).abs() < EPSILON);
}

// =========================================================================
// Module 2: Fuels Unit Tests (F01–F12)
// =========================================================================

#[test]
fn test_f01_fuels_eq_1() {
    // Nominal: 2.0 + 0.5 * 40.0 = 22.0
    assert!((fuels_eq_1(2.0, 0.5, 40.0) - 22.0).abs() < EPSILON);

    // Negative clamped
    assert!((fuels_eq_1(2.0, 0.5, -10.0) - 2.0).abs() < EPSILON);
}

#[test]
fn test_f02_fuels_eq_2() {
    // Nominal: exp(0.5 + 1.5 * ln(20))
    let res = fuels_eq_2(0.5, 1.5, 20.0);
    let expected = (0.5 + 1.5 * 20.0f64.ln()).exp();
    assert!((res - expected).abs() < EPSILON);

    // Non-positive guard
    assert_eq!(fuels_eq_2(0.5, 1.5, 0.0), 0.0);
    assert_eq!(fuels_eq_2(0.5, 1.5, -5.0), 0.0);
}

#[test]
fn test_f03_fuels_eq_13() {
    // Nominal: exp(0.1 + 0.02 * 50) = exp(1.1)
    let res = fuels_eq_13(0.1, 0.02, 50.0);
    assert!((res - 1.1f64.exp()).abs() < EPSILON);

    // Negative clamped
    assert!((fuels_eq_13(0.1, 0.02, -10.0) - 0.1f64.exp()).abs() < EPSILON);
}

#[test]
fn test_f04_fuels_eq_15() {
    // Nominal: 1.0 + 0.01 * 20 * 30 = 1.0 + 6.0 = 7.0
    let res = fuels_eq_15(1.0, 0.01, 20.0, 30.0);
    assert!((res - 7.0).abs() < EPSILON);

    // Negative clamped
    assert!((fuels_eq_15(1.0, 0.01, -5.0, 30.0) - 1.0).abs() < EPSILON);
}

#[test]
fn test_f05_fuels_eq_17() {
    // Nominal
    let res = fuels_eq_17(0.2, 0.4, 0.6, 15.0, 25.0);
    let expected = (0.2 + 0.4 * 15.0f64.ln() + 0.6 * 25.0f64.ln()).exp();
    assert!((res - expected).abs() < EPSILON);

    // Domain guard
    assert_eq!(fuels_eq_17(0.2, 0.4, 0.6, 0.0, 25.0), 0.0);
    assert_eq!(fuels_eq_17(0.2, 0.4, 0.6, 15.0, -1.0), 0.0);
}

#[test]
fn test_f06_fuels_eq_25() {
    // Nominal
    let res = fuels_eq_25(0.1, 0.2, 0.3, 0.4, 10.0, 20.0, 30.0);
    let expected = 10.0f64.powf(0.1 + 0.2 * 10.0f64.log10() + 0.3 * 20.0f64.log10() + 0.4 * 30.0f64.log10());
    assert!((res - expected).abs() < EPSILON);

    // Domain guard
    assert_eq!(fuels_eq_25(0.1, 0.2, 0.3, 0.4, 0.0, 20.0, 30.0), 0.0);
}

#[test]
fn test_f07_fuels_eq_33() {
    // Nominal: 5.0 + 10 * (2.0 + 0.1 * 10) = 5 + 10 * 3 = 35
    let res = fuels_eq_33(5.0, 2.0, 0.1, 10.0);
    assert!((res - 35.0).abs() < EPSILON);

    // Negative clamped
    assert!((fuels_eq_33(5.0, 2.0, 0.1, -10.0) - 5.0).abs() < EPSILON);
}

#[test]
fn test_f08_calc_fuel_component_wb() {
    // Nominal: 100 * 0.6 * 0.5 = 30
    assert!((calc_fuel_component_wb(100.0, 0.6) - 30.0).abs() < EPSILON);

    // Proportion clamped to [0, 1]
    assert!((calc_fuel_component_wb(100.0, 1.5) - 50.0).abs() < EPSILON);
    assert_eq!(calc_fuel_component_wb(100.0, -0.5), 0.0);
}

#[test]
fn test_f09_calc_fuel_component_fl() {
    // Nominal: 100 * 0.4 * 0.5 = 20
    assert!((calc_fuel_component_fl(100.0, 0.4) - 20.0).abs() < EPSILON);

    // Proportion clamped
    assert!((calc_fuel_component_fl(100.0, 1.2) - 50.0).abs() < EPSILON);
    assert_eq!(calc_fuel_component_fl(100.0, -0.2), 0.0);
}

#[test]
fn test_f10_calc_total_1hr_fuel() {
    // Nominal: 30 + 20 + 15 = 65
    assert!((calc_total_1hr_fuel(30.0, 20.0, 15.0) - 65.0).abs() < EPSILON);

    // Negative components clamped
    assert!((calc_total_1hr_fuel(-5.0, 20.0, 15.0) - 35.0).abs() < EPSILON);
}

#[test]
fn test_f11_calc_fuel_percentages() {
    // Nominal: 20, 30, 30, 20 -> total = 100, p = 20%, 30%, 30%, 20%
    let (p1, p10, p100, p1000, total) = calc_fuel_percentages(20.0, 30.0, 30.0, 20.0);
    assert!((total - 100.0).abs() < EPSILON);
    assert!((p1 - 20.0).abs() < EPSILON);
    assert!((p10 - 30.0).abs() < EPSILON);
    assert!((p100 - 30.0).abs() < EPSILON);
    assert!((p1000 - 20.0).abs() < EPSILON);

    // All zero or negative -> returns (0, 0, 0, 0, 0)
    let (z1, z10, z100, z1000, z_tot) = calc_fuel_percentages(0.0, 0.0, 0.0, 0.0);
    assert_eq!((z1, z10, z100, z1000, z_tot), (0.0, 0.0, 0.0, 0.0, 0.0));
}

#[test]
fn test_f12_calc_herb_loading() {
    // Nominal: 80 * 0.75 = 60
    assert!((calc_herb_loading(80.0, 0.75) - 60.0).abs() < EPSILON);

    // Cure rate clamped
    assert!((calc_herb_loading(80.0, 1.2) - 80.0).abs() < EPSILON);
    assert_eq!(calc_herb_loading(80.0, -0.5), 0.0);
}

// =========================================================================
// Module 3: Succession Unit Tests (S01–S06)
// =========================================================================

#[test]
fn test_s01_calc_growth_rate() {
    // Nominal
    assert!((calc_growth_rate(50.0, 2.5) - 52.5).abs() < EPSILON);

    // Clamped at 0.0
    assert_eq!(calc_growth_rate(10.0, -25.0), 0.0);

    // NaN guard
    assert_eq!(calc_growth_rate(f64::NAN, 5.0), 0.0);
}

#[test]
fn test_s01b_calc_plot_age() {
    // cover = 25, min = 5, rate = 2.0, start = 10 -> ((25 - 5) / 2) + 10 = 20
    assert!((calc_plot_age(25.0, 5.0, 2.0, 10.0) - 20.0).abs() < EPSILON);

    // Zero or negative growth rate -> start_age
    assert!((calc_plot_age(25.0, 5.0, 0.0, 10.0) - 10.0).abs() < EPSILON);
    assert!((calc_plot_age(25.0, 5.0, -1.0, 10.0) - 10.0).abs() < EPSILON);
}

#[test]
fn test_s02_evaluate_herb_growth() {
    // Nominal
    let ppt = 450.0;
    let ndvi = 0.45;
    let b0 = -5.2;
    let b1 = 0.11;
    let b2 = 1.38;
    let b3 = 0.05;
    let res = evaluate_herb_growth(b0, b1, b2, b3, ppt, ndvi);
    let expected = (b0 + b1 * ppt.ln() + b2 * ndvi.ln() + b3 * ppt.ln() * ndvi.ln()).exp();
    assert!((res - expected).abs() < EPSILON);

    // Domain guards: non-positive ppt or ndvi
    assert_eq!(evaluate_herb_growth(b0, b1, b2, b3, 0.0, ndvi), 0.0);
    assert_eq!(evaluate_herb_growth(b0, b1, b2, b3, ppt, -0.1), 0.0);
}

#[test]
fn test_s03_calc_s2b_quadratic_form() {
    // 2D quadratic form: x = [1, 2], Sigma = [[2, 0.5], [0.5, 3]]
    // inner = [2*1 + 0.5*2, 0.5*1 + 3*2] = [3, 6.5]
    // sum = 1*3 + 2*6.5 = 3 + 13 = 16
    let x = [1.0, 2.0];
    let sigma = [2.0, 0.5, 0.5, 3.0];
    let res = calc_s2b_quadratic_form(&x, &sigma, 2);
    assert!((res - 16.0).abs() < EPSILON);

    // Length mismatch or zero dim -> returns 0.0 safely
    assert_eq!(calc_s2b_quadratic_form(&x, &sigma, 0), 0.0);
    assert_eq!(calc_s2b_quadratic_form(&x[..1], &sigma, 2), 0.0);
}

#[test]
fn test_s04_calc_herb_biomass_production() {
    // Nominal: mean = 500, s2b = 100 (sd = 10), z = 1.5 -> 500 + 15 = 515
    let res = calc_herb_biomass_production(500.0, 100.0, 1.5);
    assert!((res - 515.0).abs() < EPSILON);

    // Negative biomass clamped to 0.0
    assert_eq!(calc_herb_biomass_production(20.0, 100.0, -5.0), 0.0);

    // Negative variance clamped
    assert_eq!(calc_herb_biomass_production(50.0, -10.0, 2.0), 50.0);
}

#[test]
fn test_s05_scale_shrub_height() {
    // Nominal
    assert!((scale_shrub_height(80.0, 5.0) - 85.0).abs() < EPSILON);

    // Decrement beyond zero clamped
    assert_eq!(scale_shrub_height(50.0, -60.0), 0.0);
}

#[test]
fn test_s06_scale_shrub_cover() {
    // Nominal
    assert!((scale_shrub_cover(45.0, 10.0) - 55.0).abs() < EPSILON);

    // Clamped upper bound at 100.0
    assert_eq!(scale_shrub_cover(90.0, 25.0), 100.0);

    // Clamped lower bound at 0.0
    assert_eq!(scale_shrub_cover(10.0, -20.0), 0.0);
}

// =========================================================================
// Module 4: Disturbance Unit Tests (D01–D06)
// =========================================================================

#[test]
fn test_d01_calc_graze_biomass_reduction() {
    // Nominal: pre = 200, util = 40% -> 200 * 0.6 = 120
    assert!((calc_graze_biomass_reduction(200.0, 40.0) - 120.0).abs() < EPSILON);

    // 0% util -> full biomass
    assert!((calc_graze_biomass_reduction(200.0, 0.0) - 200.0).abs() < EPSILON);

    // 100% or more -> 0.0
    assert_eq!(calc_graze_biomass_reduction(200.0, 100.0), 0.0);
    assert_eq!(calc_graze_biomass_reduction(200.0, 150.0), 0.0);
}

#[test]
fn test_d02_calc_graze_height_reduction() {
    // Nominal: pre = 50, util = 30% -> 50 * (0.7)^0.7 ≈ 38.953
    let res = calc_graze_height_reduction(50.0, 30.0);
    let expected = 50.0 * 0.7f64.powf(0.7);
    assert!((res - expected).abs() < EPSILON);

    // 100% util -> 0.0
    assert_eq!(calc_graze_height_reduction(50.0, 100.0), 0.0);
}

#[test]
fn test_d03_calc_mow_biomass_residual() {
    // Nominal: pre_b = 150, pre_h = 60, mow_h = 20 -> 150 * (20/60) = 50
    assert!((calc_mow_biomass_residual(150.0, 60.0, 20.0) - 50.0).abs() < EPSILON);

    // mow_h >= pre_h -> retain full biomass
    assert!((calc_mow_biomass_residual(150.0, 50.0, 60.0) - 150.0).abs() < EPSILON);

    // pre_h <= 0 -> 0.0
    assert_eq!(calc_mow_biomass_residual(150.0, 0.0, 20.0), 0.0);
}

#[test]
fn test_d04_calc_fire_biomass_consumption() {
    // Nominal: b = 500, k = 0.05, intensity = 20 -> 500 * (1 - e^-1) ≈ 316.060
    let res = calc_fire_biomass_consumption(500.0, 0.05, 20.0);
    let expected = 500.0 * (1.0 - (-1.0f64).exp());
    assert!((res - expected).abs() < EPSILON);

    // Zero intensity -> 0.0
    assert_eq!(calc_fire_biomass_consumption(500.0, 0.05, 0.0), 0.0);
}

#[test]
fn test_d05_calc_fire_shrub_mortality() {
    // Nominal: scorch = 60, alpha = -2.0, beta = 0.05 -> z = -2 + 3 = 1 -> 1 / (1 + e^-1) ≈ 0.731058
    let res = calc_fire_shrub_mortality(60.0, -2.0, 0.05);
    let expected = 1.0 / (1.0 + (-1.0f64).exp());
    assert!((res - expected).abs() < EPSILON);

    // Saturation branch guards
    assert_eq!(calc_fire_shrub_mortality(100.0, 50.0, 1.0), 1.0);
    assert_eq!(calc_fire_shrub_mortality(0.0, -50.0, 0.1), 0.0);
}

#[test]
fn test_d06_calc_fire_herb_post_regrowth() {
    // Nominal: resprout = 100, mort = 35% -> 100 * 0.65 = 65
    assert!((calc_fire_herb_post_regrowth(100.0, 35.0) - 65.0).abs() < EPSILON);

    // 100% mortality -> 0.0
    assert_eq!(calc_fire_herb_post_regrowth(100.0, 100.0), 0.0);
}

#[test]
fn test_aux_calc_graze_removal_amount() {
    // Nominal: intake = 25 lbs/day, animals = 10, days = 30, area = 10 acres
    // lbs_per_acre = (25 * 10 * 30) / 10 = 750 lbs/acre
    // grams = 750 * 453.59237 = 340194.2775
    let res = calc_graze_removal_amount(25.0, 10.0, 30.0, 10.0);
    let expected = 750.0 * 453.59237;
    assert!((res - expected).abs() < 1e-4);

    // Zero area -> 0.0
    assert_eq!(calc_graze_removal_amount(25.0, 10.0, 30.0, 0.0), 0.0);
}

#[test]
fn test_adversarial_infinities_biomass() {
    // B01: Inf + (-Inf)
    assert_eq!(eq_165(f64::INFINITY, f64::NEG_INFINITY, 1.0), 0.0);
    // B06: Inf cover and height
    assert_eq!(eq_743(f64::INFINITY, 1.0, f64::NEG_INFINITY, 2.0), 0.0);
    // B20: Stems per acre with Inf
    assert_eq!(calc_stems_per_acre_pure(f64::INFINITY, 50.0, 10.0), 0.0);
    // B22: Attenuation with Inf
    assert_eq!(calc_attenuation(f64::INFINITY, 50.0, 10.0, 50.0, 10.0), 0.0);
}

#[test]
fn test_adversarial_infinities_fuels() {
    // F01: Inf + (-Inf)
    assert_eq!(fuels_eq_1(f64::INFINITY, f64::NEG_INFINITY, 1.0), 0.0);
    // F08: Inf * 0.0
    assert_eq!(calc_fuel_component_wb(f64::INFINITY, 0.0), 0.0);
    // F10: Inf sum
    assert_eq!(calc_total_1hr_fuel(f64::INFINITY, 1.0, 1.0), 0.0);
    // F11: Inf component loadings
    let (p1, p10, p100, p1000, total) = calc_fuel_percentages(f64::INFINITY, 10.0, 10.0, 10.0);
    assert_eq!((p1, p10, p100, p1000, total), (0.0, 0.0, 0.0, 0.0, 0.0));
}

#[test]
fn test_adversarial_infinities_succession() {
    // S01: Inf + (-Inf)
    assert_eq!(calc_growth_rate(f64::INFINITY, f64::NEG_INFINITY), 0.0);
    // S01b: Inf - Inf
    assert_eq!(calc_plot_age(f64::INFINITY, f64::INFINITY, 2.0, 10.0), 0.0);
    // S02: Inf coefficients
    assert_eq!(evaluate_herb_growth(f64::INFINITY, f64::NEG_INFINITY, 1.0, 1.0, 500.0, 0.5), 0.0);
    // S03: Inf in matrix
    assert_eq!(calc_s2b_quadratic_form(&[f64::INFINITY, 1.0], &[1.0, 0.0, 0.0, 1.0], 2), 0.0);
    // S04: Inf mean with -Inf z_score
    assert_eq!(calc_herb_biomass_production(f64::INFINITY, 100.0, f64::NEG_INFINITY), 0.0);
    // S05: Inf height
    assert_eq!(scale_shrub_height(f64::INFINITY, -5.0), 0.0);
    // S06: Opposite-signed infinities in cover
    assert_eq!(scale_shrub_cover(f64::INFINITY, f64::NEG_INFINITY), 0.0);
}

#[test]
fn test_adversarial_infinities_disturbance() {
    // D01: Inf biomass with 100% util (Inf * 0.0)
    assert_eq!(calc_graze_biomass_reduction(f64::INFINITY, 100.0), 0.0);
    // D02: Inf height with 100% util (Inf * 0.0)
    assert_eq!(calc_graze_height_reduction(f64::INFINITY, 100.0), 0.0);
    // D03: Inf / Inf ratio
    assert_eq!(calc_mow_biomass_residual(100.0, f64::INFINITY, f64::INFINITY), 0.0);
    // D04: Inf biomass with 0 intensity (Inf * 0.0)
    assert_eq!(calc_fire_biomass_consumption(f64::INFINITY, 0.05, 0.0), 0.0);
    // D05: Inf + (-Inf) in logistic sigmoid
    assert_eq!(calc_fire_shrub_mortality(50.0, f64::INFINITY, f64::NEG_INFINITY), 0.0);
    // D06: Inf resprout with 100% mortality (Inf * 0.0)
    assert_eq!(calc_fire_herb_post_regrowth(f64::INFINITY, 100.0), 0.0);
    // Aux: Inf / Inf in graze removal
    assert_eq!(calc_graze_removal_amount(f64::INFINITY, 10.0, 30.0, f64::INFINITY), 0.0);
}

