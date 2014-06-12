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
#include "../DataManagement/RVS_TypeDefs.h"
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

		// Toggle debugging messages
		bool suppress_messages;
		
		// Main fuels calculation function. Expects an AnalysisPlot object (with biomass information)
		int* FuelsMain(int year, RVS::DataManagement::AnalysisPlot* ap);

	private:
		RVS::Fuels::FuelsDIO* fdio;

		// Calculates the percent of live. 100-this is the percent of dead
		double calcPercentLive();

		// Processes a record in the equation table to calculate a fuels value
		double calcShrubFuel(RVS::DataManagement::DataTable* dt, RVS::DataManagement::SppRecord* spp);

	};
}
}

#endif