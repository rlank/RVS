#include "FuelsDriver.h"

RVS::Fuels::FuelsDriver::FuelsDriver(RVS::Fuels::FuelsDIO* fdio, bool suppress_messages)
{
	this->fdio = fdio;
	this->suppress_messages = suppress_messages;
}

RVS::Fuels::FuelsDriver::~FuelsDriver()
{

}

int* RVS::Fuels::FuelsDriver::FuelsMain(int year, RVS::DataManagement::AnalysisPlot* ap)
{
	vector<RVS::DataManagement::SppRecord*>* shrubs = ap->SHRUB_RECORDS();

	ap->totalFuels.clear();  // Clear last year's fuels, if it exists

	int plot_num = ap->PLOT_ID();
	int evt_num = ap->EVT_NUM();
	int bps = ap->BPS_NUM();

	RVS::DataManagement::SppRecord* current = NULL;
	double totalShrubFuel = 0;
	for (int b = 0; b < shrubs->size(); b++)
	{
		current = shrubs->at(b);
		current->fuels.clear();  // Clear last year's fuels

		// Equation numbers for each fuels calculation
		map<string, int> equationNumbers;
		try
		{
			equationNumbers = fdio->query_crosswalk_table(current->SPP_CODE());
		}
		catch (RVS::DataManagement::DataNotFoundException &ex)
		{
			equationNumbers = fdio->query_crosswalk_table("ARTRT");
		}

		// Get percent live for each fuel class (1=1hr, 2=10hr, 3=100hr)
		//current->crl1 = calcPercentLive(current, 1);
		//current->crl2 = calcPercentLive(current, 2);
		//current->crl3 = calcPercentLive(current, 3);
		current->crl1 = 90;
		current->crl2 = 90;
		current->crl3 = 90;

		// Loop through the equations and calculate each fuel
		for (map<string, int>::iterator et = equationNumbers.begin(); et != equationNumbers.end(); et++)
		{
			int equationNumber = et->second;

			double fuel = calcShrubFuel(equationNumber, current);
			// Convert from grams to lbs
			fuel = fuel * ap->GRAMS_TO_POUNDS;

			if (std::isnan(fuel) || std::isinf(fuel) || fuel < 0)
			{
				fuel = -9999.0;
			}
			else  // only add to total fuels if the value isn't bogus
			{
				ap->totalFuels[et->first] += fuel;
			}

			current->fuels[et->first] = fuel;
			if (et->first.compare("FS1") == 0 || et->first.compare("FS2") == 0 || et->first.compare("FS3") == 0)
			{
				totalShrubFuel += fuel;
			}
			
		}

		string dom_spp = current->DOM_SPP();
		string spp_code = current->SPP_CODE();

		if (current->FUELS()["FL3"] < 0)
		{
			current->fuels["FL3"] = 0;
			current->fuels["FD3"] = 0;
		}

		// Write out the individual (shrub) record
		RC = fdio->write_intermediate_record(&year, ap, current);
	}

	// Save the total shrub fuel
	ap->shrubFuels = totalShrubFuel;
	// Get herbaceous fuels and add it to total 1 hr fuels
	ap->herbFuels = ap->herbBiomass + ap->herbHoldoverBiomass;
	ap->totalFuels["FS1"] += ap->herbFuels;

	string fuelClassTable = determineFBFMClassTable(ap);
	int fuelModel = calcFBFM(fuelClassTable, ap);

	ap->calcFBFM = fuelModel;

	// Write out the total fuels record
	RC = fdio->write_output_record(&year, ap);
	return RC;
}

double RVS::Fuels::FuelsDriver::calcPercentLive(RVS::DataManagement::SppRecord* spp, int fuelClass)
{
	vector<int> liveEqList = makePercentLiveList(fuelClass);
	vector<int> deadEqList = makePercentDeadList(fuelClass);

	// Calculate the sum of the live fuels
	double liveResults = 0.0;
	for (int e = 0; e < liveEqList.size(); e++)
	{
		liveResults += calcShrubFuel(liveEqList[e], spp);
	}
	// Take the average
	liveResults = liveResults / liveEqList.size();

	// Calculate the sum of the dead fuels
	double deadResults = 0.0;
	for (int e = 0; e < deadEqList.size(); e++)
	{
		deadResults += calcShrubFuel(deadEqList[e], spp);
	}
	// Take the average
	deadResults = deadResults / deadEqList.size();
	// Give the ratio of live to dead
	double ratio = deadResults / liveResults;

	// Cap the ratio's upper and lower bounds
	if (ratio > 1)	{ ratio = .9; }
	else if (ratio < 0) { ratio = .1; }

	return ratio * 100;
}

vector<int> RVS::Fuels::FuelsDriver::makePercentLiveList(int fuelClass)
{
	vector<int> eqList = vector<int>();
	switch (fuelClass)
	{
	case 1:
		eqList.push_back(6102);
		eqList.push_back(6087);
		break;
	case 2:
		eqList.push_back(6108);
		eqList.push_back(6095);
		eqList.push_back(6103);
		break;
	case 3:
		eqList.push_back(6089);
		eqList.push_back(6096);
		eqList.push_back(6109);
		break;
	}
	
	return eqList;
}

vector<int> RVS::Fuels::FuelsDriver::makePercentDeadList(int fuelClass)
{
	vector<int> eqList = vector<int>();
	switch (fuelClass)
	{
	case 1:
		eqList.push_back(6097);
		eqList.push_back(6111);
		break;
	case 2:
		eqList.push_back(6105);
		break;
	case 3:
		eqList.push_back(6092);
		eqList.push_back(6100);
		break;
	}

	return eqList;
}

double RVS::Fuels::FuelsDriver::calcShrubFuel(int equationNumber, RVS::DataManagement::SppRecord* spp)
{
	// Declare return objects
	string* paramNames = new string[3];
	double* coefs = new double[4];
	int* equationType = new int;
	// Populate the equation parameters
	fdio->query_equation_parameters(equationNumber, paramNames, coefs, equationType);
	// Get the values of the parameters from the shrub record
	double* params = new double[3];
	for (int i = 0; i < paramNames->size(); i++)
	{
		params[i] = spp->requestValue(paramNames[i]);
	}
	// Calculate fuels
	double fuel = RVS::Fuels::FuelsEquations::calcFuels(*equationType, coefs, params);
	fuel = fuel * spp->stemsPerAcre;
	return fuel;
}

std::string RVS::Fuels::FuelsDriver::determineFBFMClassTable(RVS::DataManagement::AnalysisPlot* ap)
{
	RVS::DataManagement::DataTable* dt = fdio->query_fbfm_rules_selector();
	*RC = sqlite3_step(dt->getStmt());

	std::string fuelClassTable;
	double shrubCovLo = 0;
	double shrubCovHi = 0;
	double herbProdLo = 0;
	double herbProdHi = 0;

	int shrubCovLoCol = dt->Columns[FC_SHRUB_COV_LOWER];
	int shrubCovHiCol = dt->Columns[FC_SHRUB_COV_UPPER];
	int herbProdLoCol = dt->Columns[FC_HERB_PROD_LOWER];
	int herbProdHiCol = dt->Columns[FC_HERB_PROD_UPPER];
	int fuelClassTableCol = dt->Columns[FC_RULE_TABLE_FIELD];

	bool pass = true;

	while (*RC == SQLITE_ROW)
	{
		fdio->getVal(dt->getStmt(), shrubCovLoCol, &shrubCovLo);
		fdio->getVal(dt->getStmt(), shrubCovHiCol, &shrubCovHi);
		fdio->getVal(dt->getStmt(), herbProdLoCol, &herbProdLo);
		fdio->getVal(dt->getStmt(), herbProdHiCol, &herbProdHi);

		if (shrubCovLo != -1)
		{
			if (shrubCovLo > ap->shrubCover)
			{
				pass = false;
			}
		}
		if (shrubCovHi != -1)
		{
			if (shrubCovHi < ap->shrubCover)
			{
				pass = false;
			}
		}
		if (herbProdLo != -1)
		{
			if (herbProdLo > ap->herbBiomass)
			{
				pass = false;
			}
		}
		if (herbProdHi != -1)
		{
			if (herbProdHi < ap->herbBiomass)
			{
				pass = false;
			}
		}

		if (pass == true)
		{
			fdio->getVal(dt->getStmt(), fuelClassTableCol, &fuelClassTable);
		}

		*RC = sqlite3_step(dt->getStmt());
		pass = true;
	}
	*RC = sqlite3_reset(dt->getStmt());
	return fuelClassTable;
}

int RVS::Fuels::FuelsDriver::calcFBFM(std::string classTable, RVS::DataManagement::AnalysisPlot* ap)
{
	int ret = 0;

	if ((int)classTable.find("Grass") > 0)
	{
		ret = calcFBFMGrass(classTable, ap);
	}
	else if ((int)classTable.find("Shrub") > 0)
	{
		ret = calcFBFMShrub(classTable, ap);
	}
	else if ((int)classTable.find("Mixed") > 0)
	{
		ret = calcFBFMMixed(classTable, ap);
	}

	return ret;
}

int RVS::Fuels::FuelsDriver::calcFBFMGrass(std::string classTable, RVS::DataManagement::AnalysisPlot* ap)
{
	RVS::DataManagement::DataTable* dt = fdio->query_fbfm_rules(classTable);
	*RC = sqlite3_step(dt->getStmt());

	double herbBiomass = ap->herbBiomass;
	double lowerBound = 0;
	double upperBound = 0;
	bool pass = false;
	while (*RC == SQLITE_ROW && pass == false)
	{
		fdio->getVal(dt->getStmt(), dt->Columns[FC_HERB_1HR_LOWER], &lowerBound);
		fdio->getVal(dt->getStmt(), dt->Columns[FC_HERB_1HR_UPPER], &upperBound);

		if (herbBiomass > lowerBound && herbBiomass < upperBound)
		{
			pass = true;
		}
		else
		{
			*RC = sqlite3_step(dt->getStmt());
		}
	}

	*RC = sqlite3_reset(dt->getStmt());

	int fbfm = switchClimateFBFM(dt, ap);
	return fbfm;
}

int RVS::Fuels::FuelsDriver::calcFBFMShrub(std::string classTable, RVS::DataManagement::AnalysisPlot* ap)
{
	RVS::DataManagement::DataTable* dt = fdio->query_fbfm_rules(classTable);
	*RC = sqlite3_step(dt->getStmt());

	double htLower = 0;
	double htUpper = 0;
	double shrubFuelLower = 0;
	double shrubFuelUpper = 0;

	int fbfm = 0;
	bool pass = true;
	while (*RC == SQLITE_ROW)
	{
		fdio->getVal(dt->getStmt(), dt->Columns[FC_HT_LOWER], &htLower);
		fdio->getVal(dt->getStmt(), dt->Columns[FC_HT_UPPER], &htUpper);
		fdio->getVal(dt->getStmt(), dt->Columns[FC_SHRUB_FUEL_LOWER], &shrubFuelLower);
		fdio->getVal(dt->getStmt(), dt->Columns[FC_SHRUB_FUEL_UPPER], &shrubFuelUpper);

		if (htLower != -1)
		{
			if (ap->shrubHeight < htLower)
			{
				pass = false;
			}
		}
		if (htUpper != -1)
		{
			if (ap->shrubHeight > htUpper)
			{
				pass = false;
			}
		}
		if (shrubFuelLower != -1)
		{
			if (ap->shrubFuels < shrubFuelLower)
			{
				pass = false;
			}
		}
		if (shrubFuelUpper != -1)
		{
			if (ap->shrubFuels > shrubFuelUpper)
			{
				pass = false;
			}
		}

		if (pass == true)
		{
			fbfm = switchClimateFBFM(dt, ap);
			break;
		}

		pass = true;
		*RC = sqlite3_step(dt->getStmt());
	}
	*RC = sqlite3_reset(dt->getStmt());
	return fbfm;
}

int RVS::Fuels::FuelsDriver::calcFBFMMixed(std::string classTable, RVS::DataManagement::AnalysisPlot* ap)
{
	RVS::DataManagement::DataTable* dt = fdio->query_fbfm_rules(classTable);
	*RC = sqlite3_step(dt->getStmt());

	int fbfm = 0;
	double proportion = ap->shrubBiomass / ap->herbBiomass;
	double totalFuels = ap->herbFuels + ap->shrubFuels;

	double proportionLower = 0;
	double proportionUpper = 0;
	double totalFuelsLower = 0;
	double totalFuelsUpper = 0;

	bool pass = true;
	while (*RC == SQLITE_ROW)
	{
		fdio->getVal(dt->getStmt(), dt->Columns[FC_PROPORTION_LOWER], &proportionLower);
		fdio->getVal(dt->getStmt(), dt->Columns[FC_PROPORTION_UPPER], &proportionUpper);
		fdio->getVal(dt->getStmt(), dt->Columns[FC_TOTAL_FUEL_LOWER], &totalFuelsLower);
		fdio->getVal(dt->getStmt(), dt->Columns[FC_TOTAL_FUEL_UPPER], &totalFuelsUpper);

		if (proportionLower != -1)
		{
			if (proportion < proportionLower)
			{
				pass = false;
			}
		}
		if (proportionUpper != -1)
		{
			if (proportion > proportionUpper)
			{
				pass = false;
			}
		}
		if (totalFuelsLower != -1)
		{
			if (totalFuels < totalFuelsLower)
			{
				pass = false;
			}
		}
		if (totalFuelsUpper != -1)
		{
			if (totalFuels > totalFuelsUpper)
			{
				pass = false;
			}
		}

		if (pass == true)
		{
			fbfm = switchClimateFBFM(dt, ap);
			break;
		}

		pass = true;
		*RC = sqlite3_step(dt->getStmt());
	}
	*RC = sqlite3_reset(dt->getStmt());
	return fbfm;
}

int RVS::Fuels::FuelsDriver::switchClimateFBFM(RVS::DataManagement::DataTable* dt, RVS::DataManagement::AnalysisPlot* ap)
{
	int fbfm = 0;
	if (ap->dryClimate)
	{
		fdio->getVal(dt->getStmt(), dt->Columns[FC_FBFM_DRY_FIELD], &fbfm);
	}
	else
	{
		fdio->getVal(dt->getStmt(), dt->Columns[FC_FBFM_HUMID_FIELD], &fbfm);
	}
	return fbfm;
}