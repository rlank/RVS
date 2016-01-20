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
        // <returns>Return code. 0 indicates a clean run.</returns>
        int* BioMain(int year, RVS::DataManagement::AnalysisPlot* ap);

		const float EXPANSION_FACTOR = 4046.8564224f;

	private:
		RVS::Biomass::BiomassDIO* bdio;
		RVS::DataManagement::AnalysisPlot* ap;
		bool suppress_messages;

		// Constants for herbaceous biomass calculation

		const float MSE = 0.05418119f;
		const float SMEAR = 1.02456869001763;
		static double** covariance_matrix;

		double calcShrubBiomass(RVS::DataManagement::SppRecord* record);
		double calcStemsPerAcre(RVS::DataManagement::SppRecord* record);
		double calcHerbBiomass(int year);
		void calcConfidence(int year, double biomass, double* lower, double* upper, double* level);
		// Given cover, calculate the holdover amount
		double calcHerbHoldover(double totalShrubCover);
		double calcAttenuation(double herbBiomass);
		void growHerbs(double* herbCover, double* herbHeight, double oldBiomass);

		double calc_s2b(string* grp_id, double* lnNDVI, double* lnPPT);
		double** matrix_mult(double** A, int aRow, int aCol, double** B, int bRow, int bCol);
		double** matrix_trans(double** A, int aRow, int aCol);
		double** generate_dummy_variables(int index, double* lnPPT, double* lnNDVI);
	};
}
}

#endif