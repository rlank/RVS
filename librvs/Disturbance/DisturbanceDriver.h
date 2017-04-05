/// ********************************************************** ///
/// Name: DisturbanceDriver.h                                  ///
/// Desc:                                                      ///
/// ********************************************************** ///

#pragma once

#include <iostream>
#include <map>

#include "../DataManagement/AnalysisPlot.h"
#include "DisturbanceDIO.h"

using namespace std;

namespace RVS
{
namespace Disturbance
{
	class DisturbanceDriver
	{
	public:
		DisturbanceDriver(RVS::Disturbance::DisturbanceDIO* fdio, bool suppress_messages = false);
		virtual ~DisturbanceDriver(void);

		// Main Disturbance calculation function. Expects an AnalysisPlot object (with biomass information)
		int* DisturbanceMain(int year, RVS::DataManagement::AnalysisPlot* ap);

		enum GRAZE_TYPE {cow, sheep, goat};

	private:
		DataManagement::AnalysisPlot* ap;

		void burnPlot(const string fireType, float intensity);

		// Graze. Takes number of grazers, area of plot in acres, and graze time in days
		void grazePlot(GRAZE_TYPE grazeType, double numberGrazers, double plotArea, double grazeTime);
		
		// Disturbance Input/Output module
		RVS::Disturbance::DisturbanceDIO* ddio;
		// Toggle debugging messages
		bool suppress_messages;

		double cow_multiplier = 26;
		double small_multiplier = 5.2;

	};
}
}
