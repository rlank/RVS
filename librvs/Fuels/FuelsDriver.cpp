#include "FuelsDriver.h"

RVS::Fuels::FuelsDriver::FuelsDriver(int plot_num, RVS::Fuels::FuelsDIO* fdio, bool suppress_messages, bool write_intermediate)
{
	this->plot_num = plot_num;
	this->fdio = fdio;
	this->suppress_messages = suppress_messages;
	this->write_intermediate = write_intermediate;
}

RVS::Fuels::FuelsDriver::~FuelsDriver()
{

}

int* RVS::Fuels::FuelsDriver::FuelsMain(RVS::Biomass::BiomassEVT* bioEVT, int* plot_num, double* shrubBiomass, double* herbBiomass)
{
	try
	{
		double fuel1Hr = RVS::Fuels::FuelsEquations::calc1Hr(*shrubBiomass, *herbBiomass);
		double fuel10Hr = RVS::Fuels::FuelsEquations::calc10Hr(*shrubBiomass);
		double fuel100Hr = RVS::Fuels::FuelsEquations::calc100Hr(*shrubBiomass);

		int evt_num = bioEVT->EVT_NUM();
		std::string bps = bioEVT->BPS_NUM();

		//RC = fdio->write_fuel_output_record(plot_num, &evt_num, &bps, &fuel1Hr, &fuel10Hr, &fuel100Hr);
		return RC;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception in FuelsMain: " << e.what() << std::endl;
		return RC;
	}
}

int* RVS::Fuels::FuelsDriver::FuelsMain(int year, std::vector<RVS::Biomass::BiomassEVT*> bioEVTs)
{
	try
	{
		RVS::Biomass::BiomassEVT* bioEVT = NULL;

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

		for (int b = 0; b < bioEVTs.size(); b++)
		{
			bioEVT = bioEVTs.at(b);
			if (b == 0)
			{
				evt_num = bioEVT->EVT_NUM();
				bps = bioEVT->BPS_NUM();
				dom_spp = bioEVT->DOM_SPP();
				spp_code = bioEVT->SPP_CODE();
			}

			fuel1Hr = RVS::Fuels::FuelsEquations::calc1Hr(bioEVT->SHRUBBIOMASS(), bioEVT->HERBBIOMASS());
			fuel10Hr = RVS::Fuels::FuelsEquations::calc10Hr(bioEVT->SHRUBBIOMASS());
			fuel100Hr = RVS::Fuels::FuelsEquations::calc100Hr(bioEVT->SHRUBBIOMASS());
			
			RC = fdio->write_fuel_intermediate_record(&plot_num, &year, &evt_num, &bps, &dom_spp, &spp_code, &fuel1Hr, &fuel10Hr, &fuel100Hr);

			totalFuel1Hr += fuel1Hr;
			totalFuel10Hr += fuel10Hr;
			totalFuel100Hr += fuel100Hr;
		}

		RC = fdio->write_fuel_output_record(&plot_num, &year, &evt_num, &bps, &totalFuel1Hr, &totalFuel10Hr, &totalFuel100Hr);
		return RC;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception in FuelsMain: " << e.what() << std::endl;
		return RC;
	}
}