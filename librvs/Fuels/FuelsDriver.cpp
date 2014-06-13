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

	if (plot_num == 7)
	{
		int iii = 0;
	}

	RVS::DataManagement::SppRecord* current = NULL;
	for (int b = 0; b < shrubs->size(); b++)
	{
		current = shrubs->at(b);
		current->fuels.clear();  // Clear last year's fuels

		// Equation numbers for each fuels calculation
		map<string, int> equationNumbers = fdio->query_crosswalk_table(current->SPP_CODE());  

		// Get percent live for each fuel class (1=1hr, 2=10hr, 3=100hr)
		current->crl1 = calcPercentLive(current, 1);
		current->crl2 = calcPercentLive(current, 2);
		current->crl3 = calcPercentLive(current, 3);

		// Loop through the equations and calculate each fuel
		for (map<string, int>::iterator et = equationNumbers.begin(); et != equationNumbers.end(); et++)
		{
			int equationNumber = et->second;
			double fuel = calcShrubFuel(equationNumber, current);

			if (isnan<double>(fuel) || isinf<double>(fuel) || fuel < 0)
			{
				fuel = -9999.0;
			}
			else  // only add to total fuels if the value isn't bogus
			{
				ap->totalFuels[et->first] += fuel;
			}

			current->fuels[et->first] = fuel;  // Always add individual fuel. Useful for debugging
		}

		string dom_spp = current->DOM_SPP();
		string spp_code = current->SPP_CODE();

		RC = fdio->write_fuel_intermediate_record(&plot_num, &year, &evt_num, &bps, &dom_spp, &spp_code, current->fuels);
	}

	RC = fdio->write_fuel_output_record(&plot_num, &year, &evt_num, &bps, ap->totalFuels);
	return RC;
}

double RVS::Fuels::FuelsDriver::calcPercentLive(RVS::DataManagement::SppRecord* spp, int fuelClass)
{
	vector<int> liveEqList = makePercentLiveList(fuelClass);
	vector<int> deadEqList = makePercentDeadList(fuelClass);

	double liveResults = 0.0;
	for (int e = 0; e < liveEqList.size(); e++)
	{
		liveResults += calcShrubFuel(liveEqList[e], spp);
	}

	liveResults = liveResults / liveEqList.size();

	double deadResults = 0.0;
	for (int e = 0; e < deadEqList.size(); e++)
	{
		deadResults += calcShrubFuel(deadEqList[e], spp);
	}

	deadResults = deadResults / deadEqList.size();

	double ratio = liveResults / deadResults;

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
		eqList.push_back(6079);
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
	string* paramNames = new string[3];
	double* coefs = new double[4];
	int* equationType = new int;

	fdio->query_equation_parameters(equationNumber, paramNames, coefs, equationType);

	double* params = new double[3];
	for (int i = 0; i < paramNames->size(); i++)
	{
		params[i] = spp->requestValue(paramNames[i]);
	}

	double fuel = RVS::Fuels::FuelsEquations::calcFuels(*equationType, coefs, params);
	fuel = fuel * spp->stemsPerAcre;
	return fuel;
}