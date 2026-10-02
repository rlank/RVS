//! Pure mathematical routines for vegetation succession transitions (S01–S06).
//!
//! Subroutines govern annual linear growth rate, stand plot age calculation,
//! climate-driven herbaceous productivity, covariance quadratic form evaluation,
//! stochastic biomass sampling, and shrub canopy morphological scaling.
//!
//! All functions are pure, non-allocating, `#[inline]`, and strictly guarded
//! against non-finite floating-point inputs (!val.is_finite()) and non-positive domains.
//!
//! Ecological formulas are documented in LaTeX style per optimization standards.

pub const MAX_EXPONENT: f64 = 709.78;
pub const MIN_EXPONENT: f64 = -708.39;

/// S01a: Annual linear growth increment for continuous state variables.
///
/// Ensures the updated value does not drop below 0.0:
/// $$V_{t+1} = \max(0.0, \; V_t + \Delta_{\text{annual}})$$
///
/// # Domain Guard
/// Returns `0.0` if any input is non-finite (`NaN`, `+Inf`, `-Inf`).
/// Guarantees a finite non-negative result.
#[inline]
pub fn calc_growth_rate(current_val: f64, annual_rate: f64) -> f64 {
    if !current_val.is_finite() || !annual_rate.is_finite() {
        return 0.0;
    }
    let updated = current_val + annual_rate;
    if !updated.is_finite() {
        if updated > 0.0 { f64::MAX } else { 0.0 }
    } else {
        updated.max(0.0)
    }
}

/// S01b: Calculates stand plot age based on observed cover, baseline cover,
/// annual growth rate, and cohort starting age:
/// $$\text{Age} = \max\left(0.0, \; \frac{C - C_{\min}}{r_{\text{cov}}} + \text{Age}_{\text{start}}\right)$$
///
/// # Domain Guard
/// Returns `0.0` if any input is non-finite.
/// Returns `start_age.max(0.0)` if `growth_rate <= 0.0` to eliminate division by zero.
#[inline]
pub fn calc_plot_age(cover: f64, min_cover: f64, growth_rate: f64, start_age: f64) -> f64 {
    if !cover.is_finite() || !min_cover.is_finite() || !growth_rate.is_finite() || !start_age.is_finite() {
        return 0.0;
    }
    if growth_rate <= 0.0 {
        return start_age.max(0.0);
    }
    let diff = cover - min_cover;
    let age = (diff / growth_rate) + start_age;
    if !age.is_finite() {
        if age > 0.0 { f64::MAX } else { 0.0 }
    } else {
        age.max(0.0)
    }
}

/// Backward compatibility alias for legacy C++ RVS plot age routine.
#[inline]
pub fn plot_age_calculation(cover: f64, min_cover: f64, growth_rate: f64, start_age: f64) -> f64 {
    calc_plot_age(cover, min_cover, growth_rate, start_age)
}

/// S02: Climate-driven log-linear herbaceous production.
///
/// Evaluates potential herbaceous primary productivity $P$ as a function of annual
/// precipitation ($\text{PPT}$, mm) and vegetation greenness index ($\text{NDVI}$):
/// $$\ln(P) = \beta_0 + \beta_1 \ln(\text{PPT}) + \beta_2 \ln(\text{NDVI}) + \beta_3 \ln(\text{PPT})\ln(\text{NDVI})$$
/// $$P = \exp(\ln(P))$$
///
/// # Domain Guard
/// Returns `0.0` if any coefficient or covariate is non-finite, or if $\text{PPT} \le 0.0$ or $\text{NDVI} \le 0.0$.
#[inline]
pub fn evaluate_herb_growth(b0: f64, b1: f64, b2: f64, b3: f64, ppt: f64, ndvi: f64) -> f64 {
    if !b0.is_finite() || !b1.is_finite() || !b2.is_finite() || !b3.is_finite()
        || !ppt.is_finite() || !ndvi.is_finite() || ppt <= 0.0 || ndvi <= 0.0
    {
        return 0.0;
    }
    let lp = ppt.ln();
    let ln = ndvi.ln();
    let sum = b0 + b1 * lp + b2 * ln + b3 * lp * ln;

    if sum.is_nan() || sum < MIN_EXPONENT {
        0.0
    } else if sum > MAX_EXPONENT || sum.is_infinite() {
        f64::MAX
    } else {
        let p = sum.exp();
        if !p.is_finite() { f64::MAX } else { p.max(0.0) }
    }
}

/// S03: Multi-normal variance quadratic form evaluation ($\mathbf{x}^T \mathbf{\Sigma} \mathbf{x}$).
///
/// Computes prediction error variance for herbaceous productivity regressions:
/// $$s_2^b = \sum_{i=0}^{n-1} \sum_{j=0}^{n-1} x_i \, \Sigma_{ij} \, x_j$$
///
/// # Parameters
/// - `x`: Design/predictor vector slice of length `dim`.
/// - `sigma`: Covariance matrix slice of length `dim * dim` in row-major layout.
/// - `dim`: Dimensionality of predictor space.
///
/// # Domain Guard
/// Returns `0.0` safely if `dim == 0`, if slice lengths are insufficient, or if any element is non-finite.
/// Ensures the returned variance is strictly finite and non-negative (`max(0.0)`).
#[inline]
pub fn calc_s2b_quadratic_form(x: &[f64], sigma: &[f64], dim: usize) -> f64 {
    if dim == 0 || x.len() < dim || sigma.len() < dim * dim {
        return 0.0;
    }

    let mut sum = 0.0;
    for i in 0..dim {
        let xi = x[i];
        if !xi.is_finite() {
            return 0.0;
        }
        let row_offset = i * dim;
        let mut inner = 0.0;
        for j in 0..dim {
            let elem = sigma[row_offset + j];
            let xj = x[j];
            if !elem.is_finite() || !xj.is_finite() {
                return 0.0;
            }
            inner += elem * xj;
        }
        if !inner.is_finite() {
            return 0.0;
        }
        sum += xi * inner;
    }

    if !sum.is_finite() {
        0.0
    } else {
        sum.max(0.0)
    }
}

/// S04: Stochastic bounded herbaceous biomass draw.
///
/// Perturbs mean predicted herbaceous production with standard normal deviate $z \sim \mathcal{N}(0, 1)$
/// scaled by standard deviation $\sqrt{s_2^b}$:
/// $$P_{\text{stochastic}} = \max\left(0.0, \; \mu_P + z \cdot \sqrt{\max(0.0, s_2^b)}\right)$$
///
/// # Domain Guard
/// Returns `0.0` if any parameter is non-finite.
/// Negative variance is guarded via `s2b.max(0.0).sqrt()`.
/// Output is guaranteed finite and non-negative.
#[inline]
pub fn calc_herb_biomass_production(mean_prod: f64, s2b: f64, z_score: f64) -> f64 {
    if !mean_prod.is_finite() || !s2b.is_finite() || !z_score.is_finite() {
        return 0.0;
    }
    let sd = s2b.max(0.0).sqrt();
    let val = mean_prod + z_score * sd;
    if !val.is_finite() {
        if val > 0.0 { f64::MAX } else { 0.0 }
    } else {
        val.max(0.0)
    }
}

/// S05: Shrub crown height growth transition.
///
/// Updates shrub height by `delta_ht`, ensuring height remains non-negative:
/// $$H_{t+1} = \max(0.0, \; H_t + \Delta H)$$
///
/// # Domain Guard
/// Returns `0.0` if any parameter is non-finite.
#[inline]
pub fn scale_shrub_height(current_ht: f64, delta_ht: f64) -> f64 {
    if !current_ht.is_finite() || !delta_ht.is_finite() {
        return 0.0;
    }
    let updated = current_ht + delta_ht;
    if !updated.is_finite() {
        if updated > 0.0 { f64::MAX } else { 0.0 }
    } else {
        updated.max(0.0)
    }
}

/// S06: Shrub canopy cover expansion limit.
///
/// Updates foliar canopy cover $C$ by `delta_cov`, bounded strictly within physical ground limits:
/// $$C_{t+1} = \operatorname{clamp}(C_t + \Delta C, \; 0.0, \; 100.0)$$
///
/// # Domain Guard
/// Returns `0.0` if any parameter is non-finite (`NaN`, `+Inf`, `-Inf`).
/// Sanitizes sum before clamping to guarantee non-NaN output in `[0.0, 100.0]`.
#[inline]
pub fn scale_shrub_cover(current_cov: f64, delta_cov: f64) -> f64 {
    if !current_cov.is_finite() || !delta_cov.is_finite() {
        return 0.0;
    }
    let updated = current_cov + delta_cov;
    if updated.is_nan() {
        0.0
    } else {
        updated.clamp(0.0, 100.0)
    }
}
