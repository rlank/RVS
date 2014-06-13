/// ********************************************************** ///
/// Name: FuelsDriver.h                                        ///
/// Desc: Main driver for the Fuels module. Contains the       ///
/// method to calculate all fuels elements found in the        ///
/// appropriate database. At a minimum this will be 1, 10, 100 ///
/// hour fuels and optionally live/dead measures as well.      ///
/// ********************************************************** ///

#pragma once

#ifndef FUELSDRIVER_H
#define FUELSDRIVER_H

#include <iostream>
#include <map>

#include "../DataManagement/AnalysisPlot.h"
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

		// Calculates the percent of live. 100-this is the percent of dead
		double calcPercentLive(RVS::DataManagement::SppRecord* spp, int fuelClass);
		// (Temporary) hard-coded list of equations to calculate percent live
		vector<int> makePercentLiveList(int fuelClass);
		// (Temporary) hard-coded list of equations to calculate percent dead
		vector<int> makePercentDeadList(int fuelClass);

		// Processes a record in the equation table to calculate a fuel value
		double calcShrubFuel(int equationNumber, RVS::DataManagement::SppRecord* spp);

	};
}
}

#endif