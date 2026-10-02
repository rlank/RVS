/// ********************************************************** ///
/// Name: RvsCoreBridge.h                                       ///
/// Desc: Modern C++20 zero-copy FFI bridge umbrella header     ///
///       connecting librvs to rvs_core Rust kernels via CXX.   ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#pragma once

#ifndef RVS_CORE_BRIDGE_H
#define RVS_CORE_BRIDGE_H

#include <concepts>
#include <cstdint>
#include <numbers>
#include <ranges>
#include <span>
#include <vector>

// Include CXX generated bridge header
#if __has_include("rvs_core/src/ffi.rs.h")
    #include "rvs_core/src/ffi.rs.h"
#elif __has_include("ffi.rs.h")
    #include "ffi.rs.h"
#else
    #error "CXX bridge header ffi.rs.h not found. Ensure crates/rvs_core is compiled first."
#endif

namespace RVS::Core {

// =============================================================================
// Zero-Copy std::span <-> rust::Slice Converters (O(1), zero allocations)
// =============================================================================

/// \brief Converts a read-only C++20 std::span to a CXX rust::Slice in O(1).
template <typename T>
[[nodiscard]] constexpr rust::Slice<const T> to_rust_slice(std::span<const T> s) noexcept {
    return rust::Slice<const T>(s.data(), s.size());
}

/// \brief Converts a mutable C++20 std::span to a CXX rust::Slice in O(1).
template <typename T>
[[nodiscard]] constexpr rust::Slice<T> to_rust_slice(std::span<T> s) noexcept {
    return rust::Slice<T>(s.data(), s.size());
}

/// \brief Converts a CXX rust::Slice to a read-only C++20 std::span in O(1).
template <typename T>
[[nodiscard]] constexpr std::span<const T> to_span(rust::Slice<const T> s) noexcept {
    return std::span<const T>(s.data(), s.size());
}

/// \brief Converts a CXX rust::Slice to a mutable C++20 std::span in O(1).
template <typename T>
[[nodiscard]] constexpr std::span<T> to_span(rust::Slice<T> s) noexcept {
    return std::span<T>(s.data(), s.size());
}

// =============================================================================
// Biomass Mathematical Kernels (B01 - B22)
// =============================================================================

/// \brief B01: Linear cover-to-biomass (Equation 165).
/// \f[ \mathcal{B} = cf_1 + cf_2 \cdot \max(0, \text{cover}) \f]
[[nodiscard]] inline double eq_165(double cf1, double cf2, double cover) noexcept {
    return RVS::Rust::rust_eq_165(cf1, cf2, cover);
}

/// \brief B02: Exponential linear cover-to-biomass (Equation 201).
/// \f[ \mathcal{B} = \exp(cf_1 + cf_2 \cdot \max(0, \text{cover})) \f]
[[nodiscard]] inline double eq_201(double cf1, double cf2, double cover) noexcept {
    return RVS::Rust::rust_eq_201(cf1, cf2, cover);
}

/// \brief B03: Power-law log-cover to biomass (Equation 202).
/// \f[ \mathcal{B} = \exp(cf_1 + cf_2 \ln(\text{cover})) \f]
[[nodiscard]] inline double eq_202(double cf1, double cf2, double cover) noexcept {
    return RVS::Rust::rust_eq_202(cf1, cf2, cover);
}

/// \brief B04: Power-law volume-to-biomass from dimensions (Equation 636).
/// \f[ \mathcal{B} = \exp\left(cf_1 + cf_2 \ln\left(\frac{L \cdot W \cdot H}{1000}\right)\right) \f]
[[nodiscard]] inline double eq_636(double cf1, double cf2, double length, double width, double height) noexcept {
    return RVS::Rust::rust_eq_636(cf1, cf2, length, width, height);
}

/// \brief B05: Power-law volume-to-biomass from precomputed volume (Equation 636_2).
/// \f[ \mathcal{B} = \exp\left(cf_1 + cf_2 \ln\left(\frac{V}{1000}\right)\right) \f]
[[nodiscard]] inline double eq_636_2(double cf1, double cf2, double vol) noexcept {
    return RVS::Rust::rust_eq_636_2(cf1, cf2, vol);
}

/// \brief B06: Bilinear cover-height to biomass (Equation 743).
/// \f[ \mathcal{B} = cf_1 + cf_2 \cdot \text{cover} \cdot \text{height} \f]
[[nodiscard]] inline double eq_743(double cf1, double cf2, double cover, double height) noexcept {
    return RVS::Rust::rust_eq_743(cf1, cf2, cover, height);
}

/// \brief B07: Log10 width-to-biomass (Equation 998).
/// \f[ \mathcal{B} = 10^{cf_1 + cf_2 \log_{10}(W)} \f]
[[nodiscard]] inline double eq_998(double cf1, double cf2, double width) noexcept {
    return RVS::Rust::rust_eq_998(cf1, cf2, width);
}

/// \brief B08: Log10 length-width to biomass (Equation 999).
/// \f[ \mathcal{B} = 10^{cf_1 + cf_2 \log_{10}(L) + cf_3 \log_{10}(W)} \f]
[[nodiscard]] inline double eq_999(double cf1, double cf2, double cf3, double length, double width) noexcept {
    return RVS::Rust::rust_eq_999(cf1, cf2, cf3, length, width);
}

/// \brief B09: Log10 3-dimensional ellipsoid to biomass (Equation 1000).
/// \f[ \mathcal{B} = 10^{cf_1 + cf_2 \log_{10}(L) + cf_3 \log_{10}(W) + cf_4 \log_{10}(H)} \f]
[[nodiscard]] inline double eq_1000(double cf1, double cf2, double cf3, double cf4, double l, double w, double h) noexcept {
    return RVS::Rust::rust_eq_1000(cf1, cf2, cf3, cf4, l, w, h);
}

/// \brief B10: Linear volume-to-biomass (Equation 1002).
/// \f[ \mathcal{B} = cf_1 + cf_2 (V / 1000) \f]
[[nodiscard]] inline double eq_1002(double cf1, double cf2, double vol) noexcept {
    return RVS::Rust::rust_eq_1002(cf1, cf2, vol);
}

/// \brief B11: Natural log 3-dimensional ellipsoid to biomass (Equation 1008).
/// \f[ \mathcal{B} = \exp(cf_1 + cf_2 \ln(L) + cf_3 \ln(W) + cf_4 \ln(H)) \f]
[[nodiscard]] inline double eq_1008(double cf1, double cf2, double cf3, double cf4, double l, double w, double h) noexcept {
    return RVS::Rust::rust_eq_1008(cf1, cf2, cf3, cf4, l, w, h);
}

/// \brief B12: Multi-parameter power-law biomass (Equation 1012).
/// \f[ \mathcal{B} = \exp(cf_1 + cf_2 \ln(p_1) + cf_3 \ln(p_2)) \f]
[[nodiscard]] inline double eq_1012(double cf1, double cf2, double cf3, double p1, double p2) noexcept {
    return RVS::Rust::rust_eq_1012(cf1, cf2, cf3, p1, p2);
}

/// \brief B13: Polynomial square-root biomass model (Equation 1058).
/// \f[ \mathcal{B} = cf_1 + cf_2 p_1 + cf_3 \sqrt{p_1} \f]
[[nodiscard]] inline double eq_1058(double cf1, double cf2, double cf3, double p1) noexcept {
    return RVS::Rust::rust_eq_1058(cf1, cf2, cf3, p1);
}

/// \brief B14: Basic BAT sum of dimensions biomass (Equation basicBAT).
/// \f[ \mathcal{B} = \exp(cf_1 + cf_2 \ln(L + W)) \f]
[[nodiscard]] inline double eq_basic_bat(double cf1, double cf2, double length, double width) noexcept {
    return RVS::Rust::rust_eq_basic_bat(cf1, cf2, length, width);
}

/// \brief B15: Log10 area and height biomass model (Equation 1153).
/// \f[ \mathcal{B} = 10^{cf_1 + cf_2 \log_{10}(L \cdot W) + cf_3 \log_{10}(H)} \f]
[[nodiscard]] inline double eq_1153(double cf1, double cf2, double cf3, double l, double w, double h) noexcept {
    return RVS::Rust::rust_eq_1153(cf1, cf2, cf3, l, w, h);
}

/// \brief B16: Simple linear single-parameter biomass (Equation 1160).
/// \f[ \mathcal{B} = cf_1 + cf_2 p_1 \f]
[[nodiscard]] inline double eq_1160(double cf1, double cf2, double p1) noexcept {
    return RVS::Rust::rust_eq_1160(cf1, cf2, p1);
}

/// \brief B17: Scaled proportion biomass model (Equation 1161).
/// \f[ \mathcal{B} = cf_1 (p_1 / cf_2) \f]
[[nodiscard]] inline double eq_1161(double cf1, double cf2, double p1) noexcept {
    return RVS::Rust::rust_eq_1161(cf1, cf2, p1);
}

/// \brief B18: Projected crown area from height (Equation PCH).
/// \f[ \text{PCH} = 10^{cf_1 + cf_2 \log_{10}(H)} \f]
[[nodiscard]] inline double eq_PCH(double cf1, double cf2, double height) noexcept {
    return RVS::Rust::rust_eq_pch(cf1, cf2, height);
}

/// \brief B19: Shrub crown width from single-stem crown area.
/// \f[ w = 2 \sqrt{\frac{A_{\text{stem}}}{\pi}} \f]
[[nodiscard]] inline double calcCrownWidth(double crownArea) noexcept {
    return RVS::Rust::rust_calc_crown_width(crownArea);
}

/// \brief B20: Expands single-stem area and canopy cover to stems per acre with low-height dampening.
[[nodiscard]] inline double calcStemsPerAcrePure(double singleStemArea, double cover, double height) noexcept {
    return RVS::Rust::rust_calc_stems_per_acre_pure(singleStemArea, cover, height);
}

/// \brief B21: 3-year weighted moving average for herbaceous holdover biomass.
/// \f[ H = 0.3 P_0 + 0.1 P_1 + 0.05 P_2 \f]
[[nodiscard]] inline double calcHerbHoldover(double p0, double p1, double p2) noexcept {
    return RVS::Rust::rust_calc_herb_holdover(p0, p1, p2);
}

/// \brief B22: Herbaceous biomass canopy competition attenuation.
[[nodiscard]] inline double calcAttenuation(double herbB, double hc, double hh, double sc, double sh) noexcept {
    return RVS::Rust::rust_calc_attenuation(herbB, hc, hh, sc, sh);
}

// =============================================================================
// Fuels Mathematical Kernels (F01 - F12)
// =============================================================================

/// \brief F01: Linear shrub fine fuel loading (Equation 1).
[[nodiscard]] inline double fuels_eq_1(double cf1, double cf2, double p1) noexcept {
    return RVS::Rust::rust_fuels_eq_1(cf1, cf2, p1);
}

/// \brief F02: Power-law fuel loading (Equation 2).
[[nodiscard]] inline double fuels_eq_2(double cf1, double cf2, double p1) noexcept {
    return RVS::Rust::rust_fuels_eq_2(cf1, cf2, p1);
}

/// \brief F03: Exponential fuel accumulation (Equation 13).
[[nodiscard]] inline double fuels_eq_13(double cf1, double cf2, double p1) noexcept {
    return RVS::Rust::rust_fuels_eq_13(cf1, cf2, p1);
}

/// \brief F04: Bilinear interaction fuel loading (Equation 15).
[[nodiscard]] inline double fuels_eq_15(double cf1, double cf2, double p1, double p2) noexcept {
    return RVS::Rust::rust_fuels_eq_15(cf1, cf2, p1, p2);
}

/// \brief F05: Trivariate power-law fuel model (Equation 17).
[[nodiscard]] inline double fuels_eq_17(double cf1, double cf2, double cf3, double p1, double p2) noexcept {
    return RVS::Rust::rust_fuels_eq_17(cf1, cf2, cf3, p1, p2);
}

/// \brief F06: Multi-parameter power-law fuel model (Equation 25).
[[nodiscard]] inline double fuels_eq_25(double cf1, double cf2, double cf3, double cf4, double p1, double p2, double p3) noexcept {
    return RVS::Rust::rust_fuels_eq_25(cf1, cf2, cf3, cf4, p1, p2, p3);
}

/// \brief F07: Quadratic polynomial fuel model (Equation 33).
[[nodiscard]] inline double fuels_eq_33(double cf1, double cf2, double cf3, double p1) noexcept {
    return RVS::Rust::rust_fuels_eq_33(cf1, cf2, cf3, p1);
}

/// \brief F08: Shrub 1-hr wood/bark component calculation.
[[nodiscard]] inline double calcFuelComponentWb(double shrubBiomass, double proportion = 1.0) noexcept {
    return RVS::Rust::rust_calc_fuel_component_wb(shrubBiomass, proportion);
}

/// \brief F09: Shrub 1-hr foliage component calculation.
[[nodiscard]] inline double calcFuelComponentFl(double shrubBiomass, double proportion = 1.0) noexcept {
    return RVS::Rust::rust_calc_fuel_component_fl(shrubBiomass, proportion);
}

/// \brief F10: Total 1-hr fine fuel loading.
[[nodiscard]] inline double calcTotal1HrFuel(double wb, double fl, double herb) noexcept {
    return RVS::Rust::rust_calc_total_1hr_fuel(wb, fl, herb);
}

/// \brief F12: Herbaceous curing and fine fuel loading.
[[nodiscard]] inline double calcHerbLoading(double herbBiomass, double cureRate) noexcept {
    return RVS::Rust::rust_calc_herb_loading(herbBiomass, cureRate);
}

// =============================================================================
// Succession Mathematical Kernels (S01 - S06)
// =============================================================================

/// \brief S01: Geometric annual vegetative growth compounding.
[[nodiscard]] inline double calcGrowthRate(double currentVal, double annualRate) noexcept {
    return RVS::Rust::rust_calc_growth_rate(currentVal, annualRate);
}

/// \brief S01b: Inferred stand establishment age from current canopy cover.
[[nodiscard]] inline double calcPlotAge(double cover, double minCover, double growthRate, double startAge) noexcept {
    return RVS::Rust::rust_calc_plot_age(cover, minCover, growthRate, startAge);
}

/// \brief S02: Climate-driven log-linear herbaceous production.
[[nodiscard]] inline double evaluateHerbGrowth(double b0, double b1, double b2, double b3, double ppt, double ndvi) noexcept {
    return RVS::Rust::rust_evaluate_herb_growth(b0, b1, b2, b3, ppt, ndvi);
}

/// \brief S03: Multi-normal variance quadratic form evaluation (x^T Sigma x).
[[nodiscard]] inline double calcS2bQuadraticForm(
    std::span<const double> x,
    std::span<const double> sigma,
    std::size_t dim
) noexcept {
    return RVS::Rust::rust_calc_s2b_quadratic_form(to_rust_slice(x), to_rust_slice(sigma), dim);
}

/// \brief S04: Confidence interval herbaceous biomass production with smearing factor.
[[nodiscard]] inline double calcHerbBiomassProduction(double meanProd, double s2b, double zScore) noexcept {
    return RVS::Rust::rust_calc_herb_biomass_production(meanProd, s2b, zScore);
}

/// \brief S05: Shrub height succession increment scaling.
[[nodiscard]] inline double scaleShrubHeight(double currentHt, double deltaHt) noexcept {
    return RVS::Rust::rust_scale_shrub_height(currentHt, deltaHt);
}

/// \brief S06: Shrub cover succession increment scaling.
[[nodiscard]] inline double scaleShrubCover(double currentCov, double deltaCov) noexcept {
    return RVS::Rust::rust_scale_shrub_cover(currentCov, deltaCov);
}

// =============================================================================
// Disturbance Mathematical Kernels (D01 - D06)
// =============================================================================

/// \brief D01: Grazing biomass reduction from defoliation pressure.
[[nodiscard]] inline double calcGrazeBiomassReduction(double preBiomass, double utilizationPct) noexcept {
    return RVS::Rust::rust_calc_graze_biomass_reduction(preBiomass, utilizationPct);
}

/// \brief D02: Grazing stubble height reduction from non-linear defoliation mechanics.
[[nodiscard]] inline double calcGrazeHeightReduction(double preHeight, double utilizationPct) noexcept {
    return RVS::Rust::rust_calc_graze_height_reduction(preHeight, utilizationPct);
}

/// \brief D03: Mechanical mowing residual biomass.
[[nodiscard]] inline double calcMowBiomassResidual(double preBiomass, double preHeight, double mowHeight) noexcept {
    return RVS::Rust::rust_calc_mow_biomass_residual(preBiomass, preHeight, mowHeight);
}

/// \brief D04: Wildfire biomass consumption based on fire intensity and severity.
[[nodiscard]] inline double calcFireBiomassConsumption(double biomass, double severityK, double fireIntensity) noexcept {
    return RVS::Rust::rust_calc_fire_biomass_consumption(biomass, severityK, fireIntensity);
}

/// \brief D05: Shrub crown scorch mortality logistic response.
[[nodiscard]] inline double calcFireShrubMortality(double scorchPct, double alpha, double beta) noexcept {
    return RVS::Rust::rust_calc_fire_shrub_mortality(scorchPct, alpha, beta);
}

/// \brief D06: Post-fire herbaceous regrowth from resprouting bud banks.
[[nodiscard]] inline double calcFireHerbPostRegrowth(double resproutPot, double mortalityPct) noexcept {
    return RVS::Rust::rust_calc_fire_herb_post_regrowth(resproutPot, mortalityPct);
}

/// \brief Grazing intake animal biomass removal amount.
[[nodiscard]] inline double calcGrazeRemovalAmount(double intakeLbsPerDay, double numAnimals, double days, double areaAcres) noexcept {
    return RVS::Rust::rust_calc_graze_removal_amount(intakeLbsPerDay, numAnimals, days, areaAcres);
}

// =============================================================================
// Zero-Copy SoA Batch Evaluation Endpoints (Features 9, 10, 11, 12)
// =============================================================================

/// \brief Parallel batch evaluation of biomass attenuation and shrub stems per acre.
/// \return 0 on success, -1 on cell buffer mismatch, -2 on shrub buffer mismatch, -99 on caught panic.
inline int32_t batch_evaluate_biomass(
    std::span<double> herb_biomass,
    std::span<const double> herb_cover,
    std::span<const double> herb_height,
    std::span<const double> shrub_cover,
    std::span<const double> shrub_height,
    std::span<double> shrub_stems,
    std::span<const double> shrub_heights,
    std::span<const double> shrub_covers,
    std::span<const uint32_t> shrub_species,
    std::span<const double> ref_cf1,
    std::span<const double> ref_cf2
) noexcept {
    return RVS::Rust::rust_batch_evaluate_biomass(
        to_rust_slice(herb_biomass),
        to_rust_slice(herb_cover),
        to_rust_slice(herb_height),
        to_rust_slice(shrub_cover),
        to_rust_slice(shrub_height),
        to_rust_slice(shrub_stems),
        to_rust_slice(shrub_heights),
        to_rust_slice(shrub_covers),
        to_rust_slice(shrub_species),
        to_rust_slice(ref_cf1),
        to_rust_slice(ref_cf2)
    );
}

/// \brief Parallel batch evaluation of 1-hour fine fuels across landscape cells.
/// \return 0 on success, -1 on buffer mismatch, -99 on caught panic.
inline int32_t batch_evaluate_fuels(
    std::span<double> out_1hr_fuel,
    std::span<const double> cells_herb_biomass,
    double cure_rate
) noexcept {
    return RVS::Rust::rust_batch_evaluate_fuels(
        to_rust_slice(out_1hr_fuel),
        to_rust_slice(cells_herb_biomass),
        cure_rate
    );
}

/// \brief Parallel batch evaluation of succession with spatially varying deltas.
/// \return 0 on success, -1 on buffer mismatch, -99 on caught panic.
inline int32_t batch_evaluate_succession(
    std::span<double> shrub_height,
    std::span<double> shrub_cover,
    std::span<const double> delta_height,
    std::span<const double> delta_cover
) noexcept {
    return RVS::Rust::rust_batch_evaluate_succession(
        to_rust_slice(shrub_height),
        to_rust_slice(shrub_cover),
        to_rust_slice(delta_height),
        to_rust_slice(delta_cover)
    );
}

/// \brief Parallel batch evaluation of succession with uniform scalar deltas.
/// \return 0 on success, -1 on buffer mismatch, -99 on caught panic.
inline int32_t batch_evaluate_succession(
    std::span<double> shrub_height,
    std::span<double> shrub_cover,
    double delta_shrub_ht,
    double delta_shrub_cov
) noexcept {
    return RVS::Rust::rust_batch_evaluate_succession_scalar(
        to_rust_slice(shrub_height),
        to_rust_slice(shrub_cover),
        delta_shrub_ht,
        delta_shrub_cov
    );
}

/// \brief Parallel batch evaluation of grazing disturbance defoliation.
/// \return 0 on success, -1 on buffer mismatch, -99 on caught panic.
inline int32_t batch_evaluate_disturbance_graze(
    std::span<double> herb_biomass,
    std::span<double> herb_height,
    double utilization_pct
) noexcept {
    return RVS::Rust::rust_batch_evaluate_disturbance_graze(
        to_rust_slice(herb_biomass),
        to_rust_slice(herb_height),
        utilization_pct
    );
}

/// \brief Parallel batch canopy competition attenuation across cell buffers.
inline void batch_calc_attenuation(
    std::span<double> herb_b,
    std::span<const double> hc,
    std::span<const double> hh,
    std::span<const double> sc,
    std::span<const double> sh
) noexcept {
    RVS::Rust::rust_batch_calc_attenuation(
        to_rust_slice(herb_b),
        to_rust_slice(hc),
        to_rust_slice(hh),
        to_rust_slice(sc),
        to_rust_slice(sh)
    );
}

} // namespace RVS::Core

#endif // RVS_CORE_BRIDGE_H
