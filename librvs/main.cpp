/// ********************************************************** ///
/// Name: main.cpp                                             ///
/// Desc: Driver for RVS. Build as executable and              ///
///       this becomes the program entry point                 ///
/// Base Class(es): none                                       ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "RVSDEF.h"
#include "RvsCoreBridge.h"
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
#include "Disturbance/DisturbanceDIO.h"
#include "Disturbance/DisturbanceDriver.h"

using namespace std;
using namespace RVS;
using namespace RVS::DataManagement;

// Modernized static storage variables eliminating naked `new` global allocations.
static int g_rc = SQLITE_OK;
int* RC = &g_rc;

static int g_years = 20;
int* YEARS = &g_years;

static bool g_suppress_msg = true;
bool* SUPPRESS_MSG = &g_suppress_msg;

const char* DEBUG_FILE = "RVS_Debug.txt";

static std::string g_climate = "Normal";
string* CLIMATE = &g_climate;

static bool g_use_mem = true;
bool* USE_MEM = &g_use_mem;

static bool g_random_climate = false;
bool* RANDOM_CLIMATE = &g_random_climate;

char* RVS_DB_PATH = const_cast<char*>("rvs_demo.db");
char* OUT_DB_PATH = const_cast<char*>("rvs_out_demo.db");

// Define runmode:
// 1: normal sim
// 2: single year FCCS shrub test
// 3: 5 year herb test
// 4: shrub equation test
static const int g_runmode = 1;
const int* runmode = &g_runmode;

void simulate(int year, RVS::DataManagement::AnalysisPlot* currentPlot, 
	Biomass::BiomassDriver* bd, 
	Fuels::FuelsDriver* fd, 
	Succession::SuccessionDriver* sd, 
	Disturbance::DisturbanceDriver* dd);

void fiveYearHerbTest(int year, RVS::DataManagement::AnalysisPlot* currentPlot,
	Biomass::BiomassDriver* bd,
	Fuels::FuelsDriver* fd,
	Succession::SuccessionDriver* sd,
	Disturbance::DisturbanceDriver* dd);

void shrubEquationTest();

void run(
	void(*simFunc)(int year, RVS::DataManagement::AnalysisPlot* currentPlot,
		Biomass::BiomassDriver* bd,
		Fuels::FuelsDriver* fd,
		Succession::SuccessionDriver* sd,
		Disturbance::DisturbanceDriver* dd));

void randomClimate();

int main(int argc, char* argv[])
{   
	for (int i = 0; i < argc; i++)
	{
		std::cout << argv[i] << std::endl;
	}

	if (argc >= 4)
	{
		RVS_DB_PATH = argv[1];
		OUT_DB_PATH = argv[2];
		*YEARS = atoi(argv[3]);
	}
	else
	{
		if (std::filesystem::exists("rvs_demo.db"))
		{
			RVS_DB_PATH = const_cast<char*>("rvs_demo.db");
		}
		else if (std::filesystem::exists("../rvs_demo.db"))
		{
			RVS_DB_PATH = const_cast<char*>("../rvs_demo.db");
		}
		OUT_DB_PATH = const_cast<char*>("rvs_out_demo.db");
	}

	run(&simulate);

	return (*RC);
}

void run(void(*simFunc)(int year, RVS::DataManagement::AnalysisPlot* currentPlot,
	Biomass::BiomassDriver* bd,
	Fuels::FuelsDriver* fd,
	Succession::SuccessionDriver* sd,
	Disturbance::DisturbanceDriver* dd))
{
	time_t t = time(NULL);
	auto dfile = std::make_unique<ofstream>(DEBUG_FILE, ios::out);
	*dfile << ctime(&t) << "\n";
	dfile->close();

	///////////////////////////
	/// User execution args ///
	///////////////////////////

	/// Get DIO ready for queries with RAII
	auto bdio = std::make_unique<Biomass::BiomassDIO>();
	auto fdio = std::make_unique<Fuels::FuelsDIO>();
	auto sdio = std::make_unique<Succession::SuccessionDIO>();
	auto ddio = std::make_unique<Disturbance::DisturbanceDIO>();

	vector<int> plotcounts = bdio->query_analysis_plots();
	map<int, std::unique_ptr<AnalysisPlot>> aps;

	bdio->write_debug_msg("Starting simulation");

	std::cout << SQLITE_VERSION << std::endl << SQLITE_SOURCE_ID << std::endl;
	std::cout << "Loading records..." << std::endl;
	AnalysisPlot* currentPlot = nullptr;

	RVS::DataManagement::DataTable* plots_dt = bdio->query_input_table();

	while (*RC == SQLITE_ROW)
	{
		auto plot = std::make_unique<AnalysisPlot>(fdio.get(), plots_dt);
		int pid = plot->PLOT_ID();
		aps.emplace(pid, std::move(plot));
		*RC = sqlite3_step(plots_dt->getStmt());
	}

	bdio->write_debug_msg("Plots loaded");

	RVS::DataManagement::DataTable* shrub_dt = bdio->query_shrubs_table();

	int plot_id = 0;
	while (*RC == SQLITE_ROW)
	{
		bdio->getVal(shrub_dt->getStmt(), shrub_dt->Columns[PLOT_NUM_FIELD], &plot_id);
		if (aps.count(plot_id))
		{
			currentPlot = aps[plot_id].get();
			currentPlot->push_shrub(bdio.get(), shrub_dt);
		}
		*RC = sqlite3_step(shrub_dt->getStmt());
	}

	for (auto &p : plotcounts)
	{
		if (aps.count(p))
		{
			currentPlot = aps[p].get();
			currentPlot->update_shrubvalues();
		}
	}

	map<int, vector<RVS::Disturbance::DisturbAction>> disturbances = ddio->query_disturbance_input();

	for (auto &d : disturbances)
	{
		if (aps.count(d.first))
		{
			aps[d.first]->setDisturbances(d.second);
		}
	}

	bdio->write_debug_msg("Plants loaded");
	std::cout << "Done." << std::endl;

	Biomass::BiomassDriver bd(bdio.get(), *SUPPRESS_MSG);
	Fuels::FuelsDriver fd(fdio.get(), *SUPPRESS_MSG);
	Succession::SuccessionDriver sd(sdio.get(), *SUPPRESS_MSG);
	Disturbance::DisturbanceDriver dd(ddio.get(), *SUPPRESS_MSG);

	for (int year = 0; year < *YEARS; year++)
	{
		std::cout << "\n===================================" << std::endl;
		std::cout << "YEAR " << year << std::endl;
		std::cout << "===================================\n" << std::endl;

		for (int &p : plotcounts)
		{
			if (aps.count(p))
			{
				currentPlot = aps[p].get();
				simFunc(year, currentPlot, &bd, &fd, &sd, &dd);
			}
		}

		stringstream ss;
		ss << "Year " << year << " finished";
		bdio->write_debug_msg(ss.str().c_str());
	}

	bdio->write_output();

	// RAII cleanup: aps container automatically frees all AnalysisPlot instances
	aps.clear();

	std::cout << std::endl << "Ran to completion." << std::endl;

	t = time(NULL);
	dfile = std::make_unique<ofstream>(DEBUG_FILE, ios::app);
	*dfile << ctime(&t) << "\n";
	dfile->close();
}

void simulate(int year, RVS::DataManagement::AnalysisPlot* currentPlot, 
	Biomass::BiomassDriver* bd, 
	Fuels::FuelsDriver* fd, 
	Succession::SuccessionDriver* sd, 
	Disturbance::DisturbanceDriver* dd)
{
	if (!*SUPPRESS_MSG)
	{
		std::cout << std::endl;
		std::cout << "Results for plot " << currentPlot->PLOT_ID() << std::endl;
		std::cout << "====================" << std::endl;
	}

	if (*RANDOM_CLIMATE) { randomClimate(); }

	*RC = sd->SuccessionMain(year, CLIMATE ? *CLIMATE : "", currentPlot);
	*RC = dd->DisturbanceMain(year, currentPlot);
	*RC = bd->BioMain(year, CLIMATE ? *CLIMATE : "", currentPlot);
	*RC = fd->FuelsMain(year, currentPlot);
}

void fiveYearHerbTest(int year, RVS::DataManagement::AnalysisPlot* currentPlot,
	Biomass::BiomassDriver* bd,
	Fuels::FuelsDriver* fd,
	Succession::SuccessionDriver* sd,
	Disturbance::DisturbanceDriver* dd)
{
	if (year == 0) { *CLIMATE = "Dry"; }
	else if (year == 1) { *CLIMATE = "Mid-Dry"; }
	else if (year == 2) { *CLIMATE = "Normal"; }
	else if (year == 3) { *CLIMATE = "Mid-Wet"; }
	else if (year == 4) { *CLIMATE = "Wet"; }

	currentPlot->HERB_RESET_TEST_ONLY();
	*RC = sd->SuccessionMain(year, CLIMATE ? *CLIMATE : "", currentPlot);
	*RC = bd->BioMain(year, CLIMATE ? *CLIMATE : "", currentPlot);
	*RC = fd->FuelsMain(year, currentPlot);
}

void shrubEquationTest()
{
	time_t t = time(NULL);
	auto dfile = std::make_unique<ofstream>(DEBUG_FILE, ios::out);
	*dfile << ctime(&t) << "\n";
	dfile->close();

	///////////////////////////
	/// User execution args ///
	///////////////////////////

	/// Get DIO ready for queries
	auto bdio = std::make_unique<Biomass::BiomassDIO>();
	auto fdio = std::make_unique<Fuels::FuelsDIO>();
	auto sdio = std::make_unique<Succession::SuccessionDIO>();
	auto ddio = std::make_unique<Disturbance::DisturbanceDIO>();

	vector<int> plotcounts = bdio->query_analysis_plots();
	map<int, std::unique_ptr<AnalysisPlot>> aps;

	///////////////////////////////////////////////////////////////////////
	/// Load analysis plots and shrub records into a map keyed by plot id
	///////////////////////////////////////////////////////////////////////

	std::cout << SQLITE_VERSION << std::endl << SQLITE_SOURCE_ID << std::endl;
	std::cout << "Loading records..." << std::endl;
	AnalysisPlot* currentPlot = nullptr;

	RVS::DataManagement::DataTable* plots_dt = bdio->query_input_table();

	while (*RC == SQLITE_ROW)
	{
		auto plot = std::make_unique<AnalysisPlot>(fdio.get(), plots_dt);
		int pid = plot->PLOT_ID();
		aps.emplace(pid, std::move(plot));
		*RC = sqlite3_step(plots_dt->getStmt());
	}

	RVS::DataManagement::DataTable* shrub_dt = bdio->query_shrubs_table();

	int plot_id = 0;
	while (*RC == SQLITE_ROW)
	{
		bdio->getVal(shrub_dt->getStmt(), shrub_dt->Columns[PLOT_NUM_FIELD], &plot_id);
		if (aps.count(plot_id))
		{
			currentPlot = aps[plot_id].get();
			currentPlot->push_shrub(bdio.get(), shrub_dt);
		}
		*RC = sqlite3_step(shrub_dt->getStmt());
	}

	map<int, vector<RVS::Disturbance::DisturbAction>> disturbances = ddio->query_disturbance_input();

	for (auto &d : disturbances)
	{
		if (aps.count(d.first))
		{
			aps[d.first]->setDisturbances(d.second);
		}
	}

	std::cout << "Done." << std::endl;

	Biomass::BiomassEqDriver beqd(bdio.get(), *SUPPRESS_MSG);
	vector<int> testEquations;
	ifstream equationsFile;
	equationsFile.open("librvs/biomass_text_equations.txt");
	string line;
	if (equationsFile.is_open())
	{
		while (getline(equationsFile, line))
		{
			if (!line.empty() && line[0] != '#')
			{
				line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
				line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
				int eqNum = std::stoi(line);
				testEquations.push_back(eqNum);
			}
		}
		equationsFile.close();
	}

	for (auto it = testEquations.begin(); it != testEquations.end(); ++it)
	{
		std::cout << "\n===================================" << std::endl;
		std::cout << "EQUATION " << *it << std::endl;
		std::cout << "===================================\n" << std::endl;
		for (int &p : plotcounts)
		{
			if (aps.count(p))
			{
				currentPlot = aps[p].get();
				int* res = beqd.BioMain(*it, currentPlot);
				if (res)
				{
					*RC = *res;
				}
			}
		}
		stringstream ss;
		ss << "EQUATION " << *it << " finished";
		bdio->write_debug_msg(ss.str().c_str());
	}

	bdio->write_output();

	// RAII cleanup: aps container automatically frees all AnalysisPlot instances
	aps.clear();

	std::cout << std::endl << "Ran to completion." << std::endl;

	t = time(NULL);
	dfile = std::make_unique<ofstream>(DEBUG_FILE, ios::app);
	*dfile << ctime(&t) << "\n";
	dfile->close();
}

void randomClimate()
{
	int i = rand() % 5;
	switch (i)
	{
	case 0:
		*CLIMATE = "Dry";
		break;
	case 1:
		*CLIMATE = "Mid-Dry";
		break;
	case 2:
		*CLIMATE = "Normal";
		break;
	case 3:
		*CLIMATE = "Mid-Wet";
		break;
	case 4:
		*CLIMATE = "Wet";
		break;
	}
}
