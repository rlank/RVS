/// ********************************************************** ///
/// Name: main.cpp                                             ///
/// Desc: Driver for RVS. Build as execultable and			   ///
/// this becomes the program entry point                       ///
/// Base Class(es): none                                       ///
/// Last Modified: 5 Aug 15                                    ///
/// ********************************************************** ///

#include <ctime>
#include <fstream>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "RVSDEF.h"
#include "DataManagement/DIO.h"
#include "DataManagement/AnalysisPlot.h"
#include "DataManagement/RVSException.h"
#include "Biomass/BiomassDIO.h"
#include "Biomass/BiomassDriver.h"
#include "Biomass/BiomassEqDriver.h"
#include "Fuels/FuelsDIO.h"
#include "Fuels/FuelsDriver.h"
#include "Succession/SuccessionDIO.h"
#include "Succession/SuccessionDriver.h"

using namespace std;
using namespace RVS;
using namespace RVS::DataManagement;

int* RC = new int(SQLITE_OK);
int* YEARS = new int(10);
bool* SUPPRESS_MSG = new bool(true);
const char* DEBUG_FILE = "RVS_Debug.txt";
string* CLIMATE = new string("Normal");
bool* USE_MEM = new bool(true);

void simulate(int year, RVS::DataManagement::AnalysisPlot* currentPlot, 
	Biomass::BiomassDriver* bd, Fuels::FuelsDriver* fd, Succession::SuccessionDriver* sd);

int main(int argc, char* argv[])
{   
	time_t t = time(NULL);
	unique_ptr<ofstream> dfile (new ofstream(DEBUG_FILE, ios::out));
	*dfile << ctime(&t) << "\n";
	dfile->close();

	///////////////////////////
	/// User execution args ///
	///////////////////////////

	/// Get DIO ready for queries
	Biomass::BiomassDIO* bdio = new Biomass::BiomassDIO();
	Fuels::FuelsDIO* fdio = new Fuels::FuelsDIO();
	Succession::SuccessionDIO* sdio = new Succession::SuccessionDIO();
    
    vector<int> plotcounts = bdio->query_analysis_plots();
	map<int, AnalysisPlot*> aps;
	
	///////////////////////////////////////////////////////////////////////
	/// Load analysis plots and shrub records into a map keyed by plot id
	///////////////////////////////////////////////////////////////////////

	std::cout << SQLITE_VERSION << std::endl << SQLITE_SOURCE_ID << std::endl;
	std::cout << "Loading records..." << std::endl;
	AnalysisPlot* currentPlot = NULL;
	
	RVS::DataManagement::DataTable* plots_dt = bdio->query_input_table();

	while (*RC == SQLITE_ROW)
	{
		currentPlot = new AnalysisPlot(fdio, plots_dt);
		aps.insert(pair<int, AnalysisPlot*>(currentPlot->PLOT_ID(), currentPlot));
		*RC = sqlite3_step(plots_dt->getStmt());
	}

	RVS::DataManagement::DataTable* shrub_dt = bdio->query_shrubs_table();

	int plot_id = 0;
	while (*RC == SQLITE_ROW)
	{
		bdio->getVal(shrub_dt->getStmt(), shrub_dt->Columns[PLOT_NUM_FIELD], &plot_id);
		currentPlot = aps[plot_id];
		currentPlot->push_shrub(bdio, shrub_dt);
		*RC = sqlite3_step(shrub_dt->getStmt());
	}

	std::cout << "Done." << std::endl;

	///////////////////////////////
	/// Prepare for simulation
	///////////////////////////////

	Biomass::BiomassDriver bd = Biomass::BiomassDriver(bdio, *SUPPRESS_MSG);
	Fuels::FuelsDriver fd = Fuels::FuelsDriver(fdio, *SUPPRESS_MSG);
	Succession::SuccessionDriver sd = Succession::SuccessionDriver(sdio, *SUPPRESS_MSG);

	for (int year = 0; year < *YEARS; year++)
	{
		std::cout << "\n===================================" << std::endl;
		std::cout << "YEAR " << year << std::endl;
		std::cout << "===================================\n" << std::endl;

		for (int &p : plotcounts)
		{
			currentPlot = aps[p];
			simulate(year, currentPlot, &bd, &fd, &sd);
		}

		stringstream ss; 
		ss << "Year " << year << " finished";
		bdio->write_debug_msg(ss.str().c_str());
	}
	
	bdio->write_output();

	delete bdio;
	delete fdio;
	delete sdio;
	
	std::cout << std::endl << "Ran to completion." << std::endl;

	t = time(NULL);
	dfile = unique_ptr<ofstream>(new ofstream(DEBUG_FILE, ios::app));
	*dfile << ctime(&t) << "\n";
	dfile->close();

	return (*RC);
}

void simulate(int year, RVS::DataManagement::AnalysisPlot* currentPlot, 
	Biomass::BiomassDriver* bd, Fuels::FuelsDriver* fd, Succession::SuccessionDriver* sd)
{
	if (!*SUPPRESS_MSG)
	{
		std::cout << std::endl;
		std::cout << "Results for plot " << currentPlot->PLOT_ID() << std::endl;
		std::cout << "====================" << std::endl;
	}

	RC = bd->BioMain(year, CLIMATE, currentPlot);

	//RC = fd->FuelsMain(year, currentPlot);

	RC = sd->SuccessionMain(year, currentPlot);
}