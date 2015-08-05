/// ********************************************************** ///
/// Name: BiomassEqDriver.h                                      ///
/// Desc:           ///
/// Base Class(es): none                                       ///
/// ********************************************************** ///

#pragma once

#ifndef BIOMASSEQDRIVER_H
#define BIOMASSEQDRIVER_H

#include <iostream>

#include "BiomassDIO.h"
#include "BiomassEquations.h"
#include "../DataManagement/AnalysisPlot.h"
#include "../DataManagement/SppRecord.h"

namespace RVS
{
	namespace Biomass
	{
		class BiomassEqDriver
		{
		public:
			// Constructor
			// <param name="level">The lookup level to use for primary production (herb biomass)</param>
			// <param name="suppress_messages">Optional. Toggle to 'true' to suppress console messages. Useful when threading.</param>
			BiomassEqDriver(RVS::Biomass::BiomassDIO* bdio, bool suppress_messages = false);
			virtual ~BiomassEqDriver(void);

			// <returns>Return code. 0 indicates a clean run.</returns>
			int* BioMain(int equationNumber, RVS::DataManagement::AnalysisPlot* ap);

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

			double calcShrubBiomass(int equationNumber, RVS::DataManagement::SppRecord* record);
			double calcStemsPerAcre(RVS::DataManagement::SppRecord* record);
		};
	}
}

#endif