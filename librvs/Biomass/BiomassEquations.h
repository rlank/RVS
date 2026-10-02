/// ********************************************************** ///
/// Name: BiomassEquations.h                                   ///
/// Desc: Equations for calculating biomass. These are hard    ///
/// coded for now but will eventually implement an equation    ///
/// parser for reading equations from the database (only to    ///
/// be used if changed to modified to limit DB reads).         ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#ifndef BIOMASSEQUATIONS_H
#define BIOMASSEQUATIONS_H

#include <cmath>
#include <map>
#include <span>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

namespace RVS
{
namespace Biomass
{
	class BiomassEquations
	{
	public:
		/// \brief Dispatches and computes Aboveground Total (BAT) shrub biomass.
		/// \param equationNumber Equation identifier from the biomass crosswalk database
		/// \param coefs Parameter coefficients (typically length 2 to 4)
		/// \param params Mapping of morphological parameter names (COV, HT, LEN, WID, VOL) to measurements
		/// \return Calculated biomass in grams or specified model units
		[[nodiscard]] static double eq_BAT(
			const int equationNumber,
			std::span<const double> coefs,
			const std::map<std::string, double>& params);

		/// \brief Backward-compatible overload for legacy pointer callers.
		[[nodiscard]] static double eq_BAT(
			const int equationNumber,
			const double* coefs,
			const std::map<std::string, double>* params)
		{
			if (!coefs || !params)
			{
				return 0.0;
			}
			return eq_BAT(equationNumber, std::span<const double>(coefs, 4), *params);
		}

		/// \brief Predicted crown height/area calculation from height.
		/// Mathematical formulation:
		/// \f[
		///   \text{PCH} = 10^{cf_1 + cf_2 \log_{10}(\text{height})}
		/// \f]
		[[nodiscard]] static double eq_PCH(const double cf1, const double cf2, const double height) noexcept;

		/// \brief Linear cover model (Equation 165).
		/// \f[
		///   B = cf_1 + cf_2 \cdot \text{cover}
		/// \f]
		[[nodiscard]] static constexpr double eq_165(const double cf1, const double cf2, const double cover) noexcept
		{
			return cf1 + cf2 * cover;
		}

		/// \brief Exponential linear cover model (Equation 201).
		/// \f[
		///   B = \exp(cf_1 + cf_2 \cdot \text{cover})
		/// \f]
		[[nodiscard]] static double eq_201(const double cf1, const double cf2, const double cover) noexcept;

		/// \brief Power law cover model (Equation 202).
		/// \f[
		///   B = \exp(cf_1 + cf_2 \ln(\text{cover}))
		/// \f]
		[[nodiscard]] static double eq_202(const double cf1, const double cf2, const double cover) noexcept;

		/// \brief Ellipsoidal volume log-linear model (Equation 636).
		/// \f[
		///   B = \exp\left(cf_1 + cf_2 \ln\left(\frac{\text{length} \cdot \text{width} \cdot \text{height}}{1000}\right)\right)
		/// \f]
		[[nodiscard]] static double eq_636(const double cf1, const double cf2, const double length, const double width, const double height) noexcept;

		/// \brief Explicit volume log-linear model (Equation 636-2).
		/// \f[
		///   B = \exp\left(cf_1 + cf_2 \ln\left(\frac{\text{vol}}{1000}\right)\right)
		/// \f]
		[[nodiscard]] static double eq_636_2(const double cf1, const double cf2, const double vol) noexcept;

		/// \brief Crown cover and height interaction model (Equation 743).
		/// \f[
		///   B = cf_1 + cf_2 \cdot \text{cover} \cdot \text{height}
		/// \f]
		[[nodiscard]] static constexpr double eq_743(const double cf1, const double cf2, const double cover, const double height) noexcept
		{
			return cf1 + cf2 * cover * height;
		}

		/// \brief Log10 width power-law model (Equation 998).
		/// \f[
		///   B = 10^{cf_1 + cf_2 \log_{10}(\text{width})}
		/// \f]
		[[nodiscard]] static double eq_998(const double cf1, const double cf2, const double width) noexcept;

		/// \brief Bivariate log10 length-width power-law model (Equation 999).
		/// \f[
		///   B = 10^{cf_1 + cf_2 \log_{10}(\text{length}) + cf_3 \log_{10}(\text{width})}
		/// \f]
		[[nodiscard]] static double eq_999(const double cf1, const double cf2, const double cf3, const double length, const double width) noexcept;

		/// \brief Trivariate log10 length-width-height power-law model (Equation 1000).
		/// \f[
		///   B = 10^{cf_1 + cf_2 \log_{10}(\text{length}) + cf_3 \log_{10}(\text{width}) + cf_4 \log_{10}(\text{height})}
		/// \f]
		[[nodiscard]] static double eq_1000(const double cf1, const double cf2, const double cf3, const double cf4, const double length, const double width, const double height) noexcept;

		/// \brief Scaled volume linear model (Equation 1002).
		/// \f[
		///   B = cf_1 + cf_2 \left(\frac{\text{vol}}{1000}\right)
		/// \f]
		[[nodiscard]] static constexpr double eq_1002(const double cf1, const double cf2, const double vol) noexcept
		{
			return cf1 + cf2 * (vol / 1000.0);
		}

		/// \brief Trivariate natural log exponential model (Equation 1008).
		/// \f[
		///   B = \exp(cf_1 + cf_2 \ln(\text{length}) + cf_3 \ln(\text{width}) + cf_4 \ln(\text{height}))
		/// \f]
		[[nodiscard]] static double eq_1008(const double cf1, const double cf2, const double cf3, const double cf4, const double length, const double width, const double height) noexcept;

		/// \brief Bivariate natural log exponential model (Equation 1012).
		/// \f[
		///   B = \exp(cf_1 + cf_2 \ln(p_1) + cf_3 \ln(p_2))
		/// \f]
		[[nodiscard]] static double eq_1012(const double cf1, const double cf2, const double cf3, const double p1, const double p2) noexcept;

		/// \brief Polynomial and square-root model (Equation 1058).
		/// \f[
		///   B = cf_1 + cf_2 \cdot p_1 + cf_3 \sqrt{p_1}
		/// \f]
		[[nodiscard]] static double eq_1058(const double cf1, const double cf2, const double cf3, const double p1) noexcept;

		/// \brief Summed dimensions natural log exponential model (Basic BAT).
		/// \f[
		///   B = \exp(cf_1 + cf_2 \ln(\text{length} + \text{width}))
		/// \f]
		[[nodiscard]] static double eq_basicBAT(const double cf1, const double cf2, const double length, const double width) noexcept;

		/// \brief Area and height log10 power-law model (Equation 1153).
		/// \f[
		///   B = 10^{cf_1 + cf_2 \log_{10}(\text{length} \cdot \text{width}) + cf_3 \log_{10}(\text{height})}
		/// \f]
		[[nodiscard]] static double eq_1153(const double cf1, const double cf2, const double cf3, const double length, const double width, const double height) noexcept;

		/// \brief Linear parameter model (Equation 1160).
		/// \f[
		///   B = cf_1 + cf_2 \cdot p_1
		/// \f]
		[[nodiscard]] static constexpr double eq_1160(const double cf1, const double cf2, const double p1) noexcept
		{
			return cf1 + cf2 * p1;
		}

		/// \brief Scaled ratio parameter model (Equation 1161).
		/// \f[
		///   B = cf_1 \left(\frac{p_1}{cf_2}\right)
		/// \f]
		[[nodiscard]] static constexpr double eq_1161(const double cf1, const double cf2, const double p1) noexcept
		{
			return cf1 * (p1 / cf2);
		}

		/// \brief Mathematical parser shunt stub.
		[[nodiscard]] static double shunt(std::string_view equation, const std::map<std::string, double>& variables);
	};
}
}

#endif // BIOMASSEQUATIONS_H
