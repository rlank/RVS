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
		bool suppress_messages;

		int determineCurrentClass(int year, std::vector<double> endpoints);
		std::list<string> makeSpeciesList(std::map<string, string> strVals);
		void addNewSpecies(std::vector<string> sClassSppCodes);

	};
}
}

#endif