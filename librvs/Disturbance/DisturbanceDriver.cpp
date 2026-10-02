#include "DisturbanceDriver.h"

namespace RVS
{
namespace Disturbance
{
	DisturbanceDriver::DisturbanceDriver(RVS::Disturbance::DisturbanceDIO* ddio, const bool suppress_messages)
		: ddio(ddio), suppress_messages(suppress_messages), ap(nullptr)
	{
	}

	int DisturbanceDriver::DisturbanceMain(const int year, RVS::DataManagement::AnalysisPlot* ap)
	{
		this->ap = ap;
		if (!ap)
		{
			return -1;
		}

		const std::vector<DisturbAction> disturbances = ap->getDisturbancesForYear(year);

		if (disturbances.empty())
		{
			ap->disturbed = false;
			ap->burned = false;
			ap->biomassReductionTotal = 0.0;
		}
		else
		{
			static constexpr std::string_view FIRE_ACTION = "FIRE";
			static constexpr std::string_view GRAZE_ACTION = "GRAZE";

			ap->disturbed = true;

			for (const auto& d : disturbances)
			{
				const std::string& action = d.getActionType();

				if (action == FIRE_ACTION)
				{
					burnPlot(d.getActionSubType(), 0.0f);
					ap->burned = true;
				}
				else if (action == GRAZE_ACTION)
				{
					const std::string& grazeStr = d.getActionSubType();
					const std::map<std::string, double>& params = d.getParameters();
					GrazeType grazeType = GrazeType::Cow;

					if (grazeStr == "SHEEP")
					{
						grazeType = GrazeType::Sheep;
					}
					else if (grazeStr == "GOAT")
					{
						grazeType = GrazeType::Goat;
					}
					else
					{
						grazeType = GrazeType::Cow;
					}

					const double numberGrazers = params.count("NUMBER") ? params.at("NUMBER") : 0.0;
					const double plotArea = params.count("AREA") ? params.at("AREA") : 1.0;
					const double grazeTime = params.count("LENGTH") ? params.at("LENGTH") : 0.0;

					grazePlot(grazeType, numberGrazers, plotArea, grazeTime);
				}
			}
		}

		return 0;
	}

	void DisturbanceDriver::burnPlot(std::string_view fireType, const float intensity)
	{
		if (!ap) return;

		ap->herbHeight = 1.0;
		ap->herbCover = 1.0;
		ap->primaryProduction = 0.0;
		ap->herbHoldoverBiomass = 0.0;
		ap->previousHerbProductions.fill(0.0);

		if (ap->SHRUB_RECORDS())
		{
			for (auto& s : *(ap->SHRUB_RECORDS()))
			{
				if (s)
				{
					s->height = 0.01;
					s->cover = 0.01;
				}
			}
		}
	}

	void DisturbanceDriver::grazePlot(
		const GrazeType grazeType,
		const double numberGrazers,
		const double plotArea,
		const double grazeTime)
	{
		if (!ap || plotArea <= 0.0) return;

		double multiplier = COW_MULTIPLIER;
		switch (grazeType)
		{
		case GrazeType::Cow:
			multiplier = COW_MULTIPLIER;
			break;
		case GrazeType::Sheep:
		case GrazeType::Goat:
			multiplier = SMALL_MULTIPLIER;
			break;
		}

		const double removeAmountLbs = (multiplier * numberGrazers * grazeTime) / plotArea;
		ap->biomassReductionTotal = removeAmountLbs * ap->POUNDS_TO_GRAMS;
	}
}
}
