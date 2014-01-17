#include "BiomassDriver.h"

using RVS::Biomass::BiomassDriver;


BiomassDriver::BiomassDriver(RVS::Biomass::BiomassLookupLevel level, bool suppress_messages, bool write_intermediate)
{
	this->level = level;
	this->suppress_messages = suppress_messages;
	this->write_intermediate = write_intermediate;

	// Create the output database table
	//RC = RVS::Biomass::BiomassDIO::create_biomass_output_table();
}


BiomassDriver::~BiomassDriver(void)
{
}

int* BiomassDriver::BioMain(int plot_num, double* biomass_return_value, RVS::Biomass::BiomassReturnType* biomass_return_type)
{
	try
	{
		if (!suppress_messages)
		{
			std::cout << std::endl;
			std::cout << "Results for plot " << plot_num << std::endl;
			std::cout << "====================" << std::endl;
		}

		// declare local return vars
		double BIOMASS_RETURN_VAL = 0;
		RVS::Biomass::BiomassReturnType BIOMASS_RET_TYPE = RVS::Biomass::RNUL;

		double production = 0;
		double herbBiomass = 0;
		double shrubBiomass = 0;

		// read in data. the query_biomass_input_table function returns a DataTable
		// with only records for that plot number
		RVS::DataManagement::DataTable* dt = RVS::Biomass::BiomassDIO::query_biomass_input_table(plot_num);

		*RC = sqlite3_step(dt->getStmt());
		RVS::Biomass::BiomassEVT* bioEVT = NULL;
		int row_counter = 0;
		while (*RC == SQLITE_ROW)
		{
			//$$ TODO are these evts getting deleted? Why am I making this?
			bioEVT = new RVS::Biomass::BiomassEVT(dt);
			evt_records.push_back(bioEVT);
			
			if (row_counter == 0)
			{
				production = this->calcHerbBiomass(bioEVT);
			}
			
			double singleBiomass = 0;

			// Branch biomass calculation to do either HERB or SHRUB
			switch (bioEVT->LIFEFORM())
			{
				case RVS::DataManagement::shrub:
					singleBiomass = this->calcShrubBiomass(bioEVT);
					break;
				case RVS::DataManagement::herb:
					singleBiomass = this->calcHerbBiomass(bioEVT);
					break;
				default:
					std::cout << "Lifeform not implemented." << std::endl;
					break;
			}

			// Write this result to the intermediate table
			if (write_intermediate)
			{
				RC = RVS::Biomass::BiomassDIO::write_biomass_intermediate_record(bioEVT, &plot_num, &singleBiomass);
			}

			shrubBiomass += singleBiomass;
			row_counter++;
			*RC = sqlite3_step(dt->getStmt());
		}

		int evt = bioEVT->EVT_NUM();
		std::string bps = (char*)bioEVT->BPS_NUM().c_str();
		char* ret_code = ENUMPARSE(bioEVT->RETURN_TYPE());

		herbBiomass = production;
		BIOMASS_RETURN_VAL = shrubBiomass + herbBiomass;
		BIOMASS_RET_TYPE = RVS::Biomass::PCH;

		delete bioEVT;
		delete dt;

		// Write output biomass record
		RC = RVS::Biomass::BiomassDIO::write_biomass_output_record(&plot_num, &evt, &bps, ret_code, &BIOMASS_RETURN_VAL, &herbBiomass, &shrubBiomass);

		if (!suppress_messages)
		{
			std::cout << std::endl;
			std::cout << "Primary Production: " << production << std::endl;
			std::cout << "Shrub Biomass: " << shrubBiomass << std::endl;
			std::cout << "Herb Biomass: " << herbBiomass << std::endl;
			std::cout << "Total Biomass: " << BIOMASS_RETURN_VAL << std::endl;
			std::cout << "Units: " << BIOMASS_RET_TYPE << std::endl;
			std::cout << "Return State: " << *RC << std::endl;
		}

		

	}
	catch (std::exception& e)
	{
		std::cout << "Exception in BioMain: " << e.what() << std::endl;
	}

	return RC;
}

double BiomassDriver::calcHerbBiomass(RVS::Biomass::BiomassEVT* evt)
{
	// Strip the leading characters off the BPS (they're location)
	std::string bps = evt->BPS_NUM().substr(2);
	int baseBPS = std::stoi(bps);

	// Get a text representation of the return level
	char* clevel;
    switch (this->level)
    {
		case RVS::Biomass::low:
			clevel = "low_val";
            break;
        case RVS::Biomass::medium:
			clevel = "med_val";
            break;
        case RVS::Biomass::high:
			clevel = "high_val";
            break;
		default:
			clevel = "med_val";
			break;
    }

	double biomass = RVS::Biomass::BiomassDIO::query_biomass_herbs_table(baseBPS, clevel);
    return biomass;
};

double BiomassDriver::calcShrubBiomass(RVS::Biomass::BiomassEVT* evt)
{
    double biomass = 0;

	// Query the biomass crosswalk table for the equation for the requested return type
    std::string returnType = RVS::Biomass::ENUMPARSE(evt->RETURN_TYPE());
	int equationNumber = RVS::Biomass::BiomassDIO::query_biomass_crosswalk_table(evt->SPP_CODE(), returnType);

	float cf1 = 0;
	float cf2 = 0;
	float cf3 = 0;
	float cf4 = 0;

	// Populate the coefficients with values from the biomass equation table
	RVS::Biomass::BiomassDIO::query_biomass_equation_coefficients(equationNumber, &cf1, &cf2, &cf3, &cf4);

    double height = evt->HEIGHT();

	//$$ TODO set up "get shrub biomass" to return different types of (requested) biomass
    double pch = RVS::Biomass::BiomassEquations::eq_PCH(cf1, cf2, height);
	double pch_acre = RVS::Biomass::BiomassEquations::eq_pch_acre(pch);
            
    biomass = pch_acre * (evt->COVER() / 100);

    return biomass;
};
