//! Pure mathematical routines for fuel loading and partitioning (F01–F12).
//!
//! All routines are pure, non-allocating, `#[inline]`, and strictly guarded
//! against non-positive input domains, non-finite floating-point inputs, and division by zero.
//!
//! Ecological formulas are documented in LaTeX style per optimization standards.

pub(crate) const MAX_EXPONENT: f64 = 709.78;
pub(crate) const MIN_EXPONENT: f64 = -708.39;
pub(crate) const MAX_EXP_10: f64 = 308.2547155599167;
pub(crate) const MIN_EXP_10: f64 = -308.2547155599167;

/// F01: Linear shrub fine fuel loading (Equation 1).
///
/// $$\mathcal{F}_1(cf_1, cf_2, p_1) = cf_1 + cf_2 \cdot \max(0.0, p_1)$$
#[inline]
pub fn fuels_eq_1(cf1: f64, cf2: f64, p1: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !p1.is_finite() {
        return 0.0;
    }
    let res = cf1 + cf2 * p1.max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// F02: Power-law fuel loading (Equation 2).
///
/// $$\mathcal{F}_2(cf_1, cf_2, p_1) = \exp(cf_1 + cf_2 \ln(p_1))$$
///
/// # Domain Guard
/// Returns `0.0` if $p_1 \le 0.0$ to guard against $\ln(0)$ and $\ln(-)$.
#[inline]
pub fn fuels_eq_2(cf1: f64, cf2: f64, p1: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !p1.is_finite() || p1 <= 0.0 {
        return 0.0;
    }
    let arg = cf1 + cf2 * p1.ln();
    if arg.is_nan() {
        0.0
    } else if arg > MAX_EXPONENT {
        f64::MAX
    } else if arg < MIN_EXPONENT {
        0.0
    } else {
        let res = arg.exp();
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// F03: Exponential fuel accumulation (Equation 13).
///
/// $$\mathcal{F}_{13}(cf_1, cf_2, p_1) = \exp(cf_1 + cf_2 \cdot \max(0.0, p_1))$$
#[inline]
pub fn fuels_eq_13(cf1: f64, cf2: f64, p1: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !p1.is_finite() {
        return 0.0;
    }
    let arg = cf1 + cf2 * p1.max(0.0);
    if arg.is_nan() {
        0.0
    } else if arg > MAX_EXPONENT {
        f64::MAX
    } else if arg < MIN_EXPONENT {
        0.0
    } else {
        let res = arg.exp();
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// F04: Bilinear interaction fuel loading (Equation 15).
///
/// $$\mathcal{F}_{15}(cf_1, cf_2, p_1, p_2) = cf_1 + cf_2 \cdot \max(0.0, p_1) \cdot \max(0.0, p_2)$$
#[inline]
pub fn fuels_eq_15(cf1: f64, cf2: f64, p1: f64, p2: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !p1.is_finite() || !p2.is_finite() {
        return 0.0;
    }
    let res = cf1 + cf2 * p1.max(0.0) * p2.max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// F05: Bivariate log-linear fuel model (Equation 17).
///
/// $$\mathcal{F}_{17}(cf_1, cf_2, cf_3, p_1, p_2) = \exp(cf_1 + cf_2 \ln(p_1) + cf_3 \ln(p_2))$$
///
/// # Domain Guard
/// Returns `0.0` if $p_1 \le 0.0$ or $p_2 \le 0.0$.
#[inline]
pub fn fuels_eq_17(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite()
        || !p1.is_finite() || !p2.is_finite()
        || p1 <= 0.0 || p2 <= 0.0
    {
        return 0.0;
    }
    let arg = cf1 + cf2 * p1.ln() + cf3 * p2.ln();
    if arg.is_nan() {
        0.0
    } else if arg > MAX_EXPONENT {
        f64::MAX
    } else if arg < MIN_EXPONENT {
        0.0
    } else {
        let res = arg.exp();
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// F06: Trivariate log10 fuel model (Equation 25).
///
/// $$\mathcal{F}_{25}(cf_1, cf_2, cf_3, cf_4, p_1, p_2, p_3) = 10^{cf_1 + cf_2 \log_{10}(p_1) + cf_3 \log_{10}(p_2) + cf_4 \log_{10}(p_3)}$$
#[inline]
pub fn fuels_eq_25(cf1: f64, cf2: f64, cf3: f64, cf4: f64, p1: f64, p2: f64, p3: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite() || !cf4.is_finite()
        || !p1.is_finite() || !p2.is_finite() || !p3.is_finite()
        || p1 <= 0.0 || p2 <= 0.0 || p3 <= 0.0
    {
        return 0.0;
    }
    let exp_val = cf1 + cf2 * p1.log10() + cf3 * p2.log10() + cf4 * p3.log10();
    if !(MIN_EXP_10..=MAX_EXP_10).contains(&exp_val) {
        0.0
    } else {
        let res = 10.0f64.powf(exp_val);
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// F06b: Trivariate natural log exponential fuel model (Equation 42).
///
/// $$\mathcal{F}_{42}(cf_1, cf_2, cf_3, cf_4, p_1, p_2, p_3) = \exp(cf_1 + cf_2 \ln(p_1) + cf_3 \ln(p_2) + cf_4 \ln(p_3))$$
#[inline]
pub fn fuels_eq_42(cf1: f64, cf2: f64, cf3: f64, cf4: f64, p1: f64, p2: f64, p3: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite() || !cf4.is_finite()
        || !p1.is_finite() || !p2.is_finite() || !p3.is_finite()
        || p1 <= 0.0 || p2 <= 0.0 || p3 <= 0.0
    {
        return 0.0;
    }
    let arg = cf1 + cf2 * p1.ln() + cf3 * p2.ln() + cf4 * p3.ln();
    if arg.is_nan() {
        0.0
    } else if arg > MAX_EXPONENT {
        f64::MAX
    } else if arg < MIN_EXPONENT {
        0.0
    } else {
        let res = arg.exp();
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// F07: Quadratic polynomial fuel model (Equation 33).
///
/// $$\mathcal{F}_{33}(cf_1, cf_2, cf_3, p_1) = cf_1 + cf_2 \cdot \max(0, p_1) + cf_3 \cdot \max(0, p_1)^2$$
#[inline]
pub fn fuels_eq_33(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite() || !p1.is_finite() {
        return 0.0;
    }
    let p = p1.max(0.0);
    let inner = cf2 + cf3 * p;
    let res = if !inner.is_finite() {
        0.0
    } else {
        cf1 + p * inner
    };
    if !res.is_finite() { 0.0 } else { res }
}

/// F07b: Bivariate linear fuel loading model (Equation 46).
///
/// $$\mathcal{F}_{46}(cf_1, cf_2, cf_3, p_1, p_2) = cf_1 + cf_2 \cdot \max(0, p_1) + cf_3 \cdot \max(0, p_2)$$
#[inline]
pub fn fuels_eq_46(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite() || !p1.is_finite() || !p2.is_finite() {
        return 0.0;
    }
    let res = cf1 + cf2 * p1.max(0.0) + cf3 * p2.max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// F08: Shrub 1-hr wood/bark component calculation.
///
/// $$F_{\text{wb}} = \max(0.0, B_{\text{shrub}}) \cdot \operatorname{clamp}(p, 0.0, 1.0) \cdot 0.5$$
#[inline]
pub fn calc_fuel_component_wb(shrub_biomass: f64, proportion: f64) -> f64 {
    if !shrub_biomass.is_finite() || !proportion.is_finite() {
        return 0.0;
    }
    let res = shrub_biomass.max(0.0) * proportion.clamp(0.0, 1.0) * 0.5;
    if !res.is_finite() { 0.0 } else { res }
}

/// F09: Shrub 1-hr foliage component calculation.
///
/// $$F_{\text{fl}} = \max(0.0, B_{\text{shrub}}) \cdot \operatorname{clamp}(p, 0.0, 1.0) \cdot 0.5$$
#[inline]
pub fn calc_fuel_component_fl(shrub_biomass: f64, proportion: f64) -> f64 {
    if !shrub_biomass.is_finite() || !proportion.is_finite() {
        return 0.0;
    }
    let res = shrub_biomass.max(0.0) * proportion.clamp(0.0, 1.0) * 0.5;
    if !res.is_finite() { 0.0 } else { res }
}

/// F10: Total 1-hr fine fuel loading.
///
/// $$F_{1\text{hr}} = \max(0, F_{\text{wb}}) + \max(0, F_{\text{fl}}) + \max(0, F_{\text{herb}})$$
#[inline]
pub fn calc_total_1hr_fuel(wb: f64, fl: f64, herb: f64) -> f64 {
    if !wb.is_finite() || !fl.is_finite() || !herb.is_finite() {
        return 0.0;
    }
    let res = wb.max(0.0) + fl.max(0.0) + herb.max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// F11: Fuel size class partition percentages.
///
/// Normalizes 1-hr, 10-hr, 100-hr, and 1000-hr fuel timelag classes into relative percentages:
/// $$T = \sum_{i \in \{1, 10, 100, 1000\}} \max(0.0, F_i)$$
/// $$P_i = \begin{cases} \frac{\max(0.0, F_i)}{T} \cdot 100.0 & \text{if } T > 0.0 \\ 0.0 & \text{otherwise} \end{cases}$$
///
/// Returns `(p1, p10, p100, p1000, total)`.
#[inline]
pub fn calc_fuel_percentages(f1: f64, f10: f64, f100: f64, f1000: f64) -> (f64, f64, f64, f64, f64) {
    if !f1.is_finite() || !f10.is_finite() || !f100.is_finite() || !f1000.is_finite() {
        return (0.0, 0.0, 0.0, 0.0, 0.0);
    }
    let f1_c = f1.max(0.0);
    let f10_c = f10.max(0.0);
    let f100_c = f100.max(0.0);
    let f1000_c = f1000.max(0.0);

    let total = f1_c + f10_c + f100_c + f1000_c;
    if total <= 0.0 || !total.is_finite() {
        (0.0, 0.0, 0.0, 0.0, 0.0)
    } else {
        let inv = 100.0 / total;
        if !inv.is_finite() {
            (0.0, 0.0, 0.0, 0.0, total)
        } else {
            let p1 = (f1_c * inv).clamp(0.0, 100.0);
            let p10 = (f10_c * inv).clamp(0.0, 100.0);
            let p100 = (f100_c * inv).clamp(0.0, 100.0);
            let p1000 = (f1000_c * inv).clamp(0.0, 100.0);
            (p1, p10, p100, p1000, total)
        }
    }
}

/// F12: Herbaceous curing and fine fuel loading.
///
/// $$F_{\text{herb}} = \max(0.0, B_{\text{herb}}) \cdot \operatorname{clamp}(c_{\text{cure}}, 0.0, 1.0)$$
#[inline]
pub fn calc_herb_loading(herb_biomass: f64, cure_rate: f64) -> f64 {
    if !herb_biomass.is_finite() || !cure_rate.is_finite() {
        return 0.0;
    }
    let res = herb_biomass.max(0.0) * cure_rate.clamp(0.0, 1.0);
    if !res.is_finite() { 0.0 } else { res }
}

// ---------------------------------------------------------------------------
// C++20 Baseline Parity Routines (matching FuelsEquations.h & FuelsDriver.cpp)
// ---------------------------------------------------------------------------

/// Computes 1-hr wood/bark fuel loading from shrub biomass:
/// $$F_{1\text{wb}} = 2.932237 + 0.353554 \cdot \max(0.0, B)$$
#[inline]
pub fn calc_1hr_wood_bark(biomass: f64) -> f64 {
    if !biomass.is_finite() {
        return 0.0;
    }
    let res = 2.932237 + 0.353554 * biomass.max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// Computes 1-hr foliage fuel loading from shrub biomass:
/// $$F_{1\text{fl}} = \exp(-1.498408 + 0.955207 \ln(B))$$
#[inline]
pub fn calc_1hr_foliage(biomass: f64) -> f64 {
    if !biomass.is_finite() || biomass <= 0.0 {
        return 0.0;
    }
    let arg = -1.498408 + 0.955207 * biomass.ln();
    if arg.is_nan() {
        0.0
    } else if arg > MAX_EXPONENT {
        f64::MAX
    } else if arg < MIN_EXPONENT {
        0.0
    } else {
        let res = arg.exp();
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// Computes total 1-hr shrub fuel loading:
/// $$F_{1\text{hr}} = F_{1\text{wb}} + F_{1\text{fl}}$$
#[inline]
pub fn calc_1hr_fuel(biomass: f64) -> f64 {
    if !biomass.is_finite() {
        return 0.0;
    }
    let res = calc_1hr_wood_bark(biomass) + calc_1hr_foliage(biomass);
    if !res.is_finite() { 0.0 } else { res }
}

/// Computes 10-hr fuel loading from shrub biomass and average stem diameter:
/// Intercept is -5.192477 if avg_stem > 22.6796, else 0.012477.
#[inline]
pub fn calc_10hr_fuel(biomass: f64, shrub_avg_stem: f64) -> f64 {
    if !biomass.is_finite() || !shrub_avg_stem.is_finite() {
        return 0.0;
    }
    let intercept = if shrub_avg_stem > 22.6796 {
        -5.192477
    } else {
        0.012477
    };
    let res = (intercept + 0.445884 * biomass.max(0.0)).max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// Computes 100-hr fuel loading from shrub biomass:
/// $$F_{100\text{hr}} = \exp(-3.552846 + 1.005534 \ln(B))$$
#[inline]
pub fn calc_100hr_fuel(biomass: f64) -> f64 {
    if !biomass.is_finite() || biomass <= 0.0 {
        return 0.0;
    }
    let arg = -3.552846 + 1.005534 * biomass.ln();
    if arg.is_nan() {
        0.0
    } else if arg > MAX_EXPONENT {
        f64::MAX
    } else if arg < MIN_EXPONENT {
        0.0
    } else {
        let res = arg.exp();
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// Computes 1000-hr fuel loading from shrub height:
/// Returns 0.0 if height <= 400.0, else $(3.6853 \log_{10}(100 H) - 1.306) / 100.0$.
#[inline]
pub fn calc_1000hr_fuel(shrub_height: f64) -> f64 {
    if !shrub_height.is_finite() || shrub_height <= 400.0 {
        return 0.0;
    }
    let arg = 100.0 * shrub_height;
    if !arg.is_finite() || arg <= 0.0 {
        return 0.0;
    }
    let val = (3.6853 * arg.log10() - 1.306) / 100.0;
    let res = val.max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// Calculates component fuel proportion: component / total.
#[inline]
pub fn calc_fuel_proportion(component: f64, total: f64) -> f64 {
    if !component.is_finite() || !total.is_finite() || total <= 0.0 {
        0.0
    } else {
        let ratio = component.max(0.0) / total;
        if !ratio.is_finite() {
            0.0
        } else {
            ratio.clamp(0.0, 1.0)
        }
    }
}

/// Calculates component fuel percentage: (component / total) * 100.0.
#[inline]
pub fn calc_fuel_percent(component: f64, total: f64) -> f64 {
    let prop = calc_fuel_proportion(component, total);
    let res = prop * 100.0;
    if !res.is_finite() { 0.0 } else { res }
}
