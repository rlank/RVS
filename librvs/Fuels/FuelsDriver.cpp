#include "FuelsDriver.h"

RVS::Fuels::FuelsDriver::FuelsDriver(bool suppress_messages, bool write_intermediate)
{
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

		if (!suppress_messages)
		{
			std::cout << "1Hr Fuels:   " << fuel1Hr << std::endl;
			std::cout << "10Hr Fuels:  " << fuel10Hr << std::endl;
			std::cout << "100Hr Fuels: " << fuel100Hr << std::endl;
		}

		int evt_num = bioEVT->EVT_NUM();
		std::string bps = bioEVT->BPS_NUM();

		RC = RVS::Fuels::FuelsDIO::write_fuel_output_record(plot_num, &evt_num, &bps, &fuel1Hr, &fuel10Hr, &fuel100Hr);
		return RC;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception in FuelsMain: " << e.what() << std::endl;
	}
}