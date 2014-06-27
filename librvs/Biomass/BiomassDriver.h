/// ********************************************************** ///
/// Name: BiomassDriver.h                                      ///
/// Desc: The 'driver' for biomass calculations. BiomassDriver ///
/// will take one analysis plot and update biomass given       ///
/// current conditions. Conditions (height, cover, time) are   ///
/// changed via Succession and Disturbance modules.            ///
/// Base Class(es): none                                       ///
/// ********************************************************** ///

#pragma once

#ifndef BIOMASSDRIVER_H
#define BIOMASSDRIVER_H

#include <iostream>

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
		// Constructor
		// <param name="level">The lookup level to use for primary production (herb biomass)</param>
        // <param name="suppress_messages">Optional. Toggle to 'true' to suppress console messages. Useful when threading.</param>
		BiomassDriver(RVS::Biomass::BiomassDIO* bdio, bool suppress_messages = false);
		virtual ~BiomassDriver(void);

        // Main function. Pass a return value and type reference, and BioMain sets them upon completion.
        // No other function needs to be called to calculate biomass.
        // <param name="plot_num">Analysis plot ID</param>
        // <param name="biomass_return_value">The calculated biomass of the plot.</param>
        // <param name="biomass_return_type">The units of the calculated biomass.</param>
        // <returns>Return code. 0 indicates a clean run.</returns>
        int* BioMain(int year, RVS::DataManagement::AnalysisPlot* ap, double* retShrubBiomass, double* retHerbBiomass);

		const float EXPANSION_FACTOR = 4046.8564224f;

	private:
		RVS::Biomass::BiomassDIO* bdio;
		RVS::DataManagement::AnalysisPlot* ap;
		bool suppress_messages;

		// Constants for herbaceous biomass calculation
		//$$ TODO get these from DB, not const
		const float INTERCEPT = -19.4346f;
		const float LN_PRECIP = 0.141f;
		const float LN_NDVI = 3.0056f;

		double calcShrubBiomass(RVS::DataManagement::SppRecord* record);
		double calcStemsPerAcre(RVS::DataManagement::SppRecord* record);
		double calcHerbBiomass(int year);
		// Given cover, calculate the holdover amount
		double calcHerbReduction(double totalShrubCover);

	};
}
}

#endif