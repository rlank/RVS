#include "BiomassDriver.h"

using RVS::Biomass::BiomassDriver;


BiomassDriver::BiomassDriver(int plot_num, RVS::Biomass::BiomassDIO* bdio, RVS::Biomass::BiomassLookupLevel level, bool suppress_messages, bool write_intermediate)
{
	this->plot_num = plot_num;
	this->bdio = bdio;
	this->level = level;
	this->suppress_messages = suppress_messages;
	this->write_intermediate = write_intermediate;
	this->createEVTRecords(plot_num);
}


BiomassDriver::~BiomassDriver(void)
{
}

int* BiomassDriver::BioMain(int year, double* retShrubBiomass, double* retHerbBiomass)
{
	try
	{
		// declare local return vars
		double BIOMASS_RETURN_VAL = 0;
		RVS::Biomass::BiomassReturnType BIOMASS_RET_TYPE = RVS::Biomass::RNUL;

		double production = 0;
		double totalShrubCover = 0;

		RVS::Biomass::BiomassEVT* bioEVT = NULL;
		for (int r = 0; r < evt_records.size(); r++)
		{
			bioEVT = evt_records.at(r);
			// First item, get the primary production and herbaceous biomass.
			// This is only done once per plot.
			if (r == 0)
			{
				production = this->calcPrimaryProduction(bioEVT);
				*retHerbBiomass = this->calcHerbBiomass(bioEVT, year);
			}
			
			double singleBiomass = this->calcShrubBiomass(bioEVT);
			*retShrubBiomass += singleBiomass;
			totalShrubCover += bioEVT->COVER();

			/* This switch no longer relevent to our model
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
			*/

			// Write this result to the intermediate table
			if (write_intermediate)
			{
				RC = bdio->write_biomass_intermediate_record(bioEVT, &plot_num, &year, &singleBiomass);
			}
		}

		double reduction = calcHerbReduction(totalShrubCover);
		*retHerbBiomass = reduction * *retHerbBiomass;

		bioEVT->SET_SHRUBBIOMASS(*retShrubBiomass);
		bioEVT->SET_HERBBIOMASS(*retHerbBiomass);

		int evt = bioEVT->EVT_NUM();
		std::string bps = (char*)bioEVT->BPS_NUM().c_str();
		char* ret_code = ENUMPARSE(bioEVT->RETURN_TYPE());

		BIOMASS_RETURN_VAL = *retShrubBiomass + *retHerbBiomass;
		BIOMASS_RET_TYPE = RVS::Biomass::PCH;

		if (!suppress_messages)
		{
			std::cout << std::endl;
			std::cout << "Primary Production: " << production << std::endl;
			std::cout << "Shrub Biomass: " << *retShrubBiomass << std::endl;
			std::cout << "Herb Biomass: " << *retHerbBiomass << std::endl;
			std::cout << "Herb Reduction: " << reduction << std::endl;
			std::cout << "Total Cover: " << totalShrubCover << std::endl;
			std::cout << "Total Biomass: " << (*retShrubBiomass + *retHerbBiomass) << std::endl;
		}

		// Write output biomass record
		RC = bdio->write_biomass_output_record(&plot_num, &year, &evt, &bps, ret_code, &BIOMASS_RETURN_VAL, retHerbBiomass, retShrubBiomass);
	}
	catch (std::exception& e)
	{
		std::cout << "Exception in BioMain: " << e.what() << std::endl;
	}

	return RC;
}

double BiomassDriver::calcPrimaryProduction(RVS::Biomass::BiomassEVT* evt)
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

	double biomass = bdio->query_biomass_herbs_table(baseBPS, clevel);
    return biomass;
}

double BiomassDriver::calcShrubBiomass(RVS::Biomass::BiomassEVT* evt)
{
    double biomass = 0;

	// Query the biomass crosswalk table for the equation for the requested return type
    //std::string returnType = RVS::Biomass::ENUMPARSE(evt->RETURN_TYPE());
	std::string returnType = "PCH";
	int equationNumber = bdio->query_biomass_crosswalk_table(evt->SPP_CODE(), returnType);

	float cf1 = 0;
	float cf2 = 0;
	float cf3 = 0;
	float cf4 = 0;

	// Populate the coefficients with values from the biomass equation table
	bdio->query_biomass_equation_coefficients(equationNumber, &cf1, &cf2, &cf3, &cf4);

    double height = evt->HEIGHT();

	//$$ TODO set up "get shrub biomass" to return different types of (requested) biomass
    double pch = RVS::Biomass::BiomassEquations::eq_PCH(cf1, cf2, height);
	//double pch_acre = RVS::Biomass::BiomassEquations::eq_pch_acre(pch);
	double pch_expanded = evt->expandCalculationToPlot(pch);
            
	biomass = pch_expanded * (evt->COVER() / 100);  // Return the expanded amount as a function of percent cover

    return biomass;
}

double BiomassDriver::calcHerbBiomass(RVS::Biomass::BiomassEVT* evt, int year)
{
	// BASIC FORM:
	// Intercept + ln_ndvi + ln_precip_mm + GRP_ID_CONST + (ln_ndvi*grp_id) + (ln_prcp_mm*grp_id)

	// Populate interact fields from database
	double* grp_id_const = new double;
	double* ndvi_grp_interact = new double;
	double* ppt_grp_interact = new double;
	bdio->query_biogroup_coefs(evt->BPS_NUM(), grp_id_const, ndvi_grp_interact, ppt_grp_interact);
	//bdio->query_biogroup_coefs("1111320", grp_id_const, ndvi_grp_interact, ppt_grp_interact);

	double ndvi = evt->getNDVI(year);
	double ppt = evt->getPPT(year);
	double ln_ndvi = log(ndvi);
	double ln_ppt = log(ppt);

	double biomass = INTERCEPT + *grp_id_const + (ln_ndvi * LN_NDVI) + (ln_ppt * LN_PRECIP) + (ln_ndvi * *ndvi_grp_interact) + (ln_ppt * *ppt_grp_interact);
	return exp(biomass);
}

double BiomassDriver::calcHerbReduction(double totalShrubCover)
{
	if (totalShrubCover > 85) totalShrubCover = 85;
	double val = -0.0004 * pow(totalShrubCover, 3) + 0.0458 * pow(totalShrubCover, 2) + 4.5394;
	return val / 100;
}

void BiomassDriver::createEVTRecords(int plot_num)
{
	// read in data. the query_biomass_input_table function returns a DataTable
	// with only records for that plot number
	RVS::DataManagement::DataTable* dt = bdio->query_biomass_input_table(plot_num);

	*RC = sqlite3_step(dt->getStmt());
	RVS::Biomass::BiomassEVT* bioEVT = NULL;
	while (*RC == SQLITE_ROW)
	{
		bioEVT = new RVS::Biomass::BiomassEVT(dt);
		evt_records.push_back(bioEVT);
		*RC = sqlite3_step(dt->getStmt());
	}
	delete dt;
}