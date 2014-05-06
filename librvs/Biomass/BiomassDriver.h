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

#include <exception>
#include <iostream>
#include <stdio.h>

#include "BiomassDIO.h"
#include "BiomassEVT.h"
#include "BiomassEquations.h"
#include "../DataManagement/RVS_TypeDefs.h"
#include "../Fuels/FuelsDriver.h"


namespace RVS
{
namespace Biomass
{
    /// BiomassDriver is the primary tool for calculating biomass. It expects an
    /// array of BiomassEVT for a single analysis plot. If the plot contains 
    /// only 1 EVT/DomSpp record, then it's just an array of 1.  
	class BiomassDriver
	{
	public:
		///
		/// <param name="level">The lookup level to use for primary production (herb biomass)</param>
        /// <param name="suppress_messages">Optional. Toggle to 'true' to suppress console messages. Useful when threading.</param>
		BiomassDriver(int plot_num, RVS::Biomass::BiomassDIO* bdio, RVS::Biomass::BiomassLookupLevel level, bool suppress_messages = false, bool write_intermediate = false);
		virtual ~BiomassDriver(void);

		bool suppress_messages;
		bool write_intermediate;

        /// Main function. Pass a return value and type reference, and BioMain sets them upon completion.
        /// No other function needs to be called to calculate biomass.
        /// <param name="plot_num">Analysis plot ID</param>
        /// <param name="biomass_return_value">The calculated biomass of the plot.</param>
        /// <param name="biomass_return_type">The units of the calculated biomass.</param>
        /// <returns>Return code. 0 indicates a clean run.</returns>
        int* BioMain(int year, double* retShrubBiomass, double* retHerbBiomass);

		inline std::vector<RVS::Biomass::BiomassEVT*> pullRecords() { return evt_records; }

	private:
		int plot_num;
		RVS::Biomass::BiomassDIO* bdio;
		RVS::Biomass::BiomassLookupLevel level;
		std::vector<RVS::Biomass::BiomassEVT*> evt_records;

		const float INTERCEPT = -19.4346;
		const float LN_PRECIP = 0.141;
		const float LN_NDVI = 3.0056;

        /// This is the primary biomass calculator for herbs and the fallback for shrubs
        /// (used when shrub equation parsing fails). Loads the Bio_Herbs table and 
        /// looks up the respective biomass value.
        /// <param name="bps">5-digit BPS number (without map zone)</param>
        /// <returns>Biomass</returns>
		double calcPrimaryProduction(RVS::Biomass::BiomassEVT* evt);
		double calcShrubBiomass(RVS::Biomass::BiomassEVT* evt);
		double calcHerbBiomass(RVS::Biomass::BiomassEVT* evt, int year);
		double calcHerbReduction(double totalShrubCover);

		void createEVTRecords(int plot_num);
	};
}
}

#endif