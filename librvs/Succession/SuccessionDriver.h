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

        int* SuccessionMain(int year, RVS::DataManagement::AnalysisPlot* ap);

	private:
		RVS::Succession::SuccessionDIO* sdio;
		RVS::DataManagement::AnalysisPlot* ap;
		std::vector<RVS::DataManagement::SppRecord*>* shrubs;
		bool suppress_messages;

		bool isShrubDominated;

		std::map<string, string> strVals_primary;
		std::map<string, double> numVals_primary;
		std::map<string, string> strVals_secondary;
		std::map<string, double> numVals_secondary;
		std::map<string, string> strVals_tertiary;
		std::map<string, double> numVals_tertiary;

		int determineCurrentClass();
		int determineVegClass(string cohortType, double cover);
		int relativeTimeInStage(int sclass);
		int relativeTimeInStageAdjuster(double cover, std::map<string, string> strVals, std::map<string, double> numVals);

		void growStage(std::map<string, string> strVals, std::map<string, double> numVals);

		std::list<string> makeSpeciesList(std::map<string, string> strVals);
		void addNewSpecies(std::vector<string> sClassSppCodes);

	};
}
}

#endif