/// ********************************************************** ///
/// Name: BiomassDriver.h                                      ///
/// Desc: The 'driver' for biomass calculations. BiomassDriver ///
/// will take one analysis plot and update biomass given       ///
/// current conditions. Conditions (height, cover, time) are   ///
/// changed via Succession and Disturbance modules.            ///
/// Base Class(es): none                                       ///
/// Requires: iostream, BiomassEVT, DIO, RVS_TypeDefs          ///
/// ********************************************************** ///

#pragma once

#ifndef BIOMASSDRIVER_H
#define BIOMASSDRIVER_H

#include <exception>
#include <iostream>
#include <stdio.h>

#include "BiomassEVT.h"
#include "BiomassEquations.h"
#include <DataManagement/DIO.h>
#include <DataManagement/RVS_TypeDefs.h>


namespace RVS
{
namespace Biomass
{
    /// <summary>
    /// BiomassDriver is the primary tool for calculating biomass. It expects an
    /// array of BiomassEVT for a single analysis plot. If the plot contains 
    /// only 1 EVT/DomSpp record, then it's just an array of 1.  
    /// </summary>
	class BiomassDriver
	{
	public:
		/// <summary>
		/// </summary>
		/// <param name="level">The lookup level to use for primary production (herb biomass)</param>
        /// <param name="suppress_messages">Optional. Toggle to 'true' to suppress console messages. Useful when threading.</param>
		BiomassDriver(RVS::Biomass::BiomassLookupLevel level, bool suppress_messages = false);
		virtual ~BiomassDriver(void);

		bool suppress_messages;

		/// <summary>
        /// Main function. Pass a return value and type reference, and BioMain sets them upon completion.
        /// No other function needs to be called to calculate biomass.
        /// </summary>
        /// <param name="plot_num">Analysis plot ID</param>
        /// <param name="biomass_return_value">The calculated biomass of the plot.</param>
        /// <param name="biomass_return_type">The units of the calculated biomass.</param>
        /// <returns>Return code. 0 indicates a clean run.</returns>
        int BioMain(int plot_num, double* biomass_return_value, RVS::Biomass::BiomassReturnType* biomass_return_type);

	private:
		RVS::Biomass::BiomassEVT** evt_records;
		RVS::Biomass::BiomassLookupLevel level;

		/// <summary>
        /// This is the primary biomass calculator for herbs and the fallback for shrubs
        /// (used when shrub equation parsing fails). Loads the Bio_Herbs table and 
        /// looks up the respective biomass value.
        /// </summary>
        /// <param name="bps">5-digit BPS number (without map zone)</param>
        /// <returns>Biomass</returns>
		double calcHerbBiomass(RVS::Biomass::BiomassEVT* evt);
		double calcShrubBiomass(RVS::Biomass::BiomassEVT* evt);

		bool checkShrubInput();
		
	};
}
}

#endif