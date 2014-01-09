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
		RVS::DataManagement::DataTable* dt = RVS::DataManagement::DIO::query_biomass_input_table(plot_num);
		sqlite3_stmt* inputStmt = dt->getStmt();

		int rc = SQLITE_OK;
		int row_counter = 0;
		while (rc == SQLITE_OK || rc == SQLITE_ROW)
		{
			rc = sqlite3_step(inputStmt);
			RVS::Biomass::BiomassEVT* bioEVT = new RVS::Biomass::BiomassEVT(dt);
			evt_records.push_back(bioEVT);
			
			/*
			if (row_counter == 0)
			{
				primary_production = this->calcHerbBiomass(bioEVT);
			}
			*/
			double singleBiomass = 0;

			// Branch biomass calculation to do either HERB or SHRUB
			switch (bioEVT->LIFEFORM())
			{
				case RVS::DataManagement::shrub:
					//singleBiomass = this->calcShrubBiomass(bioEVT);
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
		}

		BOOST_FOREACH(RVS::Biomass::BiomassEVT* v, evt_records)
		{
			std::cout << v->toString() << std::endl;
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

	RVS::DataManagement::DataTable dt = RVS::DataManagement::DIO::query_biomass_herbs_table(baseBPS);
    double biomass = 0;
    switch (this->level)
    {
		case RVS::Biomass::low:
            //biomass = Convert.ToDouble(dt.Rows[0]["low_val"]);
            break;
        case RVS::Biomass::medium:
            //biomass = Convert.ToDouble(dt.Rows[0]["med_val"]);
            break;
        case RVS::Biomass::high:
            //biomass = Convert.ToDouble(dt.Rows[0]["high_val"]);
            break;
    }
    return biomass;
};

double BiomassDriver::calcShrubBiomass(RVS::Biomass::BiomassEVT* evt)
{
    double biomass = 0;

    if (this->checkShrubInput() == true)
    {
        biomass = this->calcHerbBiomass(evt);
        return biomass;
    }

    std::string equationCode = RVS::Biomass::ENUMPARSE(evt->RETURN_TYPE());

    RVS::DataManagement::DataTable dt;
    dt = RVS::DataManagement::DIO::query_biomass_crosswalk_table(evt->SPP_CODE().c_str());

    int equationIndex = 0;
    //RVS::DataManagement::DIO.parseItem(dt.Rows[0][equationCode], ref equationIndex);  //$$ TODO

    //if (!suppress_messages)
    //    std::cout << "equ_code: " << equationIndex << std::endl;

    dt = RVS::DataManagement::DIO::query_biomass_equation_table(equationIndex);

    std::string equationstring = "";
    //RVS::DataManagement::DIO.parseItem(dt.Rows[0]["EQ_FRM"], ref equationstring);   //$$ TODO

    double cf1 = 0;
    double cf2 = 0;
    double cf3 = 0;
    double cf4 = 0;

    //DataManagement.DIO.parseItem(dt.Rows[0]["CF1"], ref cf1);    //$$ TODO
    //DataManagement.DIO.parseItem(dt.Rows[0]["CF2"], ref cf2);    //$$ TODO
    //DataManagement.DIO.parseItem(dt.Rows[0]["CF3"], ref cf3);    //$$ TODO
    //DataManagement.DIO.parseItem(dt.Rows[0]["CF4"], ref cf4);    //$$ TODO

    double height = evt->HEIGHT();

    double pch = RVS::Biomass::BiomassEquations::eq_PCH(cf1, cf2, height);
    double pch_acre = (pch / 1000) * 4046.85642;

    if (!suppress_messages)
    {
        std::cout << evt->DOM_SPP() << " CF1: " << cf1 << std::endl;
        std::cout << evt->DOM_SPP() << " CF2: " << cf2 << std::endl;
        std::cout << evt->DOM_SPP() << " HT: " << evt->HEIGHT();
        std::cout << evt->DOM_SPP() << " COV: " << evt->COVER() << std::endl;
        std::cout << evt->DOM_SPP() << " PCH: " << pch << std::endl;
    }
            
    biomass = pch_acre * (evt->COVER() / 100);

    return biomass;
};

bool BiomassDriver::checkShrubInput()
{
    bool useLookuptableInstead = false;
    return useLookuptableInstead;
};