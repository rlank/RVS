#include "BiomassDriver.h"

using RVS::Biomass::BiomassDriver;


BiomassDriver::BiomassDriver(RVS::Biomass::BiomassLookupLevel level, bool suppress_messages)
{
	this->level = level;
	this->suppress_messages = suppress_messages;
}


BiomassDriver::~BiomassDriver(void)
{
}

int BiomassDriver::BioMain(int plot_num, double* biomass_return_value, RVS::Biomass::BiomassReturnType* biomass_return_type)
{
	int RETURN_STATE = 0;
	try
	{
		if (!suppress_messages)
		{
			std::cout << std::endl;
			std::cout << "Results for plot " << plot_num << std::endl;
			std::cout << "====================" << std::endl;
		}

		// declare local return vars (these will be printed to console)
		double BIOMASS_RETURN_VAL = 0;
		RVS::Biomass::BiomassReturnType BIOMASS_RET_TYPE = RVS::Biomass::RNUL;

		double primary_production = 0;
		double herbBiomass = 0;
		double shrubBiomass = 0;

		// read in data. the query_biomass_input_table function returns a DataTable
		// with only records for that plot number
		RVS::DataManagement::DataTable* dt = RVS::Biomass::BiomassDIO::query_biomass_input_table(plot_num);

		int rc = sqlite3_step(dt->getStmt());
		int row_counter = 0;
		while (rc == SQLITE_ROW)
		{
			//$$ TODO are these evts getting deleted?
			RVS::Biomass::BiomassEVT* bioEVT = new RVS::Biomass::BiomassEVT(dt);
			evt_records.push_back(bioEVT);
			
			if (row_counter == 0)
			{
				primary_production = this->calcHerbBiomass(bioEVT);
			}
			
			double singleBiomass = 0;

			// Branch biomass calculation to do either HERB or SHRUB
			switch (bioEVT->LIFEFORM())
			{
				case RVS::DataManagement::shrub:
					singleBiomass = this->calcShrubBiomass(bioEVT);
					//evt_records[row_counter].RESULTS.Add("biomass", (float)singleBiomass);
					break;
				case RVS::DataManagement::herb:
					//singleBiomass = this.calcHerbBiomass(ref evt_records[row_counter]);
					break;
				default:
					std::cout << "Lifeform not implemented." << std::endl;
					break;
			}

			shrubBiomass += singleBiomass;
			row_counter++;
			rc = sqlite3_step(dt->getStmt());
		}

		delete dt;

		herbBiomass = primary_production - shrubBiomass;
		BIOMASS_RETURN_VAL = shrubBiomass + herbBiomass;
		BIOMASS_RET_TYPE = RVS::Biomass::PCH;


		// report biomass for plot. for now this means dump to console. later, this will write to output database
		if (!suppress_messages)
		{
			std::cout << std::endl;
			std::cout << "Primary Production: " << primary_production << std::endl;
			std::cout << "Shrub Biomass: " << shrubBiomass << std::endl;
			std::cout << "Herb Biomass: " << herbBiomass << std::endl;
			std::cout << "Total Biomass: " << BIOMASS_RETURN_VAL << std::endl;
			std::cout << "Units: " << BIOMASS_RET_TYPE << std::endl;
			std::cout << "Return State: " << RETURN_STATE << std::endl;
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Exception in BioMain: " << e.what() << std::endl;
		RETURN_STATE = 100;
	}

	return RETURN_STATE;
}

double BiomassDriver::calcHerbBiomass(RVS::Biomass::BiomassEVT* evt)
{
	std::string bps = evt->BPS_NUM().substr(2);
	int baseBPS = std::stoi(bps);

	//$$ TODO move the query work to the new BiomassDIO
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

	RVS::DataManagement::DataTable* dt = RVS::Biomass::BiomassDIO::query_biomass_herbs_table(baseBPS);
	double biomass = 0;

	int colCount = sqlite3_column_count(dt->getStmt());
	int rc = sqlite3_step(dt->getStmt());

	for (int i = 0; i < colCount; i++)
	{
		const char* colName = sqlite3_column_name(dt->getStmt(), i);
		if (strcmp(colName, clevel) == 0)
		{
			biomass = sqlite3_column_int(dt->getStmt(), i);  //$$ TODO this isn't getting the data. WHY???
			break;
		}
	}

	delete dt;
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

    double pch = RVS::Biomass::BiomassEquations::eq_PCH(cf1, cf2, height);
	double pch_acre = RVS::Biomass::BiomassEquations::eq_pch_acre(pch);

	// For verbose mode, print out the read and calculated values
    if (!suppress_messages)
    {
        //std::cout << evt->DOM_SPP() << " CF1: " << cf1 << std::endl;
        //std::cout << evt->DOM_SPP() << " CF2: " << cf2 << std::endl;
        //std::cout << evt->DOM_SPP() << " HT: " << evt->HEIGHT() << std::endl;
        //std::cout << evt->DOM_SPP() << " COV: " << evt->COVER() << std::endl;
        //std::cout << evt->DOM_SPP() << " PCH: " << pch << std::endl;
    }
            
    biomass = pch_acre * (evt->COVER() / 100);

    return biomass;
};
