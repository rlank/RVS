/// ********************************************************** ///
/// Name: main.cpp                                             ///
/// Desc: (Test) driver for RVS. Build as execultable and      ///
/// this becomes the program entry point                       ///
/// Base Class(es): none                                       ///
/// Last Modified: 5 Mar 14                                    ///
/// ********************************************************** ///

#include <exception>
#include <iostream>
#include <string>
#include <vector>

//#include <boost/exception/all.hpp>

#include "RVSDEF.h"
#include "DataManagement/DIO.h"
#include "Biomass/BiomassDIO.h"
#include "Biomass/BiomassDriver.h"
#include "Fuels/FuelsDIO.h"

int* RC = new int(SQLITE_OK);
float* GROWTH_MULT = new float(0.0f);

int main(int argc, char* argv[])
{
    static bool SUPPRESS_MSG = false;
	static bool WRITE_SHRUB = true;
	static int STOPYEAR = 1;  //$$ TODO make this C def
    
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
	RVS::Biomass::BiomassDIO* bdio = new RVS::Biomass::BiomassDIO();
	RVS::Fuels::FuelsDIO* fdio = new RVS::Fuels::FuelsDIO();
    
    std::vector<int> plotcounts = bdio->query_analysis_plots();
	std::map<int, RVS::Biomass::BiomassDriver*> bioDrivers;

	for (int year = 0; year < STOPYEAR; year++)
	{
		std::cout << "\n==================================" << std::endl;
		std::cout << "YEAR " << year << std::endl;
		std::cout << "===================================\n" << std::endl;

		for (int plot_num = 0; plot_num < (int)(plotcounts.size() - 1); plot_num++)  // full plots
		//for (int plot_num = 0; plot_num < 1; plot_num++)  // single plot
		{
			if (!SUPPRESS_MSG)
			{
				std::cout << std::endl;
				std::cout << "Results for plot " << plot_num << std::endl;
				std::cout << "====================" << std::endl;
			}

			int runplot = plotcounts[plot_num];
			if (year == 0)
			{
				bioDrivers[plot_num] = new RVS::Biomass::BiomassDriver(runplot, bdio, RVS::Biomass::BiomassLookupLevel::medium, SUPPRESS_MSG, WRITE_SHRUB);
			}

			double biomass = 0;
			double shrubBiomass = 0;
			double herbBiomass = 0;

			RC = bioDrivers[plot_num]->BioMain(year, &shrubBiomass, &herbBiomass);

			//if (!SUPPRESS_MSG)
			//{
			//	std::cout << std::endl;
			//	std::cout << "Primary Production: " << herbBiomass << std::endl;
			//	std::cout << "Shrub Biomass: " << shrubBiomass << std::endl;
			//	std::cout << "Herb Biomass: " << herbBiomass << std::endl;
			//	std::cout << "Total Biomass: " << (shrubBiomass + herbBiomass) << std::endl;
			//}

			//double fuel1Hr = 0;
			//double fuel10Hr = 0;
			//double fuel100Hr = 0;

			//RVS::Fuels::FuelsDriver fd(plot_num, fdio, SUPPRESS_MSG, WRITE_SHRUB);
			//RC = fd.FuelsMain(year, bioDrivers[plot_num]->pullRecords());

			//if (!SUPPRESS_MSG)
			//{
			//	std::cout << "1Hr Fuels:   " << fuel1Hr << std::endl;
			//	std::cout << "10Hr Fuels:  " << fuel10Hr << std::endl;
			//	std::cout << "100Hr Fuels: " << fuel100Hr << std::endl;
			//}
		}

		*GROWTH_MULT = *GROWTH_MULT + 0.01f;  // increase growth amount
	}

	bioDrivers.clear();
	delete bdio;
	delete fdio;

	std::cout << std::endl << "Ran to completion." << std::endl;
	std::string endprogram; 

	return (0);
}
