/// ********************************************************** ///
/// Name: main.cpp                                             ///
/// Desc: (Test) driver for RVS. Build as execultable and      ///
/// this becomes the program entry point                       ///
/// Base Class(es): none                                       ///
/// Last Modified: 15 Jan 14                                   ///
/// ********************************************************** ///

#include <exception>
#include <iostream>
#include <string>
#include <vector>

//#include <boost/exception/all.hpp>

#include "RVSDEF.h"
#include "Biomass/BiomassDIO.h"
#include "Biomass/BiomassDriver.h"
#include "DataManagement/DIO.h"

int* RC = new int(SQLITE_OK);

int main(int argc, char* argv[])
{
    static bool SUPPRESS_MSG = false;
	static bool WRITE_SHRUB = false;

    
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

    ////////////////////////////////////////////////////////////////////////////////
    // This is a Biomass test, so build an array of BiomassEVT and run this thing //
    ////////////////////////////////////////////////////////////////////////////////

    std::vector<int> plotcounts = RVS::DataManagement::DIO::query_analysis_plots();
    std::map<int, double> biomassResults;

	for (int plot_num = 0; plot_num < (int)(plotcounts.size()-1); plot_num++)
	{
        double biomass = 0;
        int runplot = plotcounts[plot_num];
		RVS::Biomass::BiomassReturnType retType = RVS::Biomass::BiomassReturnType::RNUL;
		RVS::Biomass::BiomassDriver bdriver = RVS::Biomass::BiomassDriver(RVS::Biomass::BiomassLookupLevel::medium, SUPPRESS_MSG, WRITE_SHRUB);

		RC = bdriver.BioMain(runplot, &biomass, &retType);
		biomassResults[runplot] = biomass;
    }

	std::cout << std::endl << "Ran to completion." << std::endl;
	std::string endprogram; 
	//std::cin >> endprogram;

	return (0);
}
