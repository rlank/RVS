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
	
	double shrub1HourWB = calc1HrWoodBark(ap->shrubBiomass);
	double shrub1HourFoliage = calc1HrFoliage(ap->shrubBiomass);
	double shrub10Hour = calc10HrFuel(ap->shrubBiomass);
	double shrub100Hour = calc100HrFuel(ap->shrubBiomass);
	double shrub1000Hour = calc1000HrFuel(ap->shrubHeight);
	
	double bio = ap->shrubBiomass;

	double herbFuel = ap->herbBiomass * ap->POUNDS_TO_GRAMS;
	double fuelTot = shrub1HourWB + shrub1HourFoliage + shrub10Hour + shrub100Hour + shrub1000Hour;

	double prop1HrWB = shrub1HourWB / fuelTot;
	double prop1HrFol = shrub1HourFoliage / fuelTot;
	double prop10Hr = shrub10Hour / fuelTot;
	double prop100Hr = shrub100Hour / fuelTot;
	double prop1000Hr = shrub1000Hour / fuelTot;

	//double propTotal = prop1HrWB + prop1HrFol + prop10Hr + prop100Hr + prop1000Hr;  // just checking

	ap->herbFuel = herbFuel;
	ap->shrub1HourWB = bio * prop1HrWB;
	ap->shrub1HourFoliage = bio * prop1HrFol;
	ap->shrub10Hour = bio * prop10Hr;
	ap->shrub100Hour = bio * prop100Hr;
	ap->shrub1000Hour = bio * shrub1000Hour;

	ap->total1HrFuel = ap->herbFuel + ap->shrub1HourWB + ap->shrub1HourFoliage;

	if (ap->disturbed)
	{
		applyDisturbance(year);
	}

	if (ap->FUEL_TOTAL() < 200)
	{
		ap->fbfmName = "NB";
	}
	else
	{
		if (ap->ISDRY()) { ap->fbfmName = calcFBFMDry(); }
		else { ap->fbfmName = calcFBFMHumid(); }
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

string RVS::Fuels::FuelsDriver::calcFBFMDry()
{
	double shrubRelativeFuel = (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB()) / ap->FUEL_TOTAL_1HR();
	double herbRelativeFuel = ap->HERB_FUEL() / ap->FUEL_TOTAL_1HR();

	string FBFM = "";

	// First determine stand type
	// SHRUB TYPE
	if (shrubRelativeFuel >= .8)
	{
		if (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB() <= 1500) { FBFM = "SH1"; }
		else if (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB() < 5000) { FBFM = "SH2"; }
		else if (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB() + ap->SHRUB_10HR() + ap->SHRUB_100HR() <= 12000) { FBFM = "SH5"; }
		else if (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB() + ap->SHRUB_10HR() + ap->SHRUB_100HR() > 12000) { FBFM = "SH7"; }
		else { FBFM = "Unk1"; }
	}
	// GRASS TYPE
	else if (herbRelativeFuel >= .8)
	{
		if (ap->HERB_FUEL() <= 1000) { FBFM = "GR1"; }
		else if (ap->HERB_FUEL() <= 2500) { FBFM = "GR2"; }
		else if (ap->HERB_FUEL() <= 7500) { FBFM = "GR4"; }
		else if (ap->HERB_FUEL() > 7500) { FBFM = "GR7"; }
		else { FBFM = "Unk3"; }
	}
	// MIXED TYPE
	else
	{
		//if (ap->FUEL_TOTAL() < 3000) { FBFM = "GS1"; }
		//else if (ap->FUEL_TOTAL() > 3000) {	FBFM = "GS2"; }
		if (ap->HERB_FUEL() <= 1000) { FBFM = "GS1"; }
		else if (ap->HERB_FUEL() > 1000) { FBFM = "GS2"; }
		else { FBFM = "Unk2"; }
	}
	
	if (FBFM.compare("") == 0)
	{
		FBFM = "Unk7";
	}

	return FBFM;
}

string RVS::Fuels::FuelsDriver::calcFBFMHumid()
{
	double shrubRelativeFuel = (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB()) / ap->FUEL_TOTAL_1HR();
	double herbRelativeFuel = ap->HERB_FUEL() / ap->FUEL_TOTAL_1HR();

	string FBFM = "";

	// First determine stand type
	// SHRUB TYPE
	if (shrubRelativeFuel >= .8)
	{
		if (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB() <= 1000) { FBFM = "SH3"; }
		else if (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB() + ap->SHRUB_10HR() + ap->SHRUB_100HR() < 5000) { FBFM = "SH4"; }
		else if (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB() + ap->SHRUB_10HR() + ap->SHRUB_100HR() <= 9000) { FBFM = "SH6"; }
		else if (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB() + ap->SHRUB_10HR() + ap->SHRUB_100HR() <= 5000) { FBFM = "SH8"; }
		else if (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB() > 5000) { FBFM = "SH9"; }
		else { FBFM = "Unk1"; }
	}
	// GRASS TYPE
	else if (herbRelativeFuel >= .8)
	{
		if (ap->HERB_FUEL() <= 4500) { FBFM = "GR3"; }
		else if (ap->HERB_FUEL() <= 6000) { FBFM = "GR5"; }
		else if (ap->HERB_FUEL() <= 8000) { FBFM = "GR6"; }
		else if (ap->HERB_FUEL() <= 20000) { FBFM = "GR8"; }
		else if (ap->HERB_FUEL() > 20000) { FBFM = "GR9"; }
		else { FBFM = "Unk3"; }
	}
	// MIXED TYPE
	else
	{
		if (ap->FUEL_TOTAL() < 5500) { FBFM = "GS3"; }
		else if (ap->FUEL_TOTAL() > 3000) { FBFM = "GS4"; }
		else { FBFM = "Unk2"; }
	}

	
	if (FBFM.compare("") == 0)
	{
		FBFM = "Unk8";
	}
	return FBFM;
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

void RVS::Fuels::FuelsDriver::applyDisturbance(int year)
{
	vector<RVS::Disturbance::DisturbAction> disturbances = ap->getDisturbancesForYear(year);
	if (disturbances.empty()) { return; }

	// Fire is handled in the DisturbanceDriver, so skip if fire
	// $TODO this assumes fire is the only action (or first action) for the year
	if (disturbances[0].getActionType().compare("FIRE") == 0) { return; }

	// If it's a graze, go ahead and set up reduction ratios and priorities
	double herbPreference = 0;
	double shrubPreference = 0;

	double totalCover = ap->HERBCOVER() + ap->SHRUBCOVER();
	double herbAbundance = ap->HERBCOVER() / totalCover * 100;
	double shrubAbundance = ap->SHRUBCOVER() / totalCover * 100;

	// $TODO make the preferences dynamic
	for (auto &d : disturbances)
	{
		if (d.getActionSubType().compare("COW") == 0)
		{
			herbPreference = 99;
			shrubPreference = 1;
		}
		else
		{
			herbPreference = 50;
			shrubPreference = 50;
		}

		double oldHerbBiomass = ap->herbFuel;
		double oldShrubBiomass = ap->shrubBiomass;

		double herbRatio = herbPreference * herbAbundance;
		double shrubRatio = shrubPreference * shrubAbundance;
		double herbsPerShrub = herbRatio / shrubRatio;

		if (herbsPerShrub < 1)	{ herbsPerShrub = 1; }
		double herbReduction = (1 - 1 / herbsPerShrub) * ap->biomassReductionTotal;
		ap->herbFuel = ap->herbFuel - herbReduction;

		double shrubBiomassReduction = (1 / herbsPerShrub) * ap->biomassReductionTotal;
		if (ap->herbFuel < 0)
		{
			shrubBiomassReduction += ap->herbFuel * -1;
			ap->herbFuel = 0.1;
		}
		ap->shrub1HourFoliage -= shrubBiomassReduction;
		if (ap->shrub1HourFoliage < 0)
		{
			ap->shrub1HourWB += ap->shrub1HourFoliage;
			ap->shrub1HourFoliage = 0.1;
		}
		if (ap->shrub1HourWB < 0)
		{
			ap->shrub10Hour += ap->shrub1HourWB;
			ap->shrub1HourWB = 0.1;
		}

		if (ap->herbCover > 0)
		{
			double herbReductionRatio = ap->herbFuel / oldHerbBiomass;
			ap->herbCover = ap->herbCover * herbReductionRatio;
			if (ap->herbCover < 0) { ap->herbCover = 0.1; }
			ap->herbHeight = ap->herbHeight * herbReductionRatio;
			if (ap->herbHeight < 0) { ap->herbHeight = 0.1; }
		}

		double newShrubBiomass = ap->shrub1HourFoliage + ap->shrub1HourWB + ap->shrub10Hour + ap->shrub100Hour + ap->shrub1000Hour;
		double shrubReductionRatio = newShrubBiomass / oldShrubBiomass;
		double numShrubs = (double)ap->shrubRecords.size();
		for (auto &s : ap->shrubRecords)
		{
			s->cover = s->cover * shrubReductionRatio;
			if (s->cover < 0) { s->cover = 0.1; }
		}

		ap->herbBiomass = ap->herbFuel * ap->GRAMS_TO_POUNDS;
		ap->shrubBiomass = newShrubBiomass;
		ap->total1HrFuel = ap->herbFuel + ap->shrub1HourWB + ap->shrub1HourFoliage;
	}
}