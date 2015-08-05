#include "BiomassEqDriver.h"

using RVS::Biomass::BiomassEqDriver;


BiomassEqDriver::BiomassEqDriver(RVS::Biomass::BiomassDIO* bdio, bool suppress_messages)
{
	this->bdio = bdio;
	this->suppress_messages = suppress_messages;
}


BiomassEqDriver::~BiomassEqDriver(void)
{
}

int* BiomassEqDriver::BioMain(int equationNumber, RVS::DataManagement::AnalysisPlot* ap)
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
		double singleBiomass = calcShrubBiomass(equationNumber, record);

		record->shrubBiomass = singleBiomass;
		record->exShrubBiomass = singleBiomass * stemsPerAcre;

		// Write this result to the intermediate table
		RC = bdio->write_intermediate_record(&equationNumber, ap, record);
	}


	return RC;
}

double BiomassEqDriver::calcShrubBiomass(int equationNumber, RVS::DataManagement::SppRecord* record)
{
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

double BiomassEqDriver::calcStemsPerAcre(RVS::DataManagement::SppRecord* record)
{
	// Lookup the equation number from the crosswalk table
	int equationNumber = 0;
	try
	{
		equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), "PCH");
	}
	catch (RVS::DataManagement::DataNotFoundException dex)
	{
		equationNumber = bdio->query_crosswalk_table("ARTR2", "PCH");
	}
	
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

