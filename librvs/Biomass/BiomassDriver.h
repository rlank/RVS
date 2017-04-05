/// ********************************************************** ///
/// Name: BiomassDriver.h                                      ///
/// Desc: The 'driver' for biomass calculations. BiomassDriver ///
/// will take one analysis plot and update biomass given       ///
/// current conditions. Conditions (height, cover, time) are   ///
/// changed via Succession and Disturbance modules.            ///
/// Base Class(es): none                                       ///
/// ********************************************************** ///

#pragma once

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
        // <returns>Return code. 0 indicates a clean run.</returns>
        int* BioMain(int year, string* climate, RVS::DataManagement::AnalysisPlot* ap);

		const float EXPANSION_FACTOR = 4046.8564224f;

	private:
		RVS::Biomass::BiomassDIO* bdio;
		RVS::DataManagement::AnalysisPlot* ap;
		bool suppress_messages;
		string* climate;

		// Constants for herbaceous biomass calculation
		double calcShrubBiomass(RVS::DataManagement::SppRecord* record);
		double calcStemsPerAcre(RVS::DataManagement::SppRecord* record);
		double calcHerbHoldover();
		double calcAttenuation(double herbBiomass);
	};
}
}
