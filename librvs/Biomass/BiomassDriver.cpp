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

int* BiomassDriver::BioMain(int year, string* climate, RVS::DataManagement::AnalysisPlot* ap)
{
	this->ap = ap;
	int plot_num = ap->PLOT_ID();
	this->climate = climate;

	double totalShrubBiomass = 0;

	if (plot_num == 74)
	{
		int asdasds = 0;
	}

	/////////// SHRUBS ///////////

	vector<RVS::DataManagement::SppRecord*>* shrubs = ap->SHRUB_RECORDS();
	double totalShrubCover = 0;
	double runShrubHeight = 0;
	double runShrubStem = 0;
	double averageHeight = 0;
	double averageStem = 0;

	if (shrubs->empty())
	{
		totalShrubCover = 0;
		runShrubHeight = 0;
		averageHeight = 0;
	}
	else
	{
		for (auto &s : *shrubs)
		{
			double stemsPerAcre = calcStemsPerAcre(s);
			s->stemsPerAcre = stemsPerAcre;
			double singleBiomass = calcShrubBiomass(s);

			s->shrubBiomass = singleBiomass;
			s->exShrubBiomass = singleBiomass * stemsPerAcre;
			totalShrubBiomass += s->exShrubBiomass;

			totalShrubCover += s->cover;
			runShrubStem += singleBiomass * s->cover;
			runShrubHeight += s->height * s->cover;
		}

		for (auto &s : *shrubs)
		{
			// Write this result to the intermediate table
			RC = bdio->write_intermediate_record(&year, ap, s);
		}

		averageHeight = runShrubHeight / totalShrubCover;
		averageStem = runShrubStem / totalShrubCover;
	}

	ap->shrubCover = totalShrubCover;
	ap->shrubHeight = averageHeight;
	ap->shrubAvgStem = averageStem;

	/////////// HERBS ///////////
	
	// Apply holdover biomass if applicable and grow herbs from last year's production
	if (ap->herbBiomass != 0)
	{
		double holdover = calcHerbHoldover();
		ap->herbHoldoverBiomass = holdover;
	}

	// Save output data

	// Push new production to queue
	ap->previousHerbProductions[2] = ap->previousHerbProductions[1];
	ap->previousHerbProductions[1] = ap->previousHerbProductions[0];
	ap->previousHerbProductions[0] = ap->primaryProduction;

	//ap->herbBiomass = totalHerbBiomass + ap->herbHoldoverBiomass - ap->herbBiomassReduction;
	ap->herbBiomass = ap->primaryProduction + ap->herbHoldoverBiomass;
	//ap->shrubBiomass = totalShrubBiomass - ap->shrubBiomassReduction;
	ap->shrubBiomass = totalShrubBiomass;
	ap->totalBiomass = ap->SHRUBBIOMASS() + ap->HERBBIOMASS();

	// Write output biomass record
	RC = bdio->write_output_record(&year, ap);

	return RC;
}

double BiomassDriver::calcShrubBiomass(RVS::DataManagement::SppRecord* record)
{
	// Get the equation number for BAT (total aboveground biomass)
	int equationNumber = 0;
	
	
	equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), BIOMASS_EQUATION_FIELD);
	if (equationNumber == 0)
	{
		stringstream* s = new stringstream();
		*s << "PLOT_ID: " << ap->PLOT_ID() << ". Shrub biomass equation not found for " << record->SPP_CODE() << ", using ARTR";
		const char* c = bdio->streamToCharPtr(s);
		bdio->write_debug_msg(c);

		equationNumber = bdio->query_crosswalk_table(BIOMASS_BACKUP_SPP_CODE, BIOMASS_EQUATION_FIELD);
	}

	if (equationNumber == 1160 && record->requestValue("VOL") > 20000)
	{
		equationNumber = 1160;
	}
	else if (equationNumber == 1160)
	{
		equationNumber = 1161;
	}

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
	int equationNumber = 0;

	equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), STEMS_PER_ACRE_EQUATION_FIELD);

	if (equationNumber == 0)
	{
		stringstream* s = new stringstream();
		*s << "PLOT_ID: " << ap->PLOT_ID() << ". Stems per acre equation not found for " << record->SPP_CODE() << ", using ARTR";
		const char* c = bdio->streamToCharPtr(s);
		bdio->write_debug_msg(c);

		equationNumber = bdio->query_crosswalk_table(BIOMASS_BACKUP_SPP_CODE, STEMS_PER_ACRE_EQUATION_FIELD);
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

	// 9/27/2017 new stems per acre reduction calculation, fixes performance at low heights
	double reduction = exp(0.0809 * record->HEIGHT()) * 0.0053;
	if (reduction <= 0.453595949) { stemsPerAcre *= reduction; }

	return stemsPerAcre;
}



double BiomassDriver::calcHerbHoldover()
{
	double val = 0;
	val += ap->previousHerbProductions[0] * .3;
	val += ap->previousHerbProductions[1] * .1;
	val += ap->previousHerbProductions[2] * .05;
	return val;
}

double BiomassDriver::calcAttenuation(double herbBiomass)
{
	double herbIndex = ap->herbCover * ap->herbHeight;
	double shrubIndex = ap->shrubCover * ap->shrubHeight;

	double ratio = herbIndex / shrubIndex;
	if (ratio > 1)
	{
		ratio = 1 / ratio;
	}
	return herbBiomass * ratio;
}

