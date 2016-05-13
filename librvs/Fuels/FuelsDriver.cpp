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
	this->ap = ap;

	vector<RVS::DataManagement::SppRecord*>* shrubs = ap->SHRUB_RECORDS();

	int plot_num = ap->PLOT_ID();
	int evt_num = ap->EVT_NUM();
	int bps = ap->BPS_NUM();

	/*
	double totalShrubFuel = 0;
	for (auto &s : *shrubs)
	{
		SppRecord* current = s;

		// Equation numbers for each fuels calculation
		map<string, int> equationNumbers;

		equationNumbers = fdio->query_crosswalk_table(current->SPP_CODE());

		if (equationNumbers.size() == 0)
		{
			equationNumbers = fdio->query_crosswalk_table(FUELS_BACKUP_SPP_CODE);
		}

		// Loop through the equations and calculate each fuel
		for (map<string, int>::iterator et = equationNumbers.begin(); et != equationNumbers.end(); et++)
		{
			int equationNumber = et->second;

			
		}


		// Write out the individual (shrub) record
		RC = fdio->write_intermediate_record(&year, ap, current);
	}
	*/

	
	
	double shrub1HourWB = calc1HrWoodBark(ap->shrubBiomass);
	double shrub1HourFoliage = calc1HrFoliage(ap->shrubBiomass);
	double shrub10Hour = calc10HrFuel(ap->shrubBiomass);
	double shrub100Hour = calc100HrFuel(ap->shrubBiomass);
	double shrub1000Hour = calc1000HrFuel(ap->shrubHeight);
	
	double bio = ap->shrubBiomass;

	double herbFuel = ap->primaryProduction * ap->POUNDS_TO_GRAMS;
	double fuelTot = shrub1HourWB + shrub1HourFoliage + shrub10Hour + shrub100Hour + shrub1000Hour;

	//double propTot = fuelTot / bio * 100;
	double prop1HrWB = shrub1HourWB / fuelTot;
	double prop1HrFol = shrub1HourFoliage / fuelTot;
	double prop10Hr = shrub10Hour / fuelTot;
	double prop100Hr = shrub100Hour / fuelTot;
	double prop1000Hr = shrub1000Hour / fuelTot;

	double propTotal = prop1HrWB + prop1HrFol + prop10Hr + prop100Hr + prop1000Hr;  // just checking

	ap->herbFuel = herbFuel;
	ap->shrub1HourWB = bio * prop1HrWB;
	ap->shrub1HourFoliage = bio * prop1HrFol;
	ap->shrub10Hour = bio * prop10Hr;
	ap->shrub100Hour = bio * prop100Hr;
	ap->shrub1000Hour = bio * shrub1000Hour;

	ap->total1HrFuel = ap->herbFuel + ap->shrub1HourWB + ap->shrub1HourFoliage;

	if (ap->disturbed)
	{
		//applyDisturbance();
	}


	//string fuelClassTable = determineFBFMClassTable(ap);
	//int fuelModel = calcFBFM(fuelClassTable, ap);
	//ap->calcFBFM = fuelModel;

	if (ap->ISDRY())
	{
		ap->fbfmName = calcFBFMDry();
	}
	else
	{
		ap->fbfmName = calcFBFMHumid();
	}

	// Write out the total fuels record
	RC = fdio->write_output_record(&year, ap);
	return RC;
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

string RVS::Fuels::FuelsDriver::calcFBFMDry()
{
	double shrubRelativeCover = ap->SHRUBCOVER() / (ap->SHRUBCOVER() + ap->HERBCOVER());
	double herbRelativeCover = ap->HERBCOVER() / (ap->SHRUBCOVER() + ap->HERBCOVER());
	string FBFM = "";

	// First determine stand type
	// SHRUB TYPE
	if (shrubRelativeCover >= .9) 
	{
		if (ap->FUEL_TOTAL_1HR() <= 1500 && ap->FUEL_TOTAL() <= 4000) { FBFM = "SH1"; }
		else if (ap->FUEL_TOTAL_1HR() >= 1500 && ap->FUEL_TOTAL_1HR() < 3000) {	FBFM = "SH2"; }
		else if (ap->FUEL_TOTAL_1HR() >= 3000 && ap->FUEL_TOTAL() < 20000 && ap->SHRUBHEIGHT() > 91.44) { FBFM = "SH5";	}
		else if (ap->FUEL_TOTAL_1HR() >= 3000 && ap->FUEL_TOTAL() >= 20000 && ap->SHRUBHEIGHT() > 91.44) { FBFM = "SH7"; }
		else { FBFM = "Unk1"; }
	}
	// MIXED TYPE
	else if (shrubRelativeCover >= .1 && ap->SHRUBHEIGHT() >= 30.48 && ap->HERB_FUEL()*ap->GRAMS_TO_POUNDS >= 200)  
	{
		if (ap->HERB_FUEL() < 1500 && ap->FUEL_TOTAL() < 4000) { FBFM = "GS1"; }
		else if (ap->HERB_FUEL() >= 1500 && ap->FUEL_TOTAL() >= 4000) {	FBFM = "GS2"; }
		else { FBFM = "Unk2"; }
	}
	// GRASS TYPE
	else if (herbRelativeCover >= .9 && ap->HERBCOVER() >= .1)
	{
		if (ap->HERB_FUEL() <= 1200 && ap->HERBHEIGHT() <= 30.48) { FBFM = "GR1"; }
		else if (ap->HERB_FUEL() > 1200 && ap->HERB_FUEL() <= 3000 && ap->HERBHEIGHT() < 60.96) { FBFM = "GR2"; }
		else if (ap->HERB_FUEL() > 1200 && ap->HERB_FUEL() <= 3000 && ap->HERBHEIGHT() >= 60.96) { FBFM = "GR3"; }
		else if (ap->HERB_FUEL() > 3000 && ap->HERB_FUEL() <= 6000) { FBFM = "GR4"; }
		else if (ap->HERB_FUEL() > 6000) { FBFM = "GR7"; }
		else { FBFM = "Unk3"; }
	}
	if (FBFM.compare("") == 0)
	{
		FBFM = "Unk7";
	}
	return FBFM;
}

string RVS::Fuels::FuelsDriver::calcFBFMHumid()
{
	double shrubRelativeCover = ap->SHRUBCOVER() / (ap->SHRUBCOVER() + ap->HERBCOVER());
	double herbRelativeCover = ap->HERBCOVER() / (ap->SHRUBCOVER() + ap->HERBCOVER());
	string FBFM = "";

	// First determine stand type
	// SHRUB TYPE
	if (shrubRelativeCover >= .9)
	{
		if (ap->FUEL_TOTAL() >= 15000 && ap->FUEL_TOTAL() <= 20000 && ap->SHRUBHEIGHT() >= 60.96) { FBFM = "SH3"; }
		else if (ap->FUEL_TOTAL() < 15000 && ap->SHRUBHEIGHT() >= 60.96) { FBFM = "SH4"; }
		else if (ap->FUEL_TOTAL() >= 8000 && ap->FUEL_TOTAL() <= 15000 && ap->SHRUBHEIGHT() < 45.72) { FBFM = "SH6"; }
		else if (ap->FUEL_TOTAL() >= 20000 && ap->FUEL_TOTAL() <= 25000 && ap->SHRUBHEIGHT() > 60.96) { FBFM = "SH8"; }
		else if (ap->FUEL_TOTAL() >= 25000 && ap->SHRUBHEIGHT() > 106.68) { FBFM = "SH8"; }
		else { FBFM = "Unk4"; }
	}
	// MIXED TYPE
	else if (shrubRelativeCover >= .1 && ap->SHRUBHEIGHT() >= 30.48 && ap->HERB_FUEL()*ap->GRAMS_TO_POUNDS >= 200)
	{
		if (ap->HERB_FUEL() > 500 && ap->FUEL_TOTAL() >= 2500 && ap->FUEL_TOTAL() < 7000) { FBFM = "GS3"; }
		else if (ap->HERB_FUEL() > 4000 && ap->FUEL_TOTAL() >= 7000) { FBFM = "GS4"; }
		else { FBFM = "Unk5"; }
	}
	// GRASS TYPE
	else if (herbRelativeCover >= .9 && ap->HERBCOVER() >= .1)
	{
		if (ap->HERB_FUEL() <= 4500) { FBFM = "GR3"; }
		else if (ap->HERB_FUEL() > 4500 && ap->FUEL_TOTAL() < 6500) { FBFM = "GR5"; }
		else if (ap->HERB_FUEL() >= 6500 && ap->FUEL_TOTAL() < 12000) { FBFM = "GR6"; }
		else if (ap->HERB_FUEL() >= 12000 && ap->FUEL_TOTAL() < 17000) { FBFM = "GR8"; }
		else if (ap->HERB_FUEL() > 17000) { FBFM = "GR9"; }
		else { FBFM = "Unk6"; }
	}
	if (FBFM.compare("") == 0)
	{
		FBFM = "Unk8";
	}
	return FBFM;
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
	/*
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
	*/
	return fbfm;
}

int RVS::Fuels::FuelsDriver::calcFBFMMixed(std::string classTable, RVS::DataManagement::AnalysisPlot* ap)
{
	int fbfm = 0;
	/*
	RVS::DataManagement::DataTable* dt = fdio->query_fbfm_rules(classTable);
	*RC = sqlite3_step(dt->getStmt());

	
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
	*/
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

double RVS::Fuels::FuelsDriver::calc1HrFuel(double biomass)
{
	double fs1 = calc1HrWoodBark(biomass);
	double bft = calc1HrFoliage(biomass);
	return fs1 + bft;
}

double RVS::Fuels::FuelsDriver::calc1HrWoodBark(double biomass)
{
	double fs1 = 2.932237 + 0.353554 * biomass;
	return fs1;
}

double RVS::Fuels::FuelsDriver::calc1HrFoliage(double biomass)
{
	double bft = -1.498408 + 0.955207 * log(biomass);
	bft = exp(bft);
	return bft;
}

double RVS::Fuels::FuelsDriver::calc10HrFuel(double biomass)
{
	double fs2 = 0;
	if (ap->shrubAvgStem > 22.6796)
	{
		fs2 = -5.192477 + 0.445884 * biomass;
	}
	else
	{
		fs2 = 0.012477 + 0.445884 * biomass;
	}
	return fs2;
}

double RVS::Fuels::FuelsDriver::calc100HrFuel(double biomass)
{
	double fs3 = -3.552846 + 1.005534 * log(biomass);
	fs3 = exp(fs3);
	return fs3;
}

double RVS::Fuels::FuelsDriver::calc1000HrFuel(double ht)
{
	double fs4 = 0;
	
	if (ht > 400)
	{
		// Height needs to be converted to METERS, output is in KG
		ht = ht * 100;  // convert to cm
		fs4 = log10(ht) * 3.6853 - 1.306;
		fs4 /= 100;  // convert to grams
	}
	
	return fs4;
}

void RVS::Fuels::FuelsDriver::applyDisturbance()
{
	ap->shrub1HourFoliage -= ap->disturbed;
	if (ap->shrub1HourFoliage < 0)
	{
		ap->herbFuels += ap->shrub1HourFoliage;
		ap->shrub1HourFoliage = 0;
	}
	if (ap->herbFuels < 0)
	{
		ap->shrub1HourWB += ap->herbFuels;
		ap->herbFuels = 0;
	}
	if (ap->shrub1HourWB < 0)
	{
		ap->shrub10Hour += ap->shrub1HourWB;
		ap->shrub1HourWB = 0;
	}
}