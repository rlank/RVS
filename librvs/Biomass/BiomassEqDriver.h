/// ********************************************************** ///
/// Name: BiomassEqDriver.h                                    ///
/// Desc: Driver for executing individual biomass equations.   ///
/// Standard: C++20                                            ///
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
			BiomassEqDriver(RVS::Biomass::BiomassDIO* bdio, bool suppress_messages = false);
			virtual ~BiomassEqDriver(void) = default;

			// <returns>Return code. 0 indicates a clean run.</returns>
			int* BioMain(int equationNumber, RVS::DataManagement::AnalysisPlot* ap);

			static constexpr double EXPANSION_FACTOR = 4046.8564224;

		private:
			RVS::Biomass::BiomassDIO* bdio;
			RVS::DataManagement::AnalysisPlot* ap = nullptr;
			bool suppress_messages;

			// Constants for herbaceous biomass calculation
			static constexpr double INTERCEPT = -19.4346;
			static constexpr double LN_PRECIP = 0.141;
			static constexpr double LN_NDVI = 3.0056;

			double calcShrubBiomass(int equationNumber, RVS::DataManagement::SppRecord* record);
			double calcStemsPerAcre(RVS::DataManagement::SppRecord* record);
		};
	}
}

#endif // BIOMASSEQDRIVER_H