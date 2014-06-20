/// ********************************************************** ///
/// Name: main.cpp                                             ///
/// Desc: (Test) driver for RVS. Build as execultable and      ///
/// this becomes the program entry point                       ///
/// Base Class(es): none                                       ///
/// Last Modified: 6 Jun 14                                    ///
/// ********************************************************** ///

#include <ctime>
#include <fstream>
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
#include "Fuels/FuelsDriver.h"

using namespace std;
using namespace RVS;
using namespace RVS::DataManagement;

int* RC = new int(SQLITE_OK);
const char* DEBUG_FILE = "RVS_Debug.txt";

int main(int argc, char* argv[])
{
    static bool SUPPRESS_MSG = false;
	static int STOPYEAR = 1;  
    
	time_t t = time(NULL);
	ofstream dfile = ofstream(DEBUG_FILE, ios::out);
	dfile << ctime(&t) << "\n";
	dfile.close();


	///////////////////////////
	/// User execution args ///
	///////////////////////////

	// Args:
	// 1: Input database path
	// 2: Output database path
	

	/// Get DIO ready for queries
	Biomass::BiomassDIO* bdio = new Biomass::BiomassDIO();
	Fuels::FuelsDIO* fdio = new Fuels::FuelsDIO();
    
    vector<int> plotcounts = bdio->query_analysis_plots();
	vector<AnalysisPlot*> aps = vector<AnalysisPlot*>();
	
	AnalysisPlot* currentPlot = NULL;
	RVS::DataManagement::DataTable* dt = NULL;
	for (int i = 0; i < plotcounts.size(); i++)
	{
		dt = bdio->query_input_table(plotcounts[i]);
		*RC = sqlite3_step(dt->getStmt());
		currentPlot = new AnalysisPlot(fdio, dt);
		aps.push_back(currentPlot);
		delete dt;
	}

	Biomass::BiomassDriver bd = Biomass::BiomassDriver(bdio, SUPPRESS_MSG);
	Fuels::FuelsDriver fd = Fuels::FuelsDriver(fdio, SUPPRESS_MSG);

	for (int year = 0; year < STOPYEAR; year++)
	{
		std::cout << "\n===================================" << std::endl;
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

			RC = fd.FuelsMain(year, currentPlot);

			if (!SUPPRESS_MSG)
			{
				std::map<std::string, double> fuels = currentPlot->TOTALFUELS();
				for (std::map<std::string, double>::iterator it = fuels.begin(); it != fuels.end(); it++)
				{
					std::cout << it->first << ": " << it->second << std::endl;
				}
			}
		}
	}

	delete bdio;
	delete fdio;
	
	std::cout << std::endl << "Ran to completion." << std::endl;

	return (*RC);
}
