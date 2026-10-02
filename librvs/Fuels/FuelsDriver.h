/// ********************************************************** ///
/// Name: FuelsDriver.h                                        ///
/// Desc: Main driver for the Fuels module. Contains the       ///
///       method to calculate all fuels elements found in the  ///
///       appropriate database.                                ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#pragma once

#ifndef FUELSDRIVER_H
#define FUELSDRIVER_H

#include <iostream>
#include <map>
#include <string>
#include <string_view>

#include "../DataManagement/AnalysisPlot.h"
#include "../Disturbance/DisturbAction.h"
#include "FuelsDIO.h"
#include "FuelsEquations.h"

namespace RVS
{
namespace Fuels
{
	class FuelsDriver
	{
	public:
		explicit FuelsDriver(RVS::Fuels::FuelsDIO* fdio, const bool suppress_messages = false);
		virtual ~FuelsDriver() = default;

		/// \brief Main fuels calculation entry point returning status code.
		/// \param year Current simulation year
		/// \param ap Pointer to analysis plot
		/// \return 0 on success, non-zero on error
		[[nodiscard]] int FuelsMain(const int year, RVS::DataManagement::AnalysisPlot* ap);

	private:
		// Fuel Input/Output module
		RVS::Fuels::FuelsDIO* fdio = nullptr;
		// Toggle debugging messages
		bool suppress_messages = false;

		DataManagement::AnalysisPlot* ap = nullptr;
		
		// Processes a record in the equation table to calculate a fuel value
		[[nodiscard]] double calcShrubFuel(const int equationNumber, RVS::DataManagement::SppRecord* spp);

		// Current FBFM
		[[nodiscard]] std::string calcFBFMDry() const;
		[[nodiscard]] std::string calcFBFMHumid() const;

		[[nodiscard]] int switchClimateFBFM(RVS::DataManagement::DataTable* dt, const RVS::DataManagement::AnalysisPlot* ap);

		[[nodiscard]] double calc1HrFuel(const double biomass) const noexcept
		{
			return FuelsEquations::calc1HrFuel(biomass);
		}

		[[nodiscard]] double calc1HrWoodBark(const double biomass) const noexcept
		{
			return FuelsEquations::calc1HrWoodBark(biomass);
		}

		[[nodiscard]] double calc1HrFoliage(const double biomass) const noexcept
		{
			return FuelsEquations::calc1HrFoliage(biomass);
		}

		[[nodiscard]] double calc10HrFuel(const double biomass) const noexcept;

		[[nodiscard]] double calc100HrFuel(const double biomass) const noexcept
		{
			return FuelsEquations::calc100HrFuel(biomass);
		}

		[[nodiscard]] double calc1000HrFuel(const double height) const noexcept
		{
			return FuelsEquations::calc1000HrFuel(height);
		}

		void applyDisturbance(const int year);
	};
}
}

#endif // FUELSDRIVER_H
