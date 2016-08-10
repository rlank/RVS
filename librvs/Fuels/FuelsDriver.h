/// ********************************************************** ///
/// Name: FuelsDriver.h                                        ///
/// Desc: Main driver for the Fuels module. Contains the       ///
/// method to calculate all fuels elements found in the        ///
/// appropriate database. At a minimum this will be 1, 10, 100 ///
/// hour fuels and optionally live/dead measures as well.      ///
/// ********************************************************** ///

#pragma once

#include <iostream>
#include <map>

#include "../DataManagement/AnalysisPlot.h"
#include "../Disturbance/DisturbAction.h"
#include "FuelsDIO.h"
#include "FuelsEquations.h"

using namespace std;

namespace RVS
{
namespace Fuels
{
	class FuelsDriver
	{
	public:
		FuelsDriver(RVS::Fuels::FuelsDIO* fdio, bool suppress_messages = false);
		virtual ~FuelsDriver(void);

		// Main fuels calculation function. Expects an AnalysisPlot object (with biomass information)
		int* FuelsMain(int year, RVS::DataManagement::AnalysisPlot* ap);

	private:
		// Fuel Input/Output module
		RVS::Fuels::FuelsDIO* fdio;
		// Toggle debugging messages
		bool suppress_messages;

		DataManagement::AnalysisPlot* ap;
		
		// Processes a record in the equation table to calculate a fuel value
		double calcShrubFuel(int equationNumber, RVS::DataManagement::SppRecord* spp);

		// Current FBFM
		string calcFBFMDry();
		string calcFBFMHumid();

		int switchClimateFBFM(RVS::DataManagement::DataTable* dt, RVS::DataManagement::AnalysisPlot* ap);

		double calc1HrFuel(double biomass);
		double calc1HrWoodBark(double biomass);
		double calc1HrFoliage(double biomass);

		double calc10HrFuel(double biomass);
		double calc100HrFuel(double biomass);
		double calc1000HrFuel(double height);

		void applyDisturbance(int year);
	};
}
}
