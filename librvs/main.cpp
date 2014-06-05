/// ********************************************************** ///
/// Name: main.cpp                                             ///
/// Desc: (Test) driver for RVS. Build as execultable and      ///
/// this becomes the program entry point                       ///
/// Base Class(es): none                                       ///
/// Last Modified: 8 Mar 14                                    ///
/// ********************************************************** ///

#include <exception>
#include <iostream>
#include <string>
#include <vector>

//#include <boost/exception/all.hpp>

#include "RVSDEF.h"
#include "DataManagement/DIO.h"
#include "DataManagement/AnalysisPlot.h"
#include "Biomass/BiomassDIO.h"
#include "Biomass/BiomassDriver.h"
#include "Fuels/FuelsDIO.h"

using namespace std;
using namespace RVS;
using namespace RVS::DataManagement;

int* RC = new int(SQLITE_OK);

int main(int argc, char* argv[])
{
    static bool SUPPRESS_MSG = false;
	static int STOPYEAR = 3;  //$$ TODO make this C def
    
	///////////////////////////
	/// User execution args ///
	///////////////////////////

	// Args:
	// 1: Input database path
	// 2: Output database path
	// 3: Write intermediate shrub biomass (true/false)

	/*
	for (int i = 1; i < argc; i++)
	{
		
		switch (i)
		{
		case 1:
			#ifdef RVS_DB_PATH
			#undef RVS_DB_PATH
			#endif 
			break;
		case 2:
			#ifdef OUT_DB_PATH
			#undef OUT_DB_PATH
			#endif
			break;
		case 3:
			if (strcmp("false", argv[3]) == 0)
			{
				WRITE_SHRUB = false;
			}
			break;

		}
		
	}
	*/

	// Get DIO ready for queries
	Biomass::BiomassDIO* bdio = new Biomass::BiomassDIO();
	//RVS::Fuels::FuelsDIO* fdio = new RVS::Fuels::FuelsDIO();
    
    vector<int> plotcounts = bdio->query_analysis_plots();
	vector<AnalysisPlot*> aps = vector<AnalysisPlot*>();
	
	AnalysisPlot* currentPlot = NULL;
	RVS::DataManagement::DataTable* dt = NULL;
	for (int i = 0; i < plotcounts.size(); i++)
	{
		dt = bdio->query_input_table(plotcounts[i]);
		*RC = sqlite3_step(dt->getStmt());
		currentPlot = new AnalysisPlot(bdio, dt);
		aps.push_back(currentPlot);
	}

	Biomass::BiomassDriver bd = Biomass::BiomassDriver(bdio, Biomass::BiomassLookupLevel::medium, true);

	for (int year = 0; year < STOPYEAR; year++)
	{
		std::cout << "\n==================================" << std::endl;
		std::cout << "YEAR " << year << std::endl;
		std::cout << "===================================\n" << std::endl;

		for (int p = 0; p < plotcounts.size(); p++)
		{
			currentPlot = aps[p];

			if (!SUPPRESS_MSG)
			{
				std::cout << std::endl;
				std::cout << "Results for plot " << currentPlot->PLOT_ID() << std::endl;
				std::cout << "====================" << std::endl;
			}
			
			double biomass = 0;
			double shrubBiomass = 0;
			double herbBiomass = 0;

			RC = bd.BioMain(year, currentPlot, &shrubBiomass, &herbBiomass);

			if (!SUPPRESS_MSG)
			{
				std::cout << std::endl;
				std::cout << "Shrub Biomass: " << shrubBiomass << std::endl;
				std::cout << "Herb Biomass: " << herbBiomass << std::endl;
				std::cout << "Total Biomass: " << (shrubBiomass + herbBiomass) << std::endl;
			}

			double fuel1Hr = 0;
			double fuel10Hr = 0;
			double fuel100Hr = 0;

			if (!SUPPRESS_MSG)
			{
				std::cout << "1Hr Fuels:   " << fuel1Hr << std::endl;
				std::cout << "10Hr Fuels:  " << fuel10Hr << std::endl;
				std::cout << "100Hr Fuels: " << fuel100Hr << std::endl;
			}
		}
	}

	delete bdio;
	//delete fdio;

	std::cout << std::endl << "Ran to completion." << std::endl;

	return (0);
}
