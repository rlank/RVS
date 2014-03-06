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
	static int STOPYEAR = 10;  // make this C def
    
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
	RVS::Biomass::BiomassDIO dio;
	RVS::Fuels::FuelsDIO fdio;
    
    std::vector<int> plotcounts = RVS::DataManagement::DIO::query_analysis_plots();
    std::map<int, double> biomassResults;
	
	for (int year = 0; year < STOPYEAR; year++)
	{
		std::cout << "\n==================================" << std::endl;
		std::cout << "YEAR " << year << std::endl;
		std::cout << "===================================\n" << std::endl;

		// Toggle output writing
		if (year == STOPYEAR)
		{

		}

		////////////////////////////////////////////////////////////////////////////////
		// BIOMASS Loop //
		////////////////////////////////////////////////////////////////////////////////

		for (int plot_num = 0; plot_num < (int)(plotcounts.size() - 1); plot_num++)  // full plots
		//for (int plot_num = 0; plot_num < 1; plot_num++)  // single plot
		{
			if (!SUPPRESS_MSG)
			{
				std::cout << std::endl;
				std::cout << "Results for plot " << plot_num << std::endl;
				std::cout << "====================" << std::endl;
			}

			double biomass = 0;
			int runplot = plotcounts[plot_num];
			RVS::Biomass::BiomassReturnType retType = RVS::Biomass::BiomassReturnType::RNUL;
			RVS::Biomass::BiomassDriver bdriver = RVS::Biomass::BiomassDriver(RVS::Biomass::BiomassLookupLevel::medium, SUPPRESS_MSG, WRITE_SHRUB);

			RC = bdriver.BioMain(runplot, &biomass, &retType);
			biomassResults[runplot] = biomass;
		}

		////////////////////////////////////////////////////////////////////////////////
		// FUELS Loop    //
		////////////////////////////////////////////////////////////////////////////////

		*GROWTH_MULT = *GROWTH_MULT + 0.01f;  // increase growth amount
	}

	std::cout << std::endl << "Ran to completion." << std::endl;
	std::string endprogram; 
	//std::cin >> endprogram;

	return (0);
}
