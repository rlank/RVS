/// ********************************************************** ///
/// Name: main.cpp                                             ///
/// Desc: (Test) driver for RVS. Build as execultable and      ///
/// this becomes the program entry point                       ///
/// Base Class(es): none                                       ///
/// Last Modified: 8 Jan 14                                    ///
/// ********************************************************** ///

#include <exception>
#include <iostream>
#include <string>
#include <vector>

//#include <boost/exception/all.hpp>

#include <RVSDEF.h>
#include <Biomass/BiomassDriver.h>
#include <DataManagement/DIO.h>

int main(int argc, char* argv[])
{
	static int RETURN_STATE = 0;
    static bool USE_THREADING = false;
    static bool SUPPRESS_MSG = false;

	char* defaultdb = "C:\\MCR\\RVS\\Data\\rvs_bio.db";

    std::string consoleRead = "";
    /*
	std::cout << "Enter path to RVS database or leave blank to use default: (" << defaultdb << ") ";
    std::cin >> RVS::DataManagement::DIO::pathToDb;
    
	///////////////////////////
	/// Do user options here //
	///////////////////////////

	*/
    std::cout << std::endl;

	// Get DIO ready for queries
	RVS::DataManagement::DIO dio(defaultdb);

    ////////////////////////////////////////////////////////////////////////////////
    // This is a Biomass test, so build an array of BiomassEVT and run this thing //
    ////////////////////////////////////////////////////////////////////////////////

    std::vector<int> plotcounts = RVS::DataManagement::DIO::query_analysis_plots();
    std::map<int, double> biomassResults;

	for (int plot_num = 0; plot_num < plotcounts.size() - 1; plot_num++)
	{
		printf("plot num: %i : %i\n", plot_num, plotcounts[plot_num]);

        double biomass = 0;
        int runplot = plotcounts[plot_num];
		RVS::Biomass::BiomassReturnType retType = RVS::Biomass::BiomassReturnType::RNUL;
        RVS::Biomass::BiomassDriver bdriver = RVS::Biomass::BiomassDriver(RVS::Biomass::BiomassLookupLevel::medium, SUPPRESS_MSG);

		//RETURN_STATE = bdriver.BioMain(runplot, &biomass, &retType);
		biomassResults[runplot] = biomass;
    }

	std::cout << std::endl << "Ran to completion." << std::endl;
	std::string endprogram; 
	std::cin >> endprogram;
	return (0);
}
