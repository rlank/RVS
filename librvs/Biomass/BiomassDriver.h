/// ********************************************************** ///
/// Name: BiomassDriver.h                                      ///
/// Desc: The 'driver' for biomass calculations. BiomassDriver ///
/// will take one analysis plot and update biomass given       ///
/// current conditions. Conditions (height, cover, time) are   ///
/// changed via Succession and Disturbance modules.            ///
/// Standard: C++20                                            ///
/// Base Class(es): none                                       ///
/// ********************************************************** ///

#pragma once

#include <iostream>
#include <numbers>
#include <span>
#include <string>
#include <string_view>

#include "BiomassDIO.h"
#include "BiomassEquations.h"
#include "../DataManagement/AnalysisPlot.h"
#include "../DataManagement/SppRecord.h"

namespace RVS
{
namespace Biomass
{
	class BiomassDriver
	{
	public:
		/// \brief Constructor
		/// \param bdio Biomass data I/O provider
		/// \param suppress_messages Toggle to 'true' to suppress console messages
		explicit BiomassDriver(RVS::Biomass::BiomassDIO* bdio, const bool suppress_messages = false);
		virtual ~BiomassDriver() = default;

		/// \brief Main biomass calculation entry point.
		/// \param year Current simulation year
		/// \param climate Climate scenario name
		/// \param ap Pointer to analysis plot to process
		/// \return Return code (0 on success)
		[[nodiscard]] int BioMain(const int year, std::string_view climate, RVS::DataManagement::AnalysisPlot* ap);

		/// \brief Conversion factor: square meters in one acre.
		static constexpr double EXPANSION_FACTOR = 4046.8564224;

		// --- Pure Mathematical Subroutines (Rust Spin-out Candidates) ---

		/// \brief Calculates shrub crown width from single-stem crown area using circle geometry.
		/// \f[
		///   w = 2 \sqrt{\frac{A}{\pi}}
		/// \f]
		/// \param crownArea Single stem crown area in cm^2
		/// \return Crown diameter/width in centimeters
		[[nodiscard]] static double calcCrownWidth(const double crownArea) noexcept;

		/// \brief Expands single-stem area and canopy cover to stems per acre with low-height dampening.
		/// \f[
		///   E = \frac{\text{EXPANSION\_FACTOR}}{A_{\text{stem}} \cdot 10^{-4}}
		/// \f]
		/// \f[
		///   S = E \cdot \left(\frac{\text{cover}}{100}\right)
		/// \f]
		/// \f[
		///   R = 0.0053 \cdot \exp(0.0809 \cdot \text{height})
		/// \f]
		/// \f[
		///   S_{\text{final}} = \begin{cases}
		///     S \cdot R & \text{if } R \le 0.453595949 \\
		///     S & \text{otherwise}
		///   \end{cases}
		/// \f]
		/// \param singleStemArea Single stem crown area in cm^2
		/// \param cover Shrub percent canopy cover (0 - 100)
		/// \param height Shrub height in centimeters
		/// \return Density in stems per acre
		[[nodiscard]] static double calcStemsPerAcrePure(
			const double singleStemArea,
			const double cover,
			const double height) noexcept;

		/// \brief Computes herbaceous holdover biomass from three prior years of production.
		/// \f[
		///   H_{\text{holdover}} = 0.3 \cdot P_0 + 0.1 \cdot P_1 + 0.05 \cdot P_2
		/// \f]
		/// \param p0 Production at year t-1 (most recent)
		/// \param p1 Production at year t-2
		/// \param p2 Production at year t-3
		/// \return Holdover biomass
		[[nodiscard]] static constexpr double calcHerbHoldover(
			const double p0,
			const double p1,
			const double p2) noexcept
		{
			return (p0 * 0.3) + (p1 * 0.1) + (p2 * 0.05);
		}

		/// \brief Computes herbaceous biomass attenuation under shrub canopy competition.
		/// \f[
		///   I_{\text{herb}} = \text{cover}_{\text{herb}} \cdot \text{height}_{\text{herb}}
		/// \f]
		/// \f[
		///   I_{\text{shrub}} = \text{cover}_{\text{shrub}} \cdot \text{height}_{\text{shrub}}
		/// \f]
		/// \f[
		///   r = \min\left(\frac{I_{\text{herb}}}{I_{\text{shrub}}}, \frac{I_{\text{shrub}}}{I_{\text{herb}}}\right)
		/// \f]
		/// \f[
		///   B_{\text{attenuated}} = B_{\text{herb}} \cdot r
		/// \f]
		/// \param herbBiomass Raw herbaceous biomass
		/// \param herbCover Herbaceous percent canopy cover
		/// \param herbHeight Herbaceous canopy height
		/// \param shrubCover Total shrub percent canopy cover
		/// \param shrubHeight Average shrub canopy height
		/// \return Attenuated herbaceous biomass
		[[nodiscard]] static constexpr double calcAttenuation(
			const double herbBiomass,
			const double herbCover,
			const double herbHeight,
			const double shrubCover,
			const double shrubHeight) noexcept
		{
			const double herbIndex = herbCover * herbHeight;
			const double shrubIndex = shrubCover * shrubHeight;

			if (shrubIndex <= 0.0 || herbIndex <= 0.0)
			{
				return herbBiomass;
			}

			double ratio = herbIndex / shrubIndex;
			if (ratio > 1.0)
			{
				ratio = 1.0 / ratio;
			}
			return herbBiomass * ratio;
		}

	private:
		RVS::Biomass::BiomassDIO* bdio;
		RVS::DataManagement::AnalysisPlot* ap{nullptr};
		bool suppress_messages;
		std::string climate;

		[[nodiscard]] double calcShrubBiomass(RVS::DataManagement::SppRecord* record);
		[[nodiscard]] double calcStemsPerAcre(RVS::DataManagement::SppRecord* record);
		[[nodiscard]] double calcHerbHoldover() const noexcept;
		[[nodiscard]] double calcAttenuation(const double herbBiomass) const noexcept;
	};
}
}
