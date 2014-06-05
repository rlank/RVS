#include "BiomassDriver.h"

using RVS::Biomass::BiomassDriver;


BiomassDriver::BiomassDriver(RVS::Biomass::BiomassDIO* bdio, RVS::Biomass::BiomassLookupLevel level, bool suppress_messages)
{
	this->bdio = bdio;
	this->level = level;
	this->suppress_messages = suppress_messages;
}


BiomassDriver::~BiomassDriver(void)
{
}

int* BiomassDriver::BioMain(int year, RVS::DataManagement::AnalysisPlot* ap, double* retShrubBiomass, double* retHerbBiomass)
{
	this->ap = ap;
	int plot_num = ap->PLOT_ID();

	std::vector<RVS::DataManagement::SppRecord*>* shrubs = ap->SHRUB_RECORDS();
	double totalShrubCover = 0;
	RVS::DataManagement::SppRecord* record = NULL;
	for (int r = 0; r < shrubs->size(); r++)
	{	
		record = shrubs->at(r);
		double stemsPerAcre = calcStemsPerAcre(record);
		record->SET_STEMSPERACRE(stemsPerAcre);
		double singleBiomass = calcShrubBiomass(record);
		record->SET_SHRUBBIOMASS(singleBiomass);
		*retShrubBiomass += singleBiomass;
		totalShrubCover += record->COVER();

		// Write this result to the intermediate table
		RC = bdio->write_biomass_intermediate_record(ap, record, &plot_num, &year, &singleBiomass);
	}

	double production = calcPrimaryProduction();
	*retHerbBiomass = calcHerbBiomass(year);
	double reduction = calcHerbReduction(totalShrubCover);
	*retHerbBiomass = reduction * *retHerbBiomass;
	ap->SET_HERBBIOMASS(*retHerbBiomass);

	int evt = ap->EVT_NUM();
	int bps = ap->BPS_NUM();

	double totalBiomass = *retShrubBiomass + *retHerbBiomass;
	ap->SET_TOTALBIOMASS(totalBiomass);

	// Write output biomass record
	RC = bdio->write_biomass_output_record(&plot_num, &year, &evt, &bps, &totalBiomass, retHerbBiomass, retShrubBiomass);

	return RC;
}

double BiomassDriver::calcPrimaryProduction()
{
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

	double primaryProduction = bdio->query_biomass_pp_table(ap->BPS_NUM(), clevel);
	return primaryProduction;
}

double BiomassDriver::calcShrubBiomass(RVS::DataManagement::SppRecord* record)
{
	// Get the equation number for BAT (total aboveground biomass)
	int equationNumber = bdio->query_biomass_crosswalk_table(record->SPP_CODE(), "BAT");

	// Populate the coefficients with values from the biomass equation table
	double* coefs = new double[4];
	bdio->query_biomass_equation_coefficients(equationNumber, &coefs[0], &coefs[1], &coefs[2], &coefs[3]);

	string* paramNames = new string[3];
	bdio->query_biomass_equation_parameters(equationNumber, &paramNames[0], &paramNames[1], &paramNames[2]);

	std::map<string, double>* params = new std::map<string, double>();

	double val = 0.0;
	for (int i = 0; i < paramNames->size(); i++)
	{
		val = record->requestValue(paramNames[i]);
		params->insert(pair<string, double>(paramNames[i], val));
	}

	double biomass = BiomassEquations::eq_BAT(equationNumber, coefs, params);
	biomass *= record->STEMSPERACRE();
	return biomass;
}

double BiomassDriver::calcStemsPerAcre(RVS::DataManagement::SppRecord* record)
{
	// Lookup the equation number from the crosswalk table
	int equationNumber = bdio->query_biomass_crosswalk_table(record->SPP_CODE(), "PCH");

	double cf1 = 0;
	double cf2 = 0;
	double dum = 0;

	// Populate the coefficients with values from the biomass equation table
	bdio->query_biomass_equation_coefficients(equationNumber, &cf1, &cf2, &dum, &dum);

	double singleStem = BiomassEquations::eq_PCH(cf1, cf2, record->HEIGHT());

	// While we're here, calculate width (singleStem is area)
	double radius = std::sqrt(singleStem / 3.1415); // Use number for PI rather than constant cause it's the only thing needed out of CMATH
	record->SET_WIDTH(radius * 2);

	// Expand the single stem result
	double expanded = (singleStem / 1000) * EXPANSION_FACTOR;
	// Return the expanded amount as a function of percent cover
	double stemsPerAcre = expanded * (record->COVER() / 100);
	return stemsPerAcre;
}

double BiomassDriver::calcHerbBiomass(int year)
{
	// BASIC FORM:
	// Intercept + ln_ndvi + ln_precip_mm + GRP_ID_CONST + (ln_ndvi*grp_id) + (ln_prcp_mm*grp_id)

	// Populate interact fields from database
	double* grp_id_const = new double;
	double* ndvi_grp_interact = new double;
	double* ppt_grp_interact = new double;
	bdio->query_biogroup_coefs(ap->BPS_NUM(), grp_id_const, ndvi_grp_interact, ppt_grp_interact);

	double ndvi = ap->getNDVI(year);
	double ppt = ap->getPPT(year);
	double ln_ndvi = log(ndvi);
	double ln_ppt = log(ppt);

	double biomass = INTERCEPT + *grp_id_const + (ln_ndvi * LN_NDVI) + (ln_ppt * LN_PRECIP) + (ln_ndvi * *ndvi_grp_interact) + (ln_ppt * *ppt_grp_interact);
	return exp(biomass);
}

double BiomassDriver::calcHerbReduction(double totalShrubCover)
{
	//$$ TODO OMG THE MAGIC NUMBERS
	if (totalShrubCover > 85) totalShrubCover = 85;
	double val = -0.0004 * pow(totalShrubCover, 3) + 0.0458 * pow(totalShrubCover, 2) + 4.5394;
	return val / 100;
}