"""
RVS Mathematical Kernel Reference Oracle & Canonical Data Structures.
Implements the exact LaTeX mathematical specifications, domain guards,
SoA buffer specifications, and simulation models from rust_candidates.md,
survey_math_catalog.md, and PROJECT.md.
"""

import math

PI = math.pi
EXPANSION_FACTOR = 4046.8564224


def clamp(val: float, low: float, high: float) -> float:
    return max(low, min(val, high))


# ==============================================================================
# Biomass Module (B01 - B22)
# ==============================================================================


def eq_165(cf1: float, cf2: float, cover: float) -> float:
    """B01: Linear cover-based biomass."""
    return cf1 + cf2 * max(0.0, cover)


def eq_201(cf1: float, cf2: float, cover: float) -> float:
    """B02: Exponential cover-based biomass."""
    try:
        return math.exp(cf1 + cf2 * max(0.0, cover))
    except OverflowError:
        return float("inf")


def eq_202(cf1: float, cf2: float, cover: float) -> float:
    """B03: Power-law cover-based biomass."""
    if cover <= 0.0:
        return 0.0
    try:
        return math.exp(cf1 + cf2 * math.log(cover))
    except OverflowError:
        return float("inf")


def eq_636(cf1: float, cf2: float, length: float, width: float, height: float) -> float:
    """B04: 3D dimensional volume log-linear."""
    vol = length * width * height
    return eq_636_2(cf1, cf2, vol)


def eq_636_2(cf1: float, cf2: float, vol: float) -> float:
    """B05: Direct volume log-linear."""
    if vol <= 0.0:
        return 0.0
    scaled_vol = vol / 1000.0
    if scaled_vol <= 0.0:
        return 0.0
    try:
        return math.exp(cf1 + cf2 * math.log(scaled_vol))
    except OverflowError:
        return float("inf")


def eq_743(cf1: float, cf2: float, cover: float, height: float) -> float:
    """B06: Canopy cover-height interaction."""
    return cf1 + cf2 * max(0.0, cover) * max(0.0, height)


def eq_998(cf1: float, cf2: float, width: float) -> float:
    """B07: Shrub width log10 power law."""
    if width <= 0.0:
        return 0.0
    try:
        return 10.0 ** (cf1 + cf2 * math.log10(width))
    except OverflowError:
        return float("inf")


def eq_999(cf1: float, cf2: float, cf3: float, length: float, width: float) -> float:
    """B08: Bivariate length-width log10."""
    if length <= 0.0 or width <= 0.0:
        return 0.0
    try:
        return 10.0 ** (cf1 + cf2 * math.log10(length) + cf3 * math.log10(width))
    except OverflowError:
        return float("inf")


def eq_1000(cf1: float, cf2: float, cf3: float, cf4: float, l: float, w: float, h: float) -> float:
    """B09: Trivariate dimensions log10."""
    if l <= 0.0 or w <= 0.0 or h <= 0.0:
        return 0.0
    try:
        return 10.0 ** (cf1 + cf2 * math.log10(l) + cf3 * math.log10(w) + cf4 * math.log10(h))
    except OverflowError:
        return float("inf")


def eq_1002(cf1: float, cf2: float, vol: float) -> float:
    """B10: Linear scaled volume."""
    return cf1 + cf2 * max(0.0, vol)


def eq_1008(cf1: float, cf2: float, cf3: float, cf4: float, l: float, w: float, h: float) -> float:
    """B11: Trivariate natural log exponential."""
    if l <= 0.0 or w <= 0.0 or h <= 0.0:
        return 0.0
    try:
        return math.exp(cf1 + cf2 * math.log(l) + cf3 * math.log(w) + cf4 * math.log(h))
    except OverflowError:
        return float("inf")


def eq_1012(cf1: float, cf2: float, cf3: float, p1: float, p2: float) -> float:
    """B12: Bivariate natural log exponential."""
    if p1 <= 0.0 or p2 <= 0.0:
        return 0.0
    try:
        return math.exp(cf1 + cf2 * math.log(p1) + cf3 * math.log(p2))
    except OverflowError:
        return float("inf")


def eq_1058(cf1: float, cf2: float, cf3: float, p1: float) -> float:
    """B13: Polynomial + square-root model."""
    sqrt_p1 = math.sqrt(p1) if p1 > 0.0 else 0.0
    return cf1 + cf2 * p1 + cf3 * sqrt_p1


def eq_basic_bat(cf1: float, cf2: float, length: float, width: float) -> float:
    """B14: Summed dimensions natural log."""
    s = length + width
    if s <= 0.0:
        return 0.0
    try:
        return math.exp(cf1 + cf2 * math.log(s))
    except OverflowError:
        return float("inf")


def eq_1153(cf1: float, cf2: float, cf3: float, l: float, w: float, h: float) -> float:
    """B15: Area and height log10 power law."""
    area = l * w
    if area <= 0.0 or h <= 0.0:
        return 0.0
    try:
        return 10.0 ** (cf1 + cf2 * math.log10(area) + cf3 * math.log10(h))
    except OverflowError:
        return float("inf")


def eq_1160(cf1: float, cf2: float, p1: float) -> float:
    """B16: Linear parameter biomass."""
    return cf1 + cf2 * max(0.0, p1)


def eq_1161(cf1: float, cf2: float, p1: float) -> float:
    """B17: Scaled ratio parameter biomass."""
    if cf2 == 0.0:
        return 0.0
    return cf1 * (p1 / cf2)


def eq_pch(cf1: float, cf2: float, height: float) -> float:
    """B18: Predicted crown area from height."""
    if height <= 0.0:
        return 0.0
    try:
        return 10.0 ** (cf1 + cf2 * math.log10(height))
    except OverflowError:
        return float("inf")


def calc_crown_width(crown_area: float) -> float:
    """B19: Shrub crown width (diameter in cm) from circular crown area in cm^2."""
    if crown_area <= 0.0:
        return 0.0
    return math.sqrt(crown_area / PI) * 2.0


def calc_stems_per_acre_pure(single_stem_area: float, cover: float, height: float) -> float:
    """B20: Expanded stems per acre with low-height dampening."""
    if single_stem_area <= 0.0 or cover <= 0.0:
        return 0.0
    expanded = EXPANSION_FACTOR / (single_stem_area * 0.0001)
    stems = expanded * (cover / 100.0)
    try:
        reduction = math.exp(0.0809 * height) * 0.0053
    except OverflowError:
        reduction = float("inf")
    if reduction <= 0.453595949:
        stems *= reduction
    return stems


def calc_herb_holdover(p0: float, p1: float, p2: float) -> float:
    """B21: 3-year herbaceous production holdover."""
    return max(0.0, p0) * 0.3 + max(0.0, p1) * 0.1 + max(0.0, p2) * 0.05


def calc_attenuation(herb_b: float, hc: float, hh: float, sc: float, sh: float) -> float:
    """B22: Canopy competition attenuation between herb and shrub."""
    hi = hc * hh
    si = sc * sh
    if si <= 0.0 or hi <= 0.0:
        return herb_b
    ratio = hi / si
    if ratio > 1.0:
        ratio = 1.0 / ratio
    return herb_b * ratio


# ==============================================================================
# Fuels Module (F01 - F12)
# ==============================================================================


def fuels_eq_1(cf1: float, cf2: float, p1: float) -> float:
    """F01: Linear fine fuel loading."""
    return cf1 + cf2 * max(0.0, p1)


def fuels_eq_2(cf1: float, cf2: float, p1: float) -> float:
    """F02: Power-law fuel loading."""
    if p1 <= 0.0:
        return 0.0
    try:
        return math.exp(cf1 + cf2 * math.log(p1))
    except OverflowError:
        return float("inf")


def fuels_eq_13(cf1: float, cf2: float, p1: float) -> float:
    """F03: Exponential fuel accumulation."""
    try:
        return math.exp(cf1 + cf2 * p1)
    except OverflowError:
        return float("inf")


def fuels_eq_15(cf1: float, cf2: float, p1: float, p2: float) -> float:
    """F04: Bilinear interaction fuel loading."""
    return cf1 + cf2 * max(0.0, p1) * max(0.0, p2)


def fuels_eq_17(cf1: float, cf2: float, cf3: float, p1: float, p2: float) -> float:
    """F05: Bivariate log-linear fuel model."""
    if p1 <= 0.0 or p2 <= 0.0:
        return 0.0
    try:
        return math.exp(cf1 + cf2 * math.log(p1) + cf3 * math.log(p2))
    except OverflowError:
        return float("inf")


def fuels_eq_25(
    cf1: float, cf2: float, cf3: float, cf4: float, p1: float, p2: float, p3: float
) -> float:
    """F06: Trivariate dimensions fuel allometry."""
    if p1 <= 0.0 or p2 <= 0.0 or p3 <= 0.0:
        return 0.0
    try:
        return 10.0 ** (cf1 + cf2 * math.log10(p1) + cf3 * math.log10(p2) + cf4 * math.log10(p3))
    except OverflowError:
        return float("inf")


def fuels_eq_33(cf1: float, cf2: float, cf3: float, p1: float) -> float:
    """F07: Non-linear quadratic fuel curve."""
    return cf1 + p1 * (cf2 + cf3 * p1)


def calc_fuel_component_wb(shrub_biomass: float, proportion: float) -> float:
    """F08: Wood/bark 1-hr fuel split."""
    return max(0.0, shrub_biomass) * clamp(proportion, 0.0, 1.0) * 0.5


def calc_fuel_component_fl(shrub_biomass: float, proportion: float) -> float:
    """F09: Foliage 1-hr fuel split."""
    return max(0.0, shrub_biomass) * clamp(proportion, 0.0, 1.0) * 0.5


def calc_total_1hr_fuel(wb: float, fl: float, herb: float) -> float:
    """F10: Aggregated 1-hour fine fuel."""
    return max(0.0, wb) + max(0.0, fl) + max(0.0, herb)


def calc_fuel_percentages(
    f1: float, f10: float, f100: float, f1000: float
) -> tuple[float, float, float, float, float]:
    """F11: Fuel size class percentages."""
    total = f1 + f10 + f100 + f1000
    if total <= 0.0:
        return (0.0, 0.0, 0.0, 0.0, 0.0)
    inv = 100.0 / total
    return (f1 * inv, f10 * inv, f100 * inv, f1000 * inv, total)


def calc_herb_loading(herb_biomass: float, cure_rate: float) -> float:
    """F12: Herbaceous fuel loading."""
    return max(0.0, herb_biomass) * clamp(cure_rate, 0.0, 1.0)


# ==============================================================================
# Succession Module (S01 - S06)
# ==============================================================================


def calc_growth_rate(current_val: float, annual_rate: float) -> float:
    """S01: Annual linear growth increment."""
    return max(0.0, current_val + annual_rate)


def evaluate_herb_growth(
    b0: float, b1: float, b2: float, b3: float, ppt: float, ndvi: float
) -> float:
    """S02: Climate-driven herbaceous production."""
    if ppt <= 0.0 or ndvi <= 0.0:
        return 0.0
    try:
        lp = math.log(ppt)
        ln = math.log(ndvi)
        return math.exp(b0 + b1 * lp + b2 * ln + b3 * lp * ln)
    except OverflowError:
        return float("inf")


def calc_s2b_quadratic_form(x: list[float], sigma: list[float], dim: int) -> float:
    """S03: Variance quadratic form x^T * Sigma * x."""
    assert len(x) == dim, f"Vector x length {len(x)} != dim {dim}"
    assert len(sigma) == dim * dim, f"Matrix sigma length {len(sigma)} != dim^2 {dim * dim}"
    total = 0.0
    for i in range(dim):
        row_offset = i * dim
        inner = sum(sigma[row_offset + j] * x[j] for j in range(dim))
        total += x[i] * inner
    return max(0.0, total)


def calc_herb_biomass_production(mean_prod: float, s2b: float, z_score: float) -> float:
    """S04: Stochastic bounded herbaceous biomass draw."""
    sd = math.sqrt(max(0.0, s2b))
    return max(0.0, mean_prod + z_score * sd)


def scale_shrub_height(current_ht: float, delta_ht: float) -> float:
    """S05: Shrub height growth transition."""
    return max(0.0, current_ht + delta_ht)


def scale_shrub_cover(current_cov: float, delta_cov: float) -> float:
    """S06: Shrub cover scaling bounded [0, 100]."""
    return clamp(current_cov + delta_cov, 0.0, 100.0)


# ==============================================================================
# Disturbance Module (D01 - D06)
# ==============================================================================


def calc_graze_biomass_reduction(pre_biomass: float, utilization_pct: float) -> float:
    """D01: Grazing biomass depletion."""
    u = clamp(utilization_pct, 0.0, 100.0) / 100.0
    return max(0.0, pre_biomass) * (1.0 - u)


def calc_graze_height_reduction(pre_height: float, utilization_pct: float) -> float:
    """D02: Grazing height reduction with non-linear stubble defoliation."""
    u = clamp(utilization_pct, 0.0, 100.0) / 100.0
    rem = max(0.0, 1.0 - u)
    return max(0.0, pre_height) * (rem**0.7)


def calc_mow_biomass_residual(pre_biomass: float, pre_height: float, mow_height: float) -> float:
    """D03: Mechanical mowing residual biomass."""
    if pre_height <= 0.0:
        return 0.0
    ratio = clamp(mow_height / pre_height, 0.0, 1.0)
    return max(0.0, pre_biomass) * ratio


def calc_fire_biomass_consumption(
    biomass: float, severity_k: float, fire_intensity: float
) -> float:
    """D04: Wildfire biomass consumption."""
    effective_intensity = max(0.0, fire_intensity)
    try:
        frac = 1.0 - math.exp(-severity_k * effective_intensity)
    except OverflowError:
        frac = 1.0
    return max(0.0, biomass) * clamp(frac, 0.0, 1.0)


def calc_fire_shrub_mortality(scorch_pct: float, alpha: float, beta: float) -> float:
    """D05: Scorch-dependent shrub mortality logistic curve."""
    s = clamp(scorch_pct, 0.0, 100.0)
    z = alpha + beta * s
    try:
        return 1.0 / (1.0 + math.exp(-z))
    except OverflowError:
        return 0.0 if -z > 0 else 1.0


def calc_fire_herb_post_regrowth(resprout_pot: float, mortality_pct: float) -> float:
    """D06: Post-fire herbaceous regrowth response."""
    m = clamp(mortality_pct, 0.0, 100.0) / 100.0
    survival = max(0.0, 1.0 - m)
    return max(0.0, resprout_pot) * survival


# ==============================================================================
# Canonical Structure-of-Arrays (SoA) Data Structures
# ==============================================================================


class LandscapeCellBuffers:
    """Canonical SoA layout for plot/cell-level attributes (length N)."""

    def __init__(self, n: int):
        self.n = n
        self.plot_id = [0] * n
        self.bps_code = [0] * n
        self.herb_cover = [0.0] * n
        self.herb_height = [0.0] * n
        self.herb_biomass = [0.0] * n
        self.shrub_cover = [0.0] * n
        self.shrub_height = [0.0] * n
        self.ndvi = [0.0] * n
        self.precip = [0.0] * n
        self.elevation = [0.0] * n
        self.fuel_1hr = [0.0] * n
        self.fuel_10hr = [0.0] * n
        self.fuel_100hr = [0.0] * n
        self.fuel_1000hr = [0.0] * n


class LandscapeShrubBuffers:
    """Canonical Segmented CSR SoA layout for sub-canopy shrub attributes (M shrubs across N plots)."""

    def __init__(self, n_cells: int, m_shrubs: int):
        self.n_cells = n_cells
        self.m_shrubs = m_shrubs
        # cell_offsets has length n_cells + 1 (CSR indexing)
        self.cell_offsets = [0] * (n_cells + 1)
        self.species_id = [0] * m_shrubs
        self.cover = [0.0] * m_shrubs
        self.height = [0.0] * m_shrubs
        self.biomass = [0.0] * m_shrubs
        self.stems_per_acre = [0.0] * m_shrubs
        self.crown_width = [0.0] * m_shrubs
