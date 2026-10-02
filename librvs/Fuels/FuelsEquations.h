/// ********************************************************** ///
/// Name: FuelsEquations.h                                     ///
/// Desc: Pure constexpr/noexcept equations for calculating    ///
///       1, 10, 100, and 1000-hour fuels, fuel proportions,   ///
///       live/dead percentages, and fuel components.          ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#ifndef FUELSEQUATIONS_H
#define FUELSEQUATIONS_H

#include <cmath>
#include <span>

namespace RVS
{
namespace Fuels
{
	class FuelsEquations
	{
	public:
		// ====================================================================
		// Fuel Percentage and Proportion Calculations
		// ====================================================================

		/// \brief Calculates the proportion of a fuel component relative to total fuel.
		/// \f[
		///   P = \begin{cases} \frac{C_{\text{fuel}}}{T_{\text{fuel}}} & \text{if } T_{\text{fuel}} > 0 \\ 0 & \text{otherwise} \end{cases}
		/// \f]
		[[nodiscard]] static constexpr double calcFuelProportion(const double componentFuel, const double totalFuel) noexcept
		{
			return (totalFuel > 0.0) ? (componentFuel / totalFuel) : 0.0;
		}

		/// \brief Calculates the percentage of a fuel component relative to total fuel.
		/// \f[
		///   \% = P \times 100.0
		/// \f]
		[[nodiscard]] static constexpr double calcFuelPercent(const double componentFuel, const double totalFuel) noexcept
		{
			return calcFuelProportion(componentFuel, totalFuel) * 100.0;
		}

		/// \brief Default 1-Hour live fuel percentage (100% live default).
		[[nodiscard]] static constexpr double calc1HrLivePercent() noexcept
		{
			return 100.0;
		}

		/// \brief Calculates 1-Hour live fuel percentage from live and total fuel biomass.
		/// \f[
		///   \%_{1\text{Hr,Live}} = \frac{F_{1\text{Hr,Live}}}{F_{1\text{Hr,Total}}} \times 100.0
		/// \f]
		[[nodiscard]] static constexpr double calc1HrLivePercent(const double liveFuel, const double totalFuel) noexcept
		{
			return calcFuelPercent(liveFuel, totalFuel);
		}

		/// \brief Default 10-Hour live fuel percentage (100% live default).
		[[nodiscard]] static constexpr double calc10HrLivePercent() noexcept
		{
			return 100.0;
		}

		/// \brief Calculates 10-Hour live fuel percentage from live and total fuel biomass.
		/// \f[
		///   \%_{10\text{Hr,Live}} = \frac{F_{10\text{Hr,Live}}}{F_{10\text{Hr,Total}}} \times 100.0
		/// \f]
		[[nodiscard]] static constexpr double calc10HrLivePercent(const double liveFuel, const double totalFuel) noexcept
		{
			return calcFuelPercent(liveFuel, totalFuel);
		}

		/// \brief Default 100-Hour live fuel percentage (100% live default).
		[[nodiscard]] static constexpr double calc100HrLivePercent() noexcept
		{
			return 100.0;
		}

		/// \brief Calculates 100-Hour live fuel percentage from live and total fuel biomass.
		/// \f[
		///   \%_{100\text{Hr,Live}} = \frac{F_{100\text{Hr,Live}}}{F_{100\text{Hr,Total}}} \times 100.0
		/// \f]
		[[nodiscard]] static constexpr double calc100HrLivePercent(const double liveFuel, const double totalFuel) noexcept
		{
			return calcFuelPercent(liveFuel, totalFuel);
		}

		// ====================================================================
		// Fuel Component Subroutines
		// ====================================================================

		/// \brief 1-Hour Wood and Bark fuel component from shrub biomass.
		/// \f[
		///   fs_1 = 2.932237 + 0.353554 \cdot B_{\text{shrub}}
		/// \f]
		[[nodiscard]] static constexpr double calc1HrWoodBark(const double biomass) noexcept
		{
			return 2.932237 + 0.353554 * biomass;
		}

		/// \brief 1-Hour Foliage fuel component from shrub biomass.
		/// \f[
		///   bft = \exp\left(-1.498408 + 0.955207 \ln(B_{\text{shrub}})\right)
		/// \f]
		[[nodiscard]] static double calc1HrFoliage(const double biomass) noexcept
		{
			if (biomass <= 0.0)
			{
				return 0.0;
			}
			const double logBio = std::log(biomass);
			return std::exp(-1.498408 + 0.955207 * logBio);
		}

		/// \brief Total 1-Hour fine fuel from shrub biomass (Wood/Bark + Foliage).
		/// \f[
		///   F_{1\text{Hr}} = fs_1(B_{\text{shrub}}) + bft(B_{\text{shrub}})
		/// \f]
		[[nodiscard]] static double calc1HrFuel(const double biomass) noexcept
		{
			return calc1HrWoodBark(biomass) + calc1HrFoliage(biomass);
		}

		/// \brief 10-Hour fuel component from shrub biomass and average stem size.
		/// \f[
		///   fs_2 = \begin{cases}
		///     -5.192477 + 0.445884 \cdot B_{\text{shrub}} & \text{if } \bar{s} > 22.6796 \\
		///     0.012477 + 0.445884 \cdot B_{\text{shrub}} & \text{otherwise}
		///   \end{cases}
		/// \f]
		[[nodiscard]] static constexpr double calc10HrFuel(const double biomass, const double shrubAvgStem = 0.0) noexcept
		{
			const double intercept = (shrubAvgStem > 22.6796) ? -5.192477 : 0.012477;
			return intercept + 0.445884 * biomass;
		}

		/// \brief 100-Hour fuel component from shrub biomass.
		/// \f[
		///   fs_3 = \exp\left(-3.552846 + 1.005534 \ln(B_{\text{shrub}})\right)
		/// \f]
		[[nodiscard]] static double calc100HrFuel(const double biomass) noexcept
		{
			if (biomass <= 0.0)
			{
				return 0.0;
			}
			const double logBio = std::log(biomass);
			return std::exp(-3.552846 + 1.005534 * logBio);
		}

		/// \brief 1000-Hour fuel component from shrub height.
		/// \f[
		///   fs_4 = \begin{cases}
		///     \frac{3.6853 \log_{10}(100 \cdot ht) - 1.306}{100} & \text{if } ht > 400 \\
		///     0 & \text{otherwise}
		///   \end{cases}
		/// \f]
		[[nodiscard]] static double calc1000HrFuel(const double ht) noexcept
		{
			if (ht <= 400.0)
			{
				return 0.0;
			}
			const double ht_cm = ht * 100.0;
			const double fs4 = (std::log10(ht_cm) * 3.6853 - 1.306) / 100.0;
			return (fs4 > 0.0) ? fs4 : 0.0;
		}

		// ====================================================================
		// Individual Allometric Fuel Equations
		// ====================================================================

		/// \brief Linear Fuel Model (Equation 1).
		/// \f[
		///   F = cf_1 + cf_2 \cdot p_1
		/// \f]
		[[nodiscard]] static constexpr double fuels_eq_1(const double cf1, const double cf2, const double p1) noexcept
		{
			return cf1 + cf2 * p1;
		}

		/// \brief Power Law Fuel Model (Equation 2).
		/// \f[
		///   F = \exp\left(cf_1 + cf_2 \ln(p_1)\right)
		/// \f]
		[[nodiscard]] static double fuels_eq_2(const double cf1, const double cf2, const double p1) noexcept
		{
			if (p1 <= 0.0) return 0.0;
			return std::exp(cf1 + cf2 * std::log(p1));
		}

		/// \brief Exponential Fuel Model (Equation 13).
		/// \f[
		///   F = \exp\left(cf_1 + cf_2 \cdot p_1\right)
		/// \f]
		[[nodiscard]] static double fuels_eq_13(const double cf1, const double cf2, const double p1) noexcept
		{
			return std::exp(cf1 + cf2 * p1);
		}

		/// \brief Linear Interaction Fuel Model (Equation 15).
		/// \f[
		///   F = cf_1 + cf_2 \cdot p_1 \cdot p_2
		/// \f]
		[[nodiscard]] static constexpr double fuels_eq_15(const double cf1, const double cf2, const double p1, const double p2) noexcept
		{
			return cf1 + cf2 * p1 * p2;
		}

		/// \brief Bivariate Log-Linear Fuel Model (Equation 17).
		/// \f[
		///   F = \exp\left(cf_1 + cf_2 \ln(p_1) + cf_3 \ln(p_2)\right)
		/// \f]
		[[nodiscard]] static double fuels_eq_17(const double cf1, const double cf2, const double cf3, const double p1, const double p2) noexcept
		{
			if (p1 <= 0.0 || p2 <= 0.0) return 0.0;
			return std::exp(cf1 + cf2 * std::log(p1) + cf3 * std::log(p2));
		}

		/// \brief Trivariate Log-Linear Fuel Model (Equation 42).
		/// \f[
		///   F = \exp\left(cf_1 + cf_2 \ln(p_1) + cf_3 \ln(p_2) + cf_4 \ln(p_3)\right)
		/// \f]
		[[nodiscard]] static double fuels_eq_42(
			const double cf1, const double cf2, const double cf3, const double cf4,
			const double p1, const double p2, const double p3) noexcept
		{
			if (p1 <= 0.0 || p2 <= 0.0 || p3 <= 0.0) return 0.0;
			return std::exp(cf1 + cf2 * std::log(p1) + cf3 * std::log(p2) + cf4 * std::log(p3));
		}

		/// \brief Bivariate Linear Additive Fuel Model (Equation 46).
		/// \f[
		///   F = cf_1 + cf_2 \cdot p_1 + cf_3 \cdot p_2
		/// \f]
		[[nodiscard]] static constexpr double fuels_eq_46(
			const double cf1, const double cf2, const double cf3,
			const double p1, const double p2) noexcept
		{
			return cf1 + cf2 * p1 + cf3 * p2;
		}

		// ====================================================================
		// High-Level Dispatcher
		// ====================================================================

		/// \brief Dispatches and computes fuel biomass via equation number.
		/// \param equationType The fuel equation identifier (1, 2, 13, 15, 17, 42, 46)
		/// \param coefs Coefficients array/span (up to 4 coefficients)
		/// \param params Predictor variables array/span (up to 3 parameters)
		/// \return Fuel biomass in grams
		[[nodiscard]] static double calcFuels(
			const int equationType,
			std::span<const double> coefs,
			std::span<const double> params) noexcept;

		/// \brief Backward-compatible overload for legacy pointer callers.
		[[nodiscard]] static double calcFuels(
			const int equationType,
			const double* coefs,
			const double* params) noexcept
		{
			if (!coefs || !params)
			{
				return 0.0;
			}
			return calcFuels(equationType, std::span<const double>(coefs, 4), std::span<const double>(params, 3));
		}
	};
}
}

#endif // FUELSEQUATIONS_H
