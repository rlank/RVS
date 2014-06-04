#include "FuelsDriver.h"

RVS::Fuels::FuelsDriver::FuelsDriver(RVS::Fuels::FuelsDIO* fdio, bool suppress_messages, bool write_intermediate)
{
	this->fdio = fdio;
	this->suppress_messages = suppress_messages;
	this->write_intermediate = write_intermediate;
}

RVS::Fuels::FuelsDriver::~FuelsDriver()
{

}

int* RVS::Fuels::FuelsDriver::FuelsMain(int year, RVS::DataManagement::AnalysisPlot* ap)
{
	int evt_num = 0;
	std::string bps;
	std::string dom_spp;
	std::string spp_code;

	double totalFuel1Hr = 0;
	double totalFuel10Hr = 0;
	double totalFuel100Hr = 0;
	double fuel1Hr = 0;
	double fuel10Hr = 0;
	double fuel100Hr = 0;

	std::vector<SppRecord*>* shrubs = ap->SHRUB_RECORDS();
	SppRecord* current = NULL;
	for (int b = 0; b < shrubs->size(); b++)
	{
		current  = shrubs->at(b);
		if (b == 0)
		{
			evt_num = ap->EVT_NUM();
			bps = ap->BPS_NUM();
			dom_spp = current->DOM_SPP();
			spp_code = current->SPP_CODE();
		}

		fuel1Hr = RVS::Fuels::FuelsEquations::calc1Hr(current->SHRUBBIOMASS(), ap->HERBBIOMASS());
		fuel10Hr = RVS::Fuels::FuelsEquations::calc10Hr(current->SHRUBBIOMASS());
		fuel100Hr = RVS::Fuels::FuelsEquations::calc100Hr(current->SHRUBBIOMASS());
			
		//RC = fdio->write_fuel_intermediate_record(&plot_num, &year, &evt_num, &bps, &dom_spp, &spp_code, &fuel1Hr, &fuel10Hr, &fuel100Hr);

		totalFuel1Hr += fuel1Hr;
		totalFuel10Hr += fuel10Hr;
		totalFuel100Hr += fuel100Hr;
	}

	//RC = fdio->write_fuel_output_record(&plot_num, &year, &evt_num, &bps, &totalFuel1Hr, &totalFuel10Hr, &totalFuel100Hr);
	return RC;
}