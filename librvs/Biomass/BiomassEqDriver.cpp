/// ********************************************************** ///
/// Name: BiomassEqDriver.cpp                                  ///
/// Desc: Driver implementation for individual biomass eq tests///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#include "BiomassEqDriver.h"

#include <array>
#include <cmath>
#include <map>
#include <string>

using RVS::Biomass::BiomassEqDriver;

BiomassEqDriver::BiomassEqDriver(RVS::Biomass::BiomassDIO* bdio, bool suppress_messages)
	: bdio(bdio), ap(nullptr), suppress_messages(suppress_messages)
{
}

int* BiomassEqDriver::BioMain(int equationNumber, RVS::DataManagement::AnalysisPlot* ap)
{
	this->ap = ap;
	if (!ap) return RC;

	std::vector<RVS::DataManagement::SppRecord*>* const shrubs = ap->SHRUB_RECORDS();
	if (!shrubs) return RC;

	for (auto* const s : *shrubs)
	{
		if (!s) continue;
		const double stemsPerAcre = calcStemsPerAcre(s);
		s->stemsPerAcre = stemsPerAcre;
		const double singleBiomass = calcShrubBiomass(equationNumber, s);

		s->shrubBiomass = singleBiomass;
		s->exShrubBiomass = singleBiomass * stemsPerAcre;

		// Write this result to the intermediate table
		RC = bdio->write_intermediate_record(&equationNumber, ap, s);
	}

	return RC;
}

double BiomassEqDriver::calcShrubBiomass(int equationNumber, RVS::DataManagement::SppRecord* record)
{
	if (!record) return 0.0;
	record->batEqNum = equationNumber;

	// Stack allocation - no heap leaks
	std::array<double, 4> coefs{};
	std::array<std::string, 3> paramNames{};
	bdio->query_equation_parameters(equationNumber, paramNames.data(), coefs.data());

	std::map<std::string, double> params;
	for (const auto& name : paramNames)
	{
		if (!name.empty())
		{
			const double val = record->requestValue(name);
			params.emplace(name, val);
		}
	}

	const double biomass = BiomassEquations::eq_BAT(equationNumber, coefs.data(), &params);
	return biomass;
}

double BiomassEqDriver::calcStemsPerAcre(RVS::DataManagement::SppRecord* record)
{
	if (!record) return 0.0;

	// Lookup the equation number from the crosswalk table
	int equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), STEMS_PER_ACRE_EQUATION_FIELD);

	if (equationNumber == 0)
	{
		equationNumber = bdio->query_crosswalk_table(BIOMASS_BACKUP_SPP_CODE, STEMS_PER_ACRE_EQUATION_FIELD);
	}

	record->pchEqNum = equationNumber;
	
	std::array<double, 4> coefs{};

	// Populate the coefficients with values from the biomass equation table
	bdio->query_equation_coefficients(equationNumber, coefs.data());

	const double singleStem = BiomassEquations::eq_PCH(coefs[0], coefs[1], record->HEIGHT());

	// While we're here, calculate width (singleStem is area)
	const double radius = std::sqrt(singleStem / 3.14159265358979323846);
	record->width = radius * 2.0;

	// Expand the single stem result
	// First convert the cm^2 to m^2, then to ACRES
	if (singleStem <= 0.0) return 0.0;
	const double expanded = EXPANSION_FACTOR / (singleStem * 0.0001);
	// Return the expanded amount as a function of percent cover
	const double stemsPerAcre = expanded * (record->COVER() / 100.0);
	return stemsPerAcre;
}
