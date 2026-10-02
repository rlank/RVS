#include "BiomassDriver.h"

#include <array>
#include <cmath>
#include <map>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

namespace RVS
{
namespace Biomass
{

BiomassDriver::BiomassDriver(RVS::Biomass::BiomassDIO* const bdio, const bool suppress_messages)
	: bdio(bdio)
	, ap(nullptr)
	, suppress_messages(suppress_messages)
{
}

int BiomassDriver::BioMain(const int year, std::string_view climate_view, RVS::DataManagement::AnalysisPlot* const analysis_plot)
{
	this->ap = analysis_plot;
	this->climate = std::string(climate_view);

	double totalShrubBiomass = 0.0;

	/////////// SHRUBS ///////////

	std::vector<RVS::DataManagement::SppRecord*>* const shrubs = ap->SHRUB_RECORDS();
	double totalShrubCover = 0.0;
	double runShrubHeight = 0.0;
	double runShrubStem = 0.0;
	double averageHeight = 0.0;
	double averageStem = 0.0;

	if (shrubs && !shrubs->empty())
	{
		for (auto* const s : *shrubs)
		{
			const double stemsPerAcre = calcStemsPerAcre(s);
			s->stemsPerAcre = stemsPerAcre;
			const double singleBiomass = calcShrubBiomass(s);

			s->shrubBiomass = singleBiomass;
			s->exShrubBiomass = singleBiomass * stemsPerAcre;
			totalShrubBiomass += s->exShrubBiomass;

			totalShrubCover += s->cover;
			runShrubStem += singleBiomass * s->cover;
			runShrubHeight += s->height * s->cover;
		}

		int intermediate_year = year;
		for (auto* const s : *shrubs)
		{
			// Write this result to the intermediate table
			int* const rc_intermediate = bdio->write_intermediate_record(&intermediate_year, ap, s);
			(void)rc_intermediate;
		}

		if (totalShrubCover > 1e-4)
		{
			averageHeight = runShrubHeight / totalShrubCover;
			averageStem = runShrubStem / totalShrubCover;
		}
		else
		{
			averageHeight = 0.0;
			averageStem = 0.0;
		}
	}

	ap->shrubCover = totalShrubCover;
	ap->shrubHeight = averageHeight;
	ap->shrubAvgStem = averageStem;

	/////////// HERBS ///////////

	// Apply holdover biomass if applicable and grow herbs from last year's production
	if (ap->herbBiomass != 0.0)
	{
		const double holdover = calcHerbHoldover();
		ap->herbHoldoverBiomass = holdover;
	}

	// Push new production to queue
	ap->previousHerbProductions[2] = ap->previousHerbProductions[1];
	ap->previousHerbProductions[1] = ap->previousHerbProductions[0];
	ap->previousHerbProductions[0] = ap->primaryProduction;

	ap->herbBiomass = ap->primaryProduction + ap->herbHoldoverBiomass;
	ap->shrubBiomass = totalShrubBiomass;
	ap->totalBiomass = ap->SHRUBBIOMASS() + ap->HERBBIOMASS();

	// Write output biomass record
	int output_year = year;
	int* const rc_out = bdio->write_output_record(&output_year, ap);

	return rc_out ? *rc_out : 0;
}

double BiomassDriver::calcCrownWidth(const double crownArea) noexcept
{
	if (crownArea <= 0.0)
	{
		return 0.0;
	}
	const double radius = std::sqrt(crownArea / std::numbers::pi);
	return radius * 2.0;
}

double BiomassDriver::calcStemsPerAcrePure(
	const double singleStemArea,
	const double cover,
	const double height) noexcept
{
	if (singleStemArea <= 0.0)
	{
		return 0.0;
	}

	// Expand the single stem result: cm^2 to m^2 (* 1e-4), then to acres
	const double expanded = EXPANSION_FACTOR / (singleStemArea * 0.0001);
	double stemsPerAcre = expanded * (cover / 100.0);

	// Reduction calculation for low heights
	const double reduction = std::exp(0.0809 * height) * 0.0053;
	if (reduction <= 0.453595949)
	{
		stemsPerAcre *= reduction;
	}

	return stemsPerAcre;
}

double BiomassDriver::calcShrubBiomass(RVS::DataManagement::SppRecord* const record)
{
	// Get the equation number for BAT (total aboveground biomass)
	int equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), BIOMASS_EQUATION_FIELD);

	if (equationNumber == 0)
	{
		std::stringstream s;
		s << "PLOT_ID: " << ap->PLOT_ID() << ". Shrub biomass equation not found for " << record->SPP_CODE() << ", using ARTR";
		bdio->write_debug_msg(s.str().c_str());

		equationNumber = bdio->query_crosswalk_table(BIOMASS_BACKUP_SPP_CODE, BIOMASS_EQUATION_FIELD);
	}

	if (equationNumber == 1160 && record->requestValue("VOL") > 20000.0)
	{
		equationNumber = 1160;
	}
	else if (equationNumber == 1160)
	{
		equationNumber = 1161;
	}

	record->batEqNum = equationNumber;

	// Populate the coefficients with values from the biomass equation table using stack arrays
	std::array<double, 4> coefs{};
	std::array<std::string, 3> paramNames{};
	bdio->query_equation_parameters(equationNumber, paramNames.data(), coefs.data());

	std::map<std::string, double> params;
	for (const auto& paramName : paramNames)
	{
		if (!paramName.empty())
		{
			const double val = record->requestValue(paramName);
			params.emplace(paramName, val);
		}
	}

	const double biomass = BiomassEquations::eq_BAT(equationNumber, coefs, params);
	return biomass;
}

double BiomassDriver::calcStemsPerAcre(RVS::DataManagement::SppRecord* const record)
{
	// Lookup the equation number from the crosswalk table
	int equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), STEMS_PER_ACRE_EQUATION_FIELD);

	if (equationNumber == 0)
	{
		std::stringstream s;
		s << "PLOT_ID: " << ap->PLOT_ID() << ". Stems per acre equation not found for " << record->SPP_CODE() << ", using ARTR";
		bdio->write_debug_msg(s.str().c_str());

		equationNumber = bdio->query_crosswalk_table(BIOMASS_BACKUP_SPP_CODE, STEMS_PER_ACRE_EQUATION_FIELD);
	}

	record->pchEqNum = equationNumber;

	// Stack array eliminates heap allocation
	std::array<double, 4> coefs{};
	bdio->query_equation_coefficients(equationNumber, coefs.data());

	const double singleStem = BiomassEquations::eq_PCH(coefs[0], coefs[1], record->HEIGHT());

	// Calculate width with circle geometry using std::numbers::pi
	record->width = calcCrownWidth(singleStem);

	// Expand the single stem result
	const double stemsPerAcre = calcStemsPerAcrePure(singleStem, record->COVER(), record->HEIGHT());

	return stemsPerAcre;
}

double BiomassDriver::calcHerbHoldover() const noexcept
{
	if (!ap)
	{
		return 0.0;
	}
	return calcHerbHoldover(
		ap->previousHerbProductions[0],
		ap->previousHerbProductions[1],
		ap->previousHerbProductions[2]);
}

double BiomassDriver::calcAttenuation(const double herbBiomass) const noexcept
{
	if (!ap)
	{
		return herbBiomass;
	}
	return calcAttenuation(
		herbBiomass,
		ap->herbCover,
		ap->herbHeight,
		ap->shrubCover,
		ap->shrubHeight);
}

} // namespace Biomass
} // namespace RVS
