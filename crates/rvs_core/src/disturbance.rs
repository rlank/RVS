//! Pure mathematical routines for disturbance response (D01–D06).
//!
//! Subroutines quantify forage depletion under grazing, non-linear stubble defoliation,
//! mechanical mowing residual retention, heat-driven combustion consumption, crown scorch
//! shrub mortality, and post-fire herbaceous resprout regrowth.
//!
//! All functions are pure, non-allocating, `#[inline]`, and strictly guarded
//! against non-finite floating-point inputs (!val.is_finite()) and non-positive inputs.
//!
//! Ecological formulas are documented in LaTeX style per optimization standards.

/// D01: Grazing utilization biomass depletion.
///
/// Computes post-grazing residual standing biomass from pre-disturbance biomass and utilization percentage:
/// $$B_{\text{post}} = \max(0.0, B_{\text{pre}}) \cdot \left(1.0 - \frac{\operatorname{clamp}(U_{\%}, 0.0, 100.0)}{100.0}\right)$$
///
/// # Domain Guard
/// Returns `0.0` if any input is non-finite or if `pre_biomass <= 0.0`.
/// Guarantees a finite non-negative result `<= pre_biomass`.
#[inline]
pub fn calc_graze_biomass_reduction(pre_biomass: f64, utilization_pct: f64) -> f64 {
    if !pre_biomass.is_finite() || !utilization_pct.is_finite() || pre_biomass <= 0.0 {
        return 0.0;
    }
    let u = utilization_pct.clamp(0.0, 100.0) / 100.0;
    let factor = (1.0 - u).clamp(0.0, 1.0);
    let res = pre_biomass * factor;
    if !res.is_finite() {
        0.0
    } else {
        res.max(0.0)
    }
}

/// D02: Non-linear grazing height defoliation.
///
/// Models empirical non-linear stubble height reduction under grazing defoliation via 0.7 power law:
/// $$H_{\text{post}} = \max(0.0, H_{\text{pre}}) \cdot \left(\max\left(0.0, \; 1.0 - \frac{\operatorname{clamp}(U_{\%}, 0.0, 100.0)}{100.0}\right)\right)^{0.7}$$
///
/// # Domain Guard
/// Returns `0.0` if any input is non-finite or if `pre_height <= 0.0`.
/// Guarantees a finite non-negative result `<= pre_height`.
#[inline]
pub fn calc_graze_height_reduction(pre_height: f64, utilization_pct: f64) -> f64 {
    if !pre_height.is_finite() || !utilization_pct.is_finite() || pre_height <= 0.0 {
        return 0.0;
    }
    let u = utilization_pct.clamp(0.0, 100.0) / 100.0;
    let rem = (1.0 - u).clamp(0.0, 1.0);
    let factor = rem.powf(0.7);
    let res = pre_height * factor;
    if !res.is_finite() {
        0.0
    } else {
        res.max(0.0)
    }
}

/// D03: Mechanical mower residual biomass calculation.
///
/// Evaluates residual biomass following mechanical mowing to cutter-bar height `mow_height`:
/// $$B_{\text{post}} = \max(0.0, B_{\text{pre}}) \cdot \operatorname{clamp}\left(\frac{H_{\text{mow}}}{H_{\text{pre}}}, \; 0.0, \; 1.0\right)$$
///
/// # Domain Guard
/// Returns `0.0` if any input is non-finite, or if `pre_biomass <= 0.0`, `pre_height <= 0.0`, or `mow_height <= 0.0`.
/// Guarantees a finite non-negative result `<= pre_biomass`.
#[inline]
pub fn calc_mow_biomass_residual(pre_biomass: f64, pre_height: f64, mow_height: f64) -> f64 {
    if !pre_biomass.is_finite() || !pre_height.is_finite() || !mow_height.is_finite()
        || pre_biomass <= 0.0 || pre_height <= 0.0 || mow_height <= 0.0
    {
        return 0.0;
    }
    let ratio = (mow_height / pre_height).clamp(0.0, 1.0);
    let res = pre_biomass * ratio;
    if !res.is_finite() {
        0.0
    } else {
        res.max(0.0)
    }
}

/// D04: Heat-driven wildfire biomass consumption.
///
/// Calculates biomass consumed by fire as an exponential saturation function of
/// fireline intensity and fuel severity factor:
/// $$B_{\text{consumed}} = \max(0.0, B_{\text{pre}}) \cdot \left(1.0 - \exp\big(-\max(0, k_{\text{sev}}) \cdot \max(0, I_{\text{fire}})\big)\right)$$
///
/// # Domain Guard
/// Returns `0.0` if any input is non-finite, or if `biomass <= 0.0`, `severity_k <= 0.0`, or `fire_intensity <= 0.0`.
/// Guarantees a finite non-negative result `<= biomass`.
#[inline]
pub fn calc_fire_biomass_consumption(biomass: f64, severity_k: f64, fire_intensity: f64) -> f64 {
    if !biomass.is_finite() || !severity_k.is_finite() || !fire_intensity.is_finite()
        || biomass <= 0.0 || severity_k <= 0.0 || fire_intensity <= 0.0
    {
        return 0.0;
    }
    let prod = severity_k * fire_intensity;
    let exp_term = if prod > 708.39 {
        0.0
    } else {
        (-prod).exp()
    };
    let consumed_fraction = if exp_term.is_nan() {
        0.0
    } else {
        (1.0 - exp_term).clamp(0.0, 1.0)
    };
    let res = biomass * consumed_fraction;
    if !res.is_finite() {
        0.0
    } else {
        res.max(0.0)
    }
}

/// D05: Scorch-dependent shrub mortality logistic sigmoid.
///
/// Predicts probability of shrub individual mortality or stand top-kill from crown scorch percentage:
/// $$z = \alpha + \beta \cdot \operatorname{clamp}(S_{\text{pct}}, 0.0, 100.0)$$
/// $$M = \frac{1.0}{1.0 + \exp(-z)}$$
///
/// # Domain Guard
/// Returns `0.0` if any input is non-finite (`NaN`, `+Inf`, `-Inf`).
/// Numerical saturation bounds for $|z| \ge 40.0$ prevent floating-point underflow or overflow.
/// Guarantees a finite probability strictly in `[0.0, 1.0]`.
#[inline]
pub fn calc_fire_shrub_mortality(scorch_pct: f64, alpha: f64, beta: f64) -> f64 {
    if !scorch_pct.is_finite() || !alpha.is_finite() || !beta.is_finite() {
        return 0.0;
    }
    let s = scorch_pct.clamp(0.0, 100.0);
    let z = alpha + beta * s;

    if z.is_nan() {
        0.0
    } else if z >= 40.0 {
        1.0
    } else if z <= -40.0 {
        0.0
    } else {
        let ez = (-z).exp();
        if ez.is_nan() || !ez.is_finite() {
            if z > 0.0 { 1.0 } else { 0.0 }
        } else {
            (1.0 / (1.0 + ez)).clamp(0.0, 1.0)
        }
    }
}

/// D06: Post-fire herbaceous resprout regrowth response.
///
/// Computes surviving post-fire herbaceous biomass from pre-fire resprout potential
/// and meristem mortality percentage:
/// $$B_{\text{post}} = \max(0.0, B_{\text{resprout}}) \cdot \max\left(0.0, \; 1.0 - \frac{\operatorname{clamp}(M_{\text{mort}}, 0.0, 100.0)}{100.0}\right)$$
///
/// # Domain Guard
/// Returns `0.0` if any input is non-finite or if `resprout_pot <= 0.0`.
/// Guarantees a finite non-negative result `<= resprout_pot`.
#[inline]
pub fn calc_fire_herb_post_regrowth(resprout_pot: f64, mortality_pct: f64) -> f64 {
    if !resprout_pot.is_finite() || !mortality_pct.is_finite() || resprout_pot <= 0.0 {
        return 0.0;
    }
    let m = mortality_pct.clamp(0.0, 100.0) / 100.0;
    let survival = (1.0 - m).clamp(0.0, 1.0);
    let res = resprout_pot * survival;
    if !res.is_finite() {
        0.0
    } else {
        res.max(0.0)
    }
}

/// Auxiliary Disturbance Routine: Graze removal biomass amount in grams.
///
/// Converts stocking rate ($N$ animals), daily intake ($M_{\text{intake}}$ lbs/day),
/// grazing duration ($T$ days), and stand area ($A$ acres) into total biomass removed (grams):
/// $$R_{\text{lbs/ac}} = \frac{M_{\text{intake}} \cdot N_{\text{grazers}} \cdot T_{\text{days}}}{A_{\text{acres}}}$$
/// $$\Delta B_{\text{grams}} = R_{\text{lbs/ac}} \cdot 453.59237$$
///
/// # Domain Guard
/// Returns `0.0` if any parameter is non-finite or non-positive.
#[inline]
pub fn calc_graze_removal_amount(
    intake_lbs_per_day: f64,
    num_animals: f64,
    days: f64,
    area_acres: f64,
) -> f64 {
    if !intake_lbs_per_day.is_finite() || !num_animals.is_finite() || !days.is_finite() || !area_acres.is_finite()
        || area_acres <= 0.0 || intake_lbs_per_day <= 0.0 || num_animals <= 0.0 || days <= 0.0
    {
        return 0.0;
    }
    const LBS_TO_GRAMS: f64 = 453.59237;
    let lbs_per_acre = (intake_lbs_per_day * num_animals * days) / area_acres;
    let grams = lbs_per_acre * LBS_TO_GRAMS;
    if !grams.is_finite() {
        if grams > 0.0 { f64::MAX } else { 0.0 }
    } else {
        grams.max(0.0)
    }
}
