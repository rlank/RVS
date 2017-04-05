/// ********************************************************** ///
/// Name: SuccessionDriver.h                                      ///
/// Desc:           ///
/// Base Class(es): none                                       ///
/// ********************************************************** ///

#pragma once

#ifndef SUCCESSIONDRIVER_H
#define SUCCESSIONDRIVER_H

#include <iostream>
#include <list>

#include "SuccessionDIO.h"
#include "../DataManagement/AnalysisPlot.h"
#include "../DataManagement/SppRecord.h"

namespace RVS
{
namespace Succession
{
	class SuccessionDriver
	{
	public:
		
		SuccessionDriver(RVS::Succession::SuccessionDIO* sdio, bool suppress_messages = false);
		virtual ~SuccessionDriver(void);

        int* SuccessionMain(int year, string* climate, RVS::DataManagement::AnalysisPlot* ap);

	private:
		RVS::Succession::SuccessionDIO* sdio;
		RVS::DataManagement::AnalysisPlot* ap;
		vector<RVS::DataManagement::SppRecord*>* shrubs;
		bool suppress_messages;
		string* climate;

		const float MSE = 0.1276825f;
		const float SMEAR = 1.06431775f;
		static double** covariance_matrix;

		vector<map<string, string>> successionStrParameters;
		vector<map<string, double>> successionNumParameters;

		void loadSuccessionVals(bool* doNotModel);

		int determineCurrentClass();
		
		int plotAge(int sclass);
		int plotAgeCalculation(double cover, map<string, string> strVals, map<string, double> numVals);

		void growStage(map<string, string> strVals, map<string, double> numVals);

		void growHerbs(double* herbCover, double* herbHeight, double* production);

		list<string> makeSpeciesList(map<string, string> strVals);
		void addNewSpecies(vector<string> sClassSppCodes);

		double calcProduction(int year);
		void calcConfidence(int year, double biomass, double* lower, double* upper, double* level);
		double calc_s2b(double* lnNDVI, double* lnPPT);
		//double calc_s2b(string* grp_id, double* lnNDVI, double* lnPPT);
		double** matrix_mult(double** A, int aRow, int aCol, double** B, int bRow, int bCol);
		double** matrix_trans(double** A, int aRow, int aCol);
		double** generate_dummy_variables(int index, double* lnPPT, double* lnNDVI);

	};
}
}

#endif