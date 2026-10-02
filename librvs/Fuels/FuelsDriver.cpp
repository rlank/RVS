#include "FuelsDriver.h"

#include <array>
#include <algorithm>

namespace RVS
{
namespace Fuels
{
	FuelsDriver::FuelsDriver(RVS::Fuels::FuelsDIO* fdio, const bool suppress_messages)
		: fdio(fdio), suppress_messages(suppress_messages), ap(nullptr)
	{
	}

	int FuelsDriver::FuelsMain(const int year, RVS::DataManagement::AnalysisPlot* ap)
	{
		this->ap = ap;
		if (!ap || !fdio)
		{
			return -1;
		}

		const double bio = ap->shrubBiomass;
		const double shrub1HourWB = calc1HrWoodBark(bio);
		const double shrub1HourFoliage = calc1HrFoliage(bio);
		const double shrub10Hour = calc10HrFuel(bio);
		const double shrub100Hour = calc100HrFuel(bio);
		const double shrub1000Hour = calc1000HrFuel(ap->shrubHeight);

		const double herbFuel = ap->herbBiomass * ap->POUNDS_TO_GRAMS;
		const double fuelTot = shrub1HourWB + shrub1HourFoliage + shrub10Hour + shrub100Hour + shrub1000Hour;

		const double prop1HrWB = FuelsEquations::calcFuelProportion(shrub1HourWB, fuelTot);
		const double prop1HrFol = FuelsEquations::calcFuelProportion(shrub1HourFoliage, fuelTot);
		const double prop10Hr = FuelsEquations::calcFuelProportion(shrub10Hour, fuelTot);
		const double prop100Hr = FuelsEquations::calcFuelProportion(shrub100Hour, fuelTot);
		const double prop1000Hr = FuelsEquations::calcFuelProportion(shrub1000Hour, fuelTot);

		ap->herbFuel = herbFuel;
		ap->shrub1HourWB = bio * prop1HrWB;
		ap->shrub1HourFoliage = bio * prop1HrFol;
		ap->shrub10Hour = bio * prop10Hr;
		ap->shrub100Hour = bio * prop100Hr;
		ap->shrub1000Hour = bio * prop1000Hr;

		ap->total1HrFuel = ap->herbFuel + ap->shrub1HourWB + ap->shrub1HourFoliage;

		if (ap->disturbed)
		{
			applyDisturbance(year);
		}

		if (ap->FUEL_TOTAL() < 200.0)
		{
			ap->fbfmName = "NB";
		}
		else
		{
			if (ap->ISDRY())
			{
				ap->fbfmName = calcFBFMDry();
			}
			else
			{
				ap->fbfmName = calcFBFMHumid();
			}
		}

		// Write out the total fuels record using modern value call
		const int rc = fdio->write_output_record(year, ap);
		return rc;
	}

	double FuelsDriver::calcShrubFuel(const int equationNumber, RVS::DataManagement::SppRecord* spp)
	{
		if (!spp || !fdio)
		{
			return 0.0;
		}

		std::array<std::string, 3> paramNames{};
		std::array<double, 4> coefs{};
		int equationType = 0;

		// Populate equation parameters via DIO
		fdio->query_equation_parameters(equationNumber, paramNames.data(), coefs.data(), &equationType);

		// Get the parameter values from the shrub record
		std::array<double, 3> params{};
		for (size_t i = 0; i < paramNames.size(); ++i)
		{
			if (!paramNames[i].empty())
			{
				params[i] = spp->requestValue(paramNames[i]);
			}
		}

		// Calculate fuels using pure function and span
		double fuel = FuelsEquations::calcFuels(equationType, coefs, params);
		fuel *= spp->stemsPerAcre;
		return fuel;
	}

	std::string FuelsDriver::calcFBFMDry() const
	{
		if (!ap || ap->FUEL_TOTAL_1HR() <= 0.0)
		{
			return "NB";
		}

		const double shrubRelativeFuel = (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB()) / ap->FUEL_TOTAL_1HR();
		const double herbRelativeFuel = ap->HERB_FUEL() / ap->FUEL_TOTAL_1HR();
		const double shrubFineFuel = ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB();
		const double herbFuel = ap->HERB_FUEL();

		std::string FBFM;

		if (shrubRelativeFuel >= 0.8)
		{
			if (shrubFineFuel <= 1500.0) { FBFM = "SH1"; }
			else if (shrubFineFuel <= 3000.0) { FBFM = "SH2"; }
			else if (shrubFineFuel > 3000.0) { FBFM = "SH5"; }
		}
		else if (shrubRelativeFuel >= 0.5)
		{
			if (shrubFineFuel <= 1500.0) { FBFM = "SH1"; }
			else if (shrubFineFuel <= 3000.0) { FBFM = "SH7"; }
			else if (shrubFineFuel > 3000.0) { FBFM = "SH7"; }
		}
		else if (herbRelativeFuel >= 0.8)
		{
			if (herbFuel <= 1000.0) { FBFM = "GR1"; }
			else if (herbFuel <= 2000.0) { FBFM = "GR2"; }
			else if (herbFuel > 2000.0) { FBFM = "GR4"; }
		}
		else if (herbRelativeFuel >= 0.5)
		{
			if (shrubFineFuel <= 1000.0) { FBFM = "GS1"; }
			else if (shrubFineFuel > 1000.0) { FBFM = "GS2"; }
		}

		if (FBFM.empty())
		{
			FBFM = "Unk8";
		}
		return FBFM;
	}

	std::string FuelsDriver::calcFBFMHumid() const
	{
		if (!ap || ap->FUEL_TOTAL_1HR() <= 0.0)
		{
			return "NB";
		}

		const double shrubRelativeFuel = (ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB()) / ap->FUEL_TOTAL_1HR();
		const double herbRelativeFuel = ap->HERB_FUEL() / ap->FUEL_TOTAL_1HR();
		const double shrubFineFuel = ap->SHRUB_1HR_FOLIAGE() + ap->SHRUB_1HR_WB();
		const double herbFuel = ap->HERB_FUEL();

		std::string FBFM;

		if (shrubRelativeFuel >= 0.8)
		{
			if (shrubFineFuel <= 1500.0) { FBFM = "SH3"; }
			else if (shrubFineFuel <= 3000.0) { FBFM = "SH4"; }
			else if (shrubFineFuel <= 4500.0) { FBFM = "SH6"; }
			else if (shrubFineFuel > 4500.0) { FBFM = "SH8"; }
		}
		else if (shrubRelativeFuel >= 0.5)
		{
			if (shrubFineFuel <= 1500.0) { FBFM = "SH3"; }
			else if (shrubFineFuel <= 3000.0) { FBFM = "SH4"; }
			else if (shrubFineFuel <= 4500.0) { FBFM = "SH6"; }
			else if (shrubFineFuel > 4500.0) { FBFM = "SH9"; }
		}
		else if (herbRelativeFuel >= 0.8)
		{
			if (herbFuel <= 1000.0) { FBFM = "GR1"; }
			else if (herbFuel <= 2000.0) { FBFM = "GR3"; }
			else if (herbFuel <= 3000.0) { FBFM = "GR5"; }
			else if (herbFuel > 3000.0) { FBFM = "GR6"; }
		}
		else if (herbRelativeFuel >= 0.5)
		{
			if (shrubFineFuel <= 1000.0) { FBFM = "GS1"; }
			else if (shrubFineFuel <= 2000.0) { FBFM = "GS3"; }
			else if (shrubFineFuel > 2000.0) { FBFM = "GS4"; }
		}

		if (FBFM.empty())
		{
			FBFM = "Unk8";
		}
		return FBFM;
	}

	int FuelsDriver::switchClimateFBFM(RVS::DataManagement::DataTable* dt, const RVS::DataManagement::AnalysisPlot* ap)
	{
		int fbfm = 0;
		if (!dt || !ap || !fdio) return 0;

		if (ap->ISDRY())
		{
			fdio->getVal(dt->getStmt(), dt->Columns[FC_FBFM_DRY_FIELD], &fbfm);
		}
		else
		{
			fdio->getVal(dt->getStmt(), dt->Columns[FC_FBFM_HUMID_FIELD], &fbfm);
		}

		return fbfm;
	}

	double FuelsDriver::calc10HrFuel(const double biomass) const noexcept
	{
		const double avgStem = ap ? ap->shrubAvgStem : 0.0;
		return FuelsEquations::calc10HrFuel(biomass, avgStem);
	}

	void FuelsDriver::applyDisturbance(const int year)
	{
		if (!ap) return;

		const std::vector<RVS::Disturbance::DisturbAction> disturbances = ap->getDisturbancesForYear(year);
		if (disturbances.empty())
		{
			return;
		}

		// Fire is handled in DisturbanceDriver, skip if fire
		if (disturbances[0].getActionType() == "FIRE")
		{
			return;
		}

		const double totalCover = ap->HERBCOVER() + ap->SHRUBCOVER();
		const double herbAbundance = (totalCover > 0.0) ? (ap->HERBCOVER() / totalCover * 100.0) : 0.0;
		const double shrubAbundance = (totalCover > 0.0) ? (ap->SHRUBCOVER() / totalCover * 100.0) : 0.0;

		for (const auto& d : disturbances)
		{
			double herbPreference = 50.0;
			double shrubPreference = 50.0;

			if (d.getActionSubType() == "COW")
			{
				herbPreference = 99.0;
				shrubPreference = 1.0;
			}

			const double oldHerbBiomass = ap->herbFuel;
			const double oldShrubBiomass = ap->shrubBiomass;

			const double herbRatio = herbPreference * herbAbundance;
			const double shrubRatio = shrubPreference * shrubAbundance;
			double herbsPerShrub = (shrubRatio > 0.0) ? (herbRatio / shrubRatio) : 1.0;

			if (herbsPerShrub < 1.0) { herbsPerShrub = 1.0; }

			const double herbReduction = (1.0 - 1.0 / herbsPerShrub) * ap->biomassReductionTotal;
			ap->herbFuel -= herbReduction;

			double shrubBiomassReduction = (1.0 / herbsPerShrub) * ap->biomassReductionTotal;
			if (ap->herbFuel < 0.0)
			{
				shrubBiomassReduction += (-ap->herbFuel);
				ap->herbFuel = 0.1;
			}

			ap->shrub1HourFoliage -= shrubBiomassReduction;
			if (ap->shrub1HourFoliage < 0.0)
			{
				ap->shrub1HourWB += ap->shrub1HourFoliage;
				ap->shrub1HourFoliage = 0.1;
			}
			if (ap->shrub1HourWB < 0.0)
			{
				ap->shrub10Hour += ap->shrub1HourWB;
				ap->shrub1HourWB = 0.1;
			}

			if (ap->herbCover > 0.0 && oldHerbBiomass > 0.0)
			{
				const double herbReductionRatio = ap->herbFuel / oldHerbBiomass;
				ap->herbCover *= herbReductionRatio;
				if (ap->herbCover < 0.0) { ap->herbCover = 0.1; }
				ap->herbHeight *= herbReductionRatio;
				if (ap->herbHeight < 0.0) { ap->herbHeight = 0.1; }
			}

			const double newShrubBiomass = ap->shrub1HourFoliage + ap->shrub1HourWB +
				ap->shrub10Hour + ap->shrub100Hour + ap->shrub1000Hour;

			if (oldShrubBiomass > 0.0)
			{
				const double shrubReductionRatio = newShrubBiomass / oldShrubBiomass;
				for (auto& s : ap->shrubRecords)
				{
					if (s)
					{
						s->cover *= shrubReductionRatio;
						if (s->cover < 0.0) { s->cover = 0.1; }
					}
				}
			}

			ap->herbBiomass = ap->herbFuel * ap->GRAMS_TO_POUNDS;
			ap->shrubBiomass = newShrubBiomass;
			ap->total1HrFuel = ap->herbFuel + ap->shrub1HourWB + ap->shrub1HourFoliage;
		}
	}
}
}
