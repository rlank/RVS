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
	for (int b = 0; b < shrubs->size(); b++)
	{
		current = shrubs->at(b);
		current->fuels.clear();  // Clear last year's fuels

		// Equation numbers for each fuels calculation
		map<string, int> equationNumbers = fdio->query_crosswalk_table(current->SPP_CODE());  

		// Datatable from FUEL_EQUATION_TABLE with each equation from the crosswalk
		RVS::DataManagement::DataTable* dt = fdio->query_equation_table(equationNumbers);  
		*RC = sqlite3_step(dt->getStmt());
		
		while (*RC == SQLITE_ROW)
		{
			std::string fuelsReturnType;
			fdio->getVal(dt->getStmt(), dt->Columns[RET_CODE_FIELD], &fuelsReturnType);
			double fuel = calcShrubFuel(dt, current);
			if (isnan<double>(fuel) || isinf<double>(fuel))
			{
				fuel = -9999.0;
			}
			else  // only add to total fuels if the value isn't bogus
			{
				ap->totalFuels[fuelsReturnType];
			}

			current->fuels[fuelsReturnType] = fuel;  // Always add individual fuel. Useful for debugging
			
			*RC = sqlite3_step(dt->getStmt());  // Advance cursor
		}

		delete dt;

		string dom_spp = current->DOM_SPP();
		string spp_code = current->SPP_CODE();

		RC = fdio->write_fuel_intermediate_record(&plot_num, &year, &evt_num, &bps, &dom_spp, &spp_code, current->fuels);
	}

	RC = fdio->write_fuel_output_record(&plot_num, &year, &evt_num, &bps, ap->totalFuels);
	return RC;
}

double RVS::Fuels::FuelsDriver::calcPercentLive()
{
	return 0;
}

double RVS::Fuels::FuelsDriver::calcShrubFuel(RVS::DataManagement::DataTable* dt, RVS::DataManagement::SppRecord* spp)
{
	int equationType;
	fdio->getVal(dt->getStmt(), dt->Columns[EQUATION_TYPE_FIELD], &equationType);
	
	std::string* paramNames = new std::string[3];
	fdio->getVal(dt->getStmt(), dt->Columns[EQN_P1_FIELD], &paramNames[0]);
	fdio->getVal(dt->getStmt(), dt->Columns[EQN_P2_FIELD], &paramNames[1]);
	fdio->getVal(dt->getStmt(), dt->Columns[EQN_P3_FIELD], &paramNames[2]);

	double* params = new double[3];
	for (int i = 0; i < paramNames->size(); i++)
	{
		params[i] = spp->requestValue(paramNames[i]);
	}

	double* coefs = new double[4];
	fdio->getVal(dt->getStmt(), dt->Columns[EQN_COEF_1_FIELD], &coefs[0]);
	fdio->getVal(dt->getStmt(), dt->Columns[EQN_COEF_2_FIELD], &coefs[1]);
	fdio->getVal(dt->getStmt(), dt->Columns[EQN_COEF_3_FIELD], &coefs[2]);
	fdio->getVal(dt->getStmt(), dt->Columns[EQN_COEF_4_FIELD], &coefs[3]);


	double fuel = RVS::Fuels::FuelsEquations::calcFuels(equationType, coefs, params);
	fuel = fuel * spp->STEMSPERACRE();

	delete[] paramNames;
	delete params;

	return fuel;
}