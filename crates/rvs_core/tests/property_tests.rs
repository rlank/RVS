//! Complete Property-Based Testing Suite covering 100% of the 46 cataloged
//! mathematical subroutines across Biomass (B01–B22), Fuels (F01–F12),
//! Succession (S01–S06), and Disturbance (D01–D06).
//!
//! Validates:
//! 1. Panic-free execution across arbitrary IEEE-754 domains (NaN, ±Inf, extreme floats).
//! 2. Zero NaN-leakage across all arbitrary float inputs (`!res.is_nan()`).
//! 3. Guaranteed finite output across all inputs (`res.is_finite()`).
//! 4. Strict mathematical domain guards (non-positive inputs, zero divisors).
//! 5. Physical boundaries (cover in [0, 100], mortality in [0, 1], non-negative biomass and heights).

use proptest::prelude::*;
use rvs_core::biomass::*;
use rvs_core::fuels::*;
use rvs_core::succession::*;
use rvs_core::disturbance::*;

// Strategy for generating arbitrary float values including non-finite edge cases
fn arbitrary_f64() -> impl Strategy<Value = f64> {
    prop_oneof![
        Just(0.0),
        Just(-0.0),
        Just(1.0),
        Just(-1.0),
        Just(f64::NAN),
        Just(f64::INFINITY),
        Just(f64::NEG_INFINITY),
        Just(f64::MIN_POSITIVE),
        Just(f64::MAX),
        -1e6..1e6f64,
    ]
}

proptest! {
    // =========================================================================
    // 1. Biomass Invariant Properties (B01–B22 Complete Coverage)
    // =========================================================================

    #[test]
    fn prop_b01_eq_165(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cover in arbitrary_f64()) {
        let res = eq_165(cf1, cf2, cover);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        if !cf1.is_finite() || !cf2.is_finite() || !cover.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b02_eq_201(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cover in arbitrary_f64()) {
        let res = eq_201(cf1, cf2, cover);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_b03_eq_202(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cover in arbitrary_f64()) {
        let res = eq_202(cf1, cf2, cover);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if cover <= 0.0 || !cover.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b04_eq_636(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), l in arbitrary_f64(), w in arbitrary_f64(), h in arbitrary_f64()) {
        let res = eq_636(cf1, cf2, l, w, h);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if l <= 0.0 || w <= 0.0 || h <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b05_eq_636_2(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), vol in arbitrary_f64()) {
        let res = eq_636_2(cf1, cf2, vol);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if vol <= 0.0 || !vol.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b06_eq_743(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cover in arbitrary_f64(), height in arbitrary_f64()) {
        let res = eq_743(cf1, cf2, cover, height);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
    }

    #[test]
    fn prop_b07_eq_998(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), w in arbitrary_f64()) {
        let res = eq_998(cf1, cf2, w);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if w <= 0.0 || !w.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b08_eq_999(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), l in arbitrary_f64(), w in arbitrary_f64()) {
        let res = eq_999(cf1, cf2, cf3, l, w);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if l <= 0.0 || w <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b09_eq_1000(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), cf4 in arbitrary_f64(), l in arbitrary_f64(), w in arbitrary_f64(), h in arbitrary_f64()) {
        let res = eq_1000(cf1, cf2, cf3, cf4, l, w, h);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if l <= 0.0 || w <= 0.0 || h <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b10_eq_1002(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), vol in arbitrary_f64()) {
        let res = eq_1002(cf1, cf2, vol);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
    }

    #[test]
    fn prop_b11_eq_1008(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), cf4 in arbitrary_f64(), l in arbitrary_f64(), w in arbitrary_f64(), h in arbitrary_f64()) {
        let res = eq_1008(cf1, cf2, cf3, cf4, l, w, h);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if l <= 0.0 || w <= 0.0 || h <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b12_eq_1012(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), p1 in arbitrary_f64(), p2 in arbitrary_f64()) {
        let res = eq_1012(cf1, cf2, cf3, p1, p2);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if p1 <= 0.0 || p2 <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b13_eq_1058(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), p1 in arbitrary_f64()) {
        let res = eq_1058(cf1, cf2, cf3, p1);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
    }

    #[test]
    fn prop_b14_eq_basic_bat(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), l in arbitrary_f64(), w in arbitrary_f64()) {
        let res = eq_basic_bat(cf1, cf2, l, w);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if l <= 0.0 || w <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b15_eq_1153(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), l in arbitrary_f64(), w in arbitrary_f64(), h in arbitrary_f64()) {
        let res = eq_1153(cf1, cf2, cf3, l, w, h);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if l <= 0.0 || w <= 0.0 || h <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b16_eq_1160(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), p1 in arbitrary_f64()) {
        let res = eq_1160(cf1, cf2, p1);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
    }

    #[test]
    fn prop_b17_eq_1161(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), p1 in arbitrary_f64()) {
        let res = eq_1161(cf1, cf2, p1);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        if cf2 == 0.0 || !cf2.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b18_eq_pch(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), h in arbitrary_f64()) {
        let res = eq_pch(cf1, cf2, h);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if h <= 0.0 || !h.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b19_calc_crown_width(area in arbitrary_f64()) {
        let res = calc_crown_width(area);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if area <= 0.0 || !area.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b20_calc_stems_per_acre(stem_area in arbitrary_f64(), cover in arbitrary_f64(), h in arbitrary_f64()) {
        let res = calc_stems_per_acre_pure(stem_area, cover, h);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if stem_area <= 0.0 || cover <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_b21_calc_herb_holdover(p0 in arbitrary_f64(), p1 in arbitrary_f64(), p2 in arbitrary_f64()) {
        let res = calc_herb_holdover(p0, p1, p2);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        let slice_res = calc_herb_holdover_slice(&[p0, p1, p2]);
        prop_assert_eq!(res, slice_res);
    }

    #[test]
    fn prop_b22_calc_attenuation(herb_b in arbitrary_f64(), hc in arbitrary_f64(), hh in arbitrary_f64(), sc in arbitrary_f64(), sh in arbitrary_f64()) {
        let res = calc_attenuation(herb_b, hc, hh, sc, sh);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    // =========================================================================
    // 2. Fuels Invariant Properties (F01–F12 Complete Coverage)
    // =========================================================================

    #[test]
    fn prop_f01_fuels_eq_1(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), p1 in arbitrary_f64()) {
        let res = fuels_eq_1(cf1, cf2, p1);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
    }

    #[test]
    fn prop_f02_fuels_eq_2(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), p1 in arbitrary_f64()) {
        let res = fuels_eq_2(cf1, cf2, p1);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if p1 <= 0.0 || !p1.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_f03_fuels_eq_13(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), p1 in arbitrary_f64()) {
        let res = fuels_eq_13(cf1, cf2, p1);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_f04_fuels_eq_15(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), p1 in arbitrary_f64(), p2 in arbitrary_f64()) {
        let res = fuels_eq_15(cf1, cf2, p1, p2);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
    }

    #[test]
    fn prop_f05_fuels_eq_17(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), p1 in arbitrary_f64(), p2 in arbitrary_f64()) {
        let res = fuels_eq_17(cf1, cf2, cf3, p1, p2);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if p1 <= 0.0 || p2 <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_f06_fuels_eq_25(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), cf4 in arbitrary_f64(), p1 in arbitrary_f64(), p2 in arbitrary_f64(), p3 in arbitrary_f64()) {
        let res = fuels_eq_25(cf1, cf2, cf3, cf4, p1, p2, p3);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if p1 <= 0.0 || p2 <= 0.0 || p3 <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_f06b_fuels_eq_42(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), cf4 in arbitrary_f64(), p1 in arbitrary_f64(), p2 in arbitrary_f64(), p3 in arbitrary_f64()) {
        let res = fuels_eq_42(cf1, cf2, cf3, cf4, p1, p2, p3);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if p1 <= 0.0 || p2 <= 0.0 || p3 <= 0.0 {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_f07_fuels_eq_33(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), p1 in arbitrary_f64()) {
        let res = fuels_eq_33(cf1, cf2, cf3, p1);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
    }

    #[test]
    fn prop_f07b_fuels_eq_46(cf1 in arbitrary_f64(), cf2 in arbitrary_f64(), cf3 in arbitrary_f64(), p1 in arbitrary_f64(), p2 in arbitrary_f64()) {
        let res = fuels_eq_46(cf1, cf2, cf3, p1, p2);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
    }

    #[test]
    fn prop_f08_calc_fuel_component_wb(shrub_b in arbitrary_f64(), prop in arbitrary_f64()) {
        let res = calc_fuel_component_wb(shrub_b, prop);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_f09_calc_fuel_component_fl(shrub_b in arbitrary_f64(), prop in arbitrary_f64()) {
        let res = calc_fuel_component_fl(shrub_b, prop);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_f10_calc_total_1hr_fuel(wb in arbitrary_f64(), fl in arbitrary_f64(), herb in arbitrary_f64()) {
        let res = calc_total_1hr_fuel(wb, fl, herb);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_f11_calc_fuel_percentages(f1 in arbitrary_f64(), f10 in arbitrary_f64(), f100 in arbitrary_f64(), f1000 in arbitrary_f64()) {
        let (p1, p10, p100, p1000, tot) = calc_fuel_percentages(f1, f10, f100, f1000);
        prop_assert!(!p1.is_nan());
        prop_assert!(!p10.is_nan());
        prop_assert!(!p100.is_nan());
        prop_assert!(!p1000.is_nan());
        prop_assert!(!tot.is_nan());
        prop_assert!(p1.is_finite());
        prop_assert!(p10.is_finite());
        prop_assert!(p100.is_finite());
        prop_assert!(p1000.is_finite());
        prop_assert!(tot.is_finite());
        prop_assert!(p1 >= 0.0 && p1 <= 100.0);
        prop_assert!(p10 >= 0.0 && p10 <= 100.0);
        prop_assert!(p100 >= 0.0 && p100 <= 100.0);
        prop_assert!(p1000 >= 0.0 && p1000 <= 100.0);
    }

    #[test]
    fn prop_f12_calc_herb_loading(herb_b in arbitrary_f64(), cure in arbitrary_f64()) {
        let res = calc_herb_loading(herb_b, cure);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_fuels_helpers(b in arbitrary_f64(), stem in arbitrary_f64(), h in arbitrary_f64(), comp in arbitrary_f64(), tot in arbitrary_f64()) {
        let wb = calc_1hr_wood_bark(b);
        prop_assert!(!wb.is_nan());
        prop_assert!(wb.is_finite());

        let fl = calc_1hr_foliage(b);
        prop_assert!(!fl.is_nan());
        prop_assert!(fl.is_finite());

        let f1 = calc_1hr_fuel(b);
        prop_assert!(!f1.is_nan());
        prop_assert!(f1.is_finite());

        let f10 = calc_10hr_fuel(b, stem);
        prop_assert!(!f10.is_nan());
        prop_assert!(f10.is_finite());

        let f100 = calc_100hr_fuel(b);
        prop_assert!(!f100.is_nan());
        prop_assert!(f100.is_finite());

        let f1000 = calc_1000hr_fuel(h);
        prop_assert!(!f1000.is_nan());
        prop_assert!(f1000.is_finite());

        let prop = calc_fuel_proportion(comp, tot);
        prop_assert!(!prop.is_nan());
        prop_assert!(prop.is_finite());
        prop_assert!(prop >= 0.0 && prop <= 1.0);

        let pct = calc_fuel_percent(comp, tot);
        prop_assert!(!pct.is_nan());
        prop_assert!(pct.is_finite());
        prop_assert!(pct >= 0.0 && pct <= 100.0);
    }

    // =========================================================================
    // 3. Succession Invariant Properties (S01–S06 Complete Coverage)
    // =========================================================================

    #[test]
    fn prop_s01_calc_growth_rate(v in arbitrary_f64(), r in arbitrary_f64()) {
        let res = calc_growth_rate(v, r);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_s01b_calc_plot_age(c in arbitrary_f64(), min_c in arbitrary_f64(), gr in arbitrary_f64(), start in arbitrary_f64()) {
        let res = calc_plot_age(c, min_c, gr, start);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_s02_evaluate_herb_growth(b0 in arbitrary_f64(), b1 in arbitrary_f64(), b2 in arbitrary_f64(), b3 in arbitrary_f64(), ppt in arbitrary_f64(), ndvi in arbitrary_f64()) {
        let res = evaluate_herb_growth(b0, b1, b2, b3, ppt, ndvi);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if ppt <= 0.0 || ndvi <= 0.0 || !ppt.is_finite() || !ndvi.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_s03_calc_s2b_quadratic_form(x1 in arbitrary_f64(), x2 in arbitrary_f64(), s1 in arbitrary_f64(), s2 in arbitrary_f64(), s3 in arbitrary_f64(), s4 in arbitrary_f64()) {
        let x = [x1, x2];
        let sigma = [s1, s2, s3, s4];
        let res = calc_s2b_quadratic_form(&x, &sigma, 2);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_s04_calc_herb_biomass_production(mean in arbitrary_f64(), s2b in arbitrary_f64(), z in arbitrary_f64()) {
        let res = calc_herb_biomass_production(mean, s2b, z);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_s05_scale_shrub_height(h in arbitrary_f64(), delta in arbitrary_f64()) {
        let res = scale_shrub_height(h, delta);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_s06_scale_shrub_cover(cov in arbitrary_f64(), delta in arbitrary_f64()) {
        let res = scale_shrub_cover(cov, delta);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0 && res <= 100.0);
    }

    // =========================================================================
    // 4. Disturbance Invariant Properties (D01–D06 Complete Coverage)
    // =========================================================================

    #[test]
    fn prop_d01_calc_graze_biomass_reduction(pre in arbitrary_f64(), util in arbitrary_f64()) {
        let res = calc_graze_biomass_reduction(pre, util);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_d02_calc_graze_height_reduction(pre in arbitrary_f64(), util in arbitrary_f64()) {
        let res = calc_graze_height_reduction(pre, util);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_d03_calc_mow_biomass_residual(pre_b in arbitrary_f64(), pre_h in arbitrary_f64(), mow_h in arbitrary_f64()) {
        let res = calc_mow_biomass_residual(pre_b, pre_h, mow_h);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if pre_h <= 0.0 || mow_h <= 0.0 || pre_b <= 0.0 || !pre_b.is_finite() || !pre_h.is_finite() || !mow_h.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_d04_calc_fire_biomass_consumption(b in arbitrary_f64(), k in arbitrary_f64(), i in arbitrary_f64()) {
        let res = calc_fire_biomass_consumption(b, k, i);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if b <= 0.0 || k <= 0.0 || i <= 0.0 || !b.is_finite() || !k.is_finite() || !i.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }

    #[test]
    fn prop_d05_calc_fire_shrub_mortality(scorch in arbitrary_f64(), alpha in arbitrary_f64(), beta in arbitrary_f64()) {
        let res = calc_fire_shrub_mortality(scorch, alpha, beta);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0 && res <= 1.0);
    }

    #[test]
    fn prop_d06_calc_fire_herb_post_regrowth(resprout in arbitrary_f64(), mort in arbitrary_f64()) {
        let res = calc_fire_herb_post_regrowth(resprout, mort);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
    }

    #[test]
    fn prop_d07_calc_graze_removal_amount(intake in arbitrary_f64(), anim in arbitrary_f64(), days in arbitrary_f64(), ac in arbitrary_f64()) {
        let res = calc_graze_removal_amount(intake, anim, days, ac);
        prop_assert!(!res.is_nan());
        prop_assert!(res.is_finite());
        prop_assert!(res >= 0.0);
        if ac <= 0.0 || intake <= 0.0 || anim <= 0.0 || days <= 0.0 || !ac.is_finite() || !intake.is_finite() || !anim.is_finite() || !days.is_finite() {
            prop_assert_eq!(res, 0.0);
        }
    }
}
