//! Pure mathematical routines for shrub and herbaceous biomass calculation (B01–B22).
//!
//! All functions are pure, non-allocating, `#[inline]`, and strictly guarded
//! against non-positive input domains, non-finite floating-point inputs, and numerical instabilities.
//!
//! Ecological formulas are documented in LaTeX style per optimization standards.

use std::f64::consts::PI;

/// Expansion factor: square meters per acre ($4046.8564224 \text{ m}^2/\text{acre}$).
pub const EXPANSION_FACTOR: f64 = 4046.8564224;

/// Maximum safe exponent for `f64::exp` to prevent IEEE-754 overflow ($+\infty$).
pub(crate) const MAX_EXPONENT: f64 = 709.78;

/// Minimum exponent below which `f64::exp` underflows to 0.0.
pub(crate) const MIN_EXPONENT: f64 = -708.39;

/// Maximum safe exponent for base-10 exponentiation to prevent IEEE-754 overflow ($+\infty$).
pub(crate) const MAX_EXP_10: f64 = 308.2547155599167;

/// Minimum exponent below which base-10 exponentiation underflows to 0.0.
pub(crate) const MIN_EXP_10: f64 = -308.2547155599167;

/// B01: Linear cover-based biomass model (Equation 165).
///
/// $$\mathcal{B}_{165}(c_1, c_2, \text{cover}) = c_1 + c_2 \cdot \max(0.0, \text{cover})$$
#[inline]
pub fn eq_165(cf1: f64, cf2: f64, cover: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cover.is_finite() {
        return 0.0;
    }
    let res = cf1 + cf2 * cover.max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// B02: Exponential cover-based biomass model (Equation 201).
///
/// $$\mathcal{B}_{201}(c_1, c_2, \text{cover}) = \exp(c_1 + c_2 \cdot \max(0.0, \text{cover}))$$
#[inline]
pub fn eq_201(cf1: f64, cf2: f64, cover: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cover.is_finite() {
        return 0.0;
    }
    let arg = cf1 + cf2 * cover.max(0.0);
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

/// B03: Power-law cover-based biomass model (Equation 202).
///
/// $$\mathcal{B}_{202}(c_1, c_2, \text{cover}) = \exp(c_1 + c_2 \ln(\text{cover}))$$
///
/// # Domain Guard
/// Returns `0.0` if $\text{cover} \le 0.0$ to guard against $\ln(0)$ and $\ln(-)$.
#[inline]
pub fn eq_202(cf1: f64, cf2: f64, cover: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cover.is_finite() || cover <= 0.0 {
        return 0.0;
    }
    let arg = cf1 + cf2 * cover.ln();
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

/// B04: Ellipsoidal 3D dimensional volume log-linear model (Equation 636).
///
/// $$\mathcal{B}_{636}(c_1, c_2, L, W, H) = \exp\left(c_1 + c_2 \ln\left(\frac{L \cdot W \cdot H}{1000}\right)\right)$$
///
/// Dimensions are crown length, width, and height in cm. Volume is scaled to $\text{dm}^3$.
#[inline]
pub fn eq_636(cf1: f64, cf2: f64, length: f64, width: f64, height: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite()
        || !length.is_finite() || !width.is_finite() || !height.is_finite()
        || length <= 0.0 || width <= 0.0 || height <= 0.0
    {
        return 0.0;
    }
    let vol = length * width * height;
    if !vol.is_finite() || vol <= 0.0 {
        return 0.0;
    }
    eq_636_2(cf1, cf2, vol)
}

/// B05: Direct volume log-linear model (Equation 636-2).
///
/// $$\mathcal{B}_{636\_2}(c_1, c_2, V) = \exp\left(c_1 + c_2 \ln\left(\frac{V}{1000}\right)\right)$$
///
/// Volume $V$ is in $\text{cm}^3$, scaled to $\text{dm}^3$ ($\div 1000$).
#[inline]
pub fn eq_636_2(cf1: f64, cf2: f64, vol: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !vol.is_finite() || vol <= 0.0 {
        return 0.0;
    }
    let arg = cf1 + cf2 * (vol / 1000.0).ln();
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

/// B06: Canopy cover-height interaction model (Equation 743).
///
/// $$\mathcal{B}_{743}(c_1, c_2, \text{cover}, \text{height}) = c_1 + c_2 \cdot \max(0, \text{cover}) \cdot \max(0, \text{height})$$
#[inline]
pub fn eq_743(cf1: f64, cf2: f64, cover: f64, height: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cover.is_finite() || !height.is_finite() {
        return 0.0;
    }
    let res = cf1 + cf2 * cover.max(0.0) * height.max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// B07: Shrub crown width $\log_{10}$ power law (Equation 998).
///
/// $$\mathcal{B}_{998}(c_1, c_2, W) = 10^{c_1 + c_2 \log_{10}(W)}$$
#[inline]
pub fn eq_998(cf1: f64, cf2: f64, width: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !width.is_finite() || width <= 0.0 {
        return 0.0;
    }
    let exp_val = cf1 + cf2 * width.log10();
    if !(MIN_EXP_10..=MAX_EXP_10).contains(&exp_val) {
        0.0
    } else {
        let res = 10.0f64.powf(exp_val);
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// B08: Bivariate length-width $\log_{10}$ model (Equation 999).
///
/// $$\mathcal{B}_{999}(c_1, c_2, c_3, L, W) = 10^{c_1 + c_2 \log_{10}(L) + c_3 \log_{10}(W)}$$
#[inline]
pub fn eq_999(cf1: f64, cf2: f64, cf3: f64, length: f64, width: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite()
        || !length.is_finite() || !width.is_finite()
        || length <= 0.0 || width <= 0.0
    {
        return 0.0;
    }
    let exp_val = cf1 + cf2 * length.log10() + cf3 * width.log10();
    if !(MIN_EXP_10..=MAX_EXP_10).contains(&exp_val) {
        0.0
    } else {
        let res = 10.0f64.powf(exp_val);
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// B09: Trivariate dimensions $\log_{10}$ model (Equation 1000).
///
/// $$\mathcal{B}_{1000}(c_1, c_2, c_3, c_4, L, W, H) = 10^{c_1 + c_2 \log_{10}(L) + c_3 \log_{10}(W) + c_4 \log_{10}(H)}$$
#[inline]
pub fn eq_1000(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite() || !cf4.is_finite()
        || !l.is_finite() || !w.is_finite() || !h.is_finite()
        || l <= 0.0 || w <= 0.0 || h <= 0.0
    {
        return 0.0;
    }
    let exp_val = cf1 + cf2 * l.log10() + cf3 * w.log10() + cf4 * h.log10();
    if !(MIN_EXP_10..=MAX_EXP_10).contains(&exp_val) {
        0.0
    } else {
        let res = 10.0f64.powf(exp_val);
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// B10: Linear scaled volume model (Equation 1002).
///
/// $$\mathcal{B}_{1002}(c_1, c_2, V) = c_1 + c_2 \left(\frac{\max(0, V)}{1000}\right)$$
#[inline]
pub fn eq_1002(cf1: f64, cf2: f64, vol: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !vol.is_finite() {
        return 0.0;
    }
    let res = cf1 + cf2 * (vol.max(0.0) / 1000.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// B11: Trivariate natural log exponential model (Equation 1008).
///
/// $$\mathcal{B}_{1008}(c_1, c_2, c_3, c_4, L, W, H) = \exp(c_1 + c_2 \ln(L) + c_3 \ln(W) + c_4 \ln(H))$$
#[inline]
pub fn eq_1008(cf1: f64, cf2: f64, cf3: f64, cf4: f64, l: f64, w: f64, h: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite() || !cf4.is_finite()
        || !l.is_finite() || !w.is_finite() || !h.is_finite()
        || l <= 0.0 || w <= 0.0 || h <= 0.0
    {
        return 0.0;
    }
    let arg = cf1 + cf2 * l.ln() + cf3 * w.ln() + cf4 * h.ln();
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

/// B12: Bivariate natural log exponential model (Equation 1012).
///
/// $$\mathcal{B}_{1012}(c_1, c_2, c_3, p_1, p_2) = \exp(c_1 + c_2 \ln(p_1) + c_3 \ln(p_2))$$
#[inline]
pub fn eq_1012(cf1: f64, cf2: f64, cf3: f64, p1: f64, p2: f64) -> f64 {
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

/// B13: Polynomial and square-root model (Equation 1058).
///
/// $$\mathcal{B}_{1058}(c_1, c_2, c_3, p_1) = c_1 + c_2 \cdot \max(0, p_1) + c_3 \sqrt{\max(0, p_1)}$$
#[inline]
pub fn eq_1058(cf1: f64, cf2: f64, cf3: f64, p1: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite() || !p1.is_finite() {
        return 0.0;
    }
    let p_clamped = p1.max(0.0);
    let res = cf1 + cf2 * p_clamped + cf3 * p_clamped.sqrt();
    if !res.is_finite() { 0.0 } else { res }
}

/// B14: Summed dimensions natural log model (Basic BAT).
///
/// $$\mathcal{B}_{\text{basicBAT}}(c_1, c_2, L, W) = \exp(c_1 + c_2 \ln(L + W))$$
#[inline]
pub fn eq_basic_bat(cf1: f64, cf2: f64, length: f64, width: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite()
        || !length.is_finite() || !width.is_finite()
        || length <= 0.0 || width <= 0.0
    {
        return 0.0;
    }
    let sum = length + width;
    if !sum.is_finite() || sum <= 0.0 {
        return 0.0;
    }
    let arg = cf1 + cf2 * sum.ln();
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

/// B15: Area and height $\log_{10}$ power law (Equation 1153).
///
/// $$\mathcal{B}_{1153}(c_1, c_2, c_3, L, W, H) = 10^{c_1 + c_2 \log_{10}(L \cdot W) + c_3 \log_{10}(H)}$$
#[inline]
pub fn eq_1153(cf1: f64, cf2: f64, cf3: f64, l: f64, w: f64, h: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !cf3.is_finite()
        || !l.is_finite() || !w.is_finite() || !h.is_finite()
        || l <= 0.0 || w <= 0.0 || h <= 0.0
    {
        return 0.0;
    }
    let area = l * w;
    if !area.is_finite() || area <= 0.0 {
        return 0.0;
    }
    let exp_val = cf1 + cf2 * area.log10() + cf3 * h.log10();
    if !(MIN_EXP_10..=MAX_EXP_10).contains(&exp_val) {
        0.0
    } else {
        let res = 10.0f64.powf(exp_val);
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// B16: Linear parameter biomass (Equation 1160).
///
/// $$\mathcal{B}_{1160}(c_1, c_2, p_1) = c_1 + c_2 \cdot \max(0, p_1)$$
#[inline]
pub fn eq_1160(cf1: f64, cf2: f64, p1: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !p1.is_finite() {
        return 0.0;
    }
    let res = cf1 + cf2 * p1.max(0.0);
    if !res.is_finite() { 0.0 } else { res }
}

/// B17: Scaled ratio parameter biomass (Equation 1161).
///
/// $$\mathcal{B}_{1161}(c_1, c_2, p_1) = c_1 \left(\frac{\max(0, p_1)}{c_2}\right)$$
#[inline]
pub fn eq_1161(cf1: f64, cf2: f64, p1: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !p1.is_finite() || cf2 == 0.0 {
        return 0.0;
    }
    let ratio = p1.max(0.0) / cf2;
    if !ratio.is_finite() {
        return 0.0;
    }
    let res = cf1 * ratio;
    if !res.is_finite() { 0.0 } else { res }
}

/// B18: Predicted crown area from height power law (Equation PCH).
///
/// $$A_{\text{stem}} = 10^{c_1 + c_2 \log_{10}(H)}$$
#[inline]
pub fn eq_pch(cf1: f64, cf2: f64, height: f64) -> f64 {
    if !cf1.is_finite() || !cf2.is_finite() || !height.is_finite() || height <= 0.0 {
        return 0.0;
    }
    let exp_val = cf1 + cf2 * height.log10();
    if !(MIN_EXP_10..=MAX_EXP_10).contains(&exp_val) {
        0.0
    } else {
        let res = 10.0f64.powf(exp_val);
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// B19: Shrub crown diameter/width from crown area.
///
/// $$w = 2 \sqrt{\frac{A}{\pi}}$$
#[inline]
pub fn calc_crown_width(crown_area: f64) -> f64 {
    if !crown_area.is_finite() || crown_area <= 0.0 {
        0.0
    } else {
        let res = (crown_area / PI).sqrt() * 2.0;
        if !res.is_finite() { 0.0 } else { res }
    }
}

/// B20: Expanded stems per acre with low-height dampening.
///
/// $$E = \frac{4046.8564224}{A_{\text{stem}} \cdot 10^{-4}}, \quad S = E \cdot \left(\frac{\text{cover}}{100}\right)$$
/// $$R = 0.0053 \cdot \exp(0.0809 \cdot \text{height})$$
/// $$S_{\text{final}} = \begin{cases} S \cdot R & \text{if } R \le 0.453595949 \\ S & \text{otherwise} \end{cases}$$
#[inline]
pub fn calc_stems_per_acre_pure(single_stem_area: f64, cover: f64, height: f64) -> f64 {
    if !single_stem_area.is_finite() || !cover.is_finite() || !height.is_finite()
        || single_stem_area <= 0.0 || cover <= 0.0
    {
        return 0.0;
    }
    let denom = single_stem_area * 0.0001;
    if denom <= 0.0 || !denom.is_finite() {
        return 0.0;
    }
    let expanded = EXPANSION_FACTOR / denom;
    if !expanded.is_finite() {
        return 0.0;
    }
    let cover_pct = cover.clamp(0.0, 100.0) / 100.0;
    let mut stems = expanded * cover_pct;
    let exp_arg = 0.0809 * height.max(0.0);
    let reduction = if exp_arg > MAX_EXPONENT {
        f64::MAX
    } else if exp_arg < MIN_EXPONENT {
        0.0
    } else {
        exp_arg.exp() * 0.0053
    };
    if reduction <= 0.453595949 {
        stems *= reduction;
    }
    if !stems.is_finite() { 0.0 } else { stems }
}

/// B21: Herbaceous holdover production from prior 3 years.
///
/// $$H_{\text{holdover}} = 0.3 \cdot \max(0, P_{t-1}) + 0.1 \cdot \max(0, P_{t-2}) + 0.05 \cdot \max(0, P_{t-3})$$
#[inline]
pub fn calc_herb_holdover(p0: f64, p1: f64, p2: f64) -> f64 {
    let p0_c = if !p0.is_finite() { 0.0 } else { p0.max(0.0) };
    let p1_c = if !p1.is_finite() { 0.0 } else { p1.max(0.0) };
    let p2_c = if !p2.is_finite() { 0.0 } else { p2.max(0.0) };
    let res = p0_c * 0.3 + p1_c * 0.1 + p2_c * 0.05;
    if !res.is_finite() { 0.0 } else { res }
}

/// B21-Slice variant: Herbaceous holdover from 3-element array.
#[inline]
pub fn calc_herb_holdover_slice(productions: &[f64; 3]) -> f64 {
    calc_herb_holdover(productions[0], productions[1], productions[2])
}

/// B22: Canopy competition attenuation.
///
/// Attenuates herbaceous biomass through canopy volume competition index:
/// $$I_{\text{herb}} = \max(0, C_h) \cdot \max(0, H_h), \quad I_{\text{shrub}} = \max(0, C_s) \cdot \max(0, H_s)$$
/// $$r = \min\left(\frac{I_{\text{herb}}}{I_{\text{shrub}}}, \; \frac{I_{\text{shrub}}}{I_{\text{herb}}}\right), \quad B_{\text{atten}} = \max(0, B_h) \cdot r$$
#[inline]
pub fn calc_attenuation(herb_b: f64, hc: f64, hh: f64, sc: f64, sh: f64) -> f64 {
    if !herb_b.is_finite() || !hc.is_finite() || !hh.is_finite() || !sc.is_finite() || !sh.is_finite() {
        return 0.0;
    }
    let hi = hc.max(0.0) * hh.max(0.0);
    let si = sc.max(0.0) * sh.max(0.0);
    if si <= 0.0 || hi <= 0.0 || !hi.is_finite() || !si.is_finite() {
        let b = herb_b.max(0.0);
        return if !b.is_finite() { 0.0 } else { b };
    }
    let mut ratio = hi / si;
    if !ratio.is_finite() || ratio.is_nan() {
        ratio = 1.0;
    } else if ratio > 1.0 {
        ratio = 1.0 / ratio;
    }
    let res = herb_b.max(0.0) * ratio;
    if !res.is_finite() { 0.0 } else { res }
}
