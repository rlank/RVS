#include "BiomassDriver.h"

using RVS::Biomass::BiomassDriver;


BiomassDriver::BiomassDriver(RVS::Biomass::BiomassDIO* bdio, bool suppress_messages)
{
	this->bdio = bdio;
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
	double runShrubHeight = 0;
	RVS::DataManagement::SppRecord* record = NULL;
	for (int r = 0; r < shrubs->size(); r++)
	{
		record = shrubs->at(r);
		double stemsPerAcre = calcStemsPerAcre(record);
		record->stemsPerAcre = stemsPerAcre;
		double singleBiomass = calcShrubBiomass(record);

		//$$ TEMP fudging biomass
		singleBiomass *= .08;

		record->shrubBiomass = singleBiomass;
		record->exShrubBiomass = singleBiomass * stemsPerAcre;
		*retShrubBiomass += record->exShrubBiomass;

		totalShrubCover += record->cover;
		runShrubHeight += record->height * record->cover;

		// Write this result to the intermediate table
		RC = bdio->write_intermediate_record(&year, ap, record);
	}

	// Apply holdover biomass if applicable
	if (ap->herbBiomass != 0)
	{
		double holdover = calcHerbReduction(totalShrubCover);
		ap->herbHoldoverBiomass = holdover * ap->herbBiomass;
	}

	*retHerbBiomass = calcHerbBiomass(year);

	double averageHeight = runShrubHeight / totalShrubCover;

	ap->herbBiomass = *retHerbBiomass + ap->herbHoldoverBiomass;
	ap->shrubBiomass = *retShrubBiomass;
	ap->shrubCover = totalShrubCover;
	ap->shrubHeight = averageHeight;

	ap->totalBiomass = ap->SHRUBBIOMASS() + ap->HERBBIOMASS();

	// Write output biomass record
	RC = bdio->write_output_record(&year, ap);

	return RC;
}

double BiomassDriver::calcShrubBiomass(RVS::DataManagement::SppRecord* record)
{
	// Get the equation number for BAT (total aboveground biomass)
	int equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), "BAT");
	record->batEqNum = equationNumber;

	// Populate the coefficients with values from the biomass equation table
	double* coefs = new double[4];
	string* paramNames = new string[3];
	bdio->query_equation_parameters(equationNumber, paramNames, coefs);

	std::map<string, double>* params = new std::map<string, double>();

	double val = 0.0;
	for (int i = 0; i < paramNames->size(); i++)
	{
		val = record->requestValue(paramNames[i]);
		params->insert(pair<string, double>(paramNames[i], val));
	}

	double biomass = BiomassEquations::eq_BAT(equationNumber, coefs, params);
	return biomass;
}

double BiomassDriver::calcStemsPerAcre(RVS::DataManagement::SppRecord* record)
{
	// Lookup the equation number from the crosswalk table
	int equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), "PCH");
	record->pchEqNum = equationNumber;
	
	double* coefs = new double[4];

	// Populate the coefficients with values from the biomass equation table
	bdio->query_equation_coefficients(equationNumber, coefs);

	double singleStem = BiomassEquations::eq_PCH(coefs[0], coefs[1], record->HEIGHT());

	// While we're here, calculate width (singleStem is area)
	double radius = std::sqrt(singleStem / 3.1415); // Use number for PI rather than constant cause it's the only thing needed out of CMATH
	record->width = radius * 2;

	// Expand the single stem result
	// First convert the cm^2 to m^2, then to ACRES
	double expanded = EXPANSION_FACTOR / (singleStem * .0001);
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
	try
	{
		bdio->query_biogroup_coefs(ap->BPS_NUM(), grp_id_const, ndvi_grp_interact, ppt_grp_interact);
	}
	catch (RVS::DataManagement::DataNotFoundException &ex)
	{
		bdio->query_biogroup_coefs(ap->BPS_NUM(true), grp_id_const, ndvi_grp_interact, ppt_grp_interact);
	}

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