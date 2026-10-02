/// ********************************************************** ///
/// Name: DisturbanceDriver.h                                  ///
/// Desc: Driver for fire and grazing disturbances.            ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#pragma once

#ifndef DISTURBANCEDRIVER_H
#define DISTURBANCEDRIVER_H

#include <iostream>
#include <map>
#include <string>
#include <string_view>

#include "../DataManagement/AnalysisPlot.h"
#include "DisturbanceDIO.h"

namespace RVS
{
namespace Disturbance
{
	/// \brief Scoped enumeration for grazer types
	enum class GrazeType : uint8_t
	{
		Cow,
		Sheep,
		Goat
	};

	class DisturbanceDriver
	{
	public:
		// Backward compatibility aliases for legacy code
		using GRAZE_TYPE = GrazeType;
		static constexpr GrazeType cow = GrazeType::Cow;
		static constexpr GrazeType sheep = GrazeType::Sheep;
		static constexpr GrazeType goat = GrazeType::Goat;

		explicit DisturbanceDriver(RVS::Disturbance::DisturbanceDIO* ddio, const bool suppress_messages = false);
		virtual ~DisturbanceDriver() = default;

		/// \brief Main Disturbance calculation entry point.
		/// \param year Current simulation year
		/// \param ap Analysis plot
		/// \return Status code (0 on success)
		[[nodiscard]] int DisturbanceMain(const int year, RVS::DataManagement::AnalysisPlot* ap);

	private:
		DataManagement::AnalysisPlot* ap = nullptr;
		RVS::Disturbance::DisturbanceDIO* ddio = nullptr;
		bool suppress_messages = false;

		static constexpr double COW_MULTIPLIER = 26.0;
		static constexpr double SMALL_MULTIPLIER = 5.2;

		void burnPlot(std::string_view fireType, const float intensity);

		/// \brief Graze calculation.
		/// Mathematical formulation:
		/// \f[
		///   R_{\text{lbs/ac}} = \frac{M_{\text{type}} \cdot N_{\text{grazers}} \cdot T_{\text{days}}}{A_{\text{acres}}}
		/// \f]
		/// \f[
		///   \Delta_{\text{biomass}} = R_{\text{lbs/ac}} \cdot K_{\text{lbs}\to\text{g}}
		/// \f]
		void grazePlot(
			const GrazeType grazeType,
			const double numberGrazers,
			const double plotArea,
			const double grazeTime);
	};
}
}

#endif // DISTURBANCEDRIVER_H
