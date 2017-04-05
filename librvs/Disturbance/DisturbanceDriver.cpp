#include "DisturbanceDriver.h"

RVS::Disturbance::DisturbanceDriver::DisturbanceDriver(RVS::Disturbance::DisturbanceDIO* ddio, bool suppress_messages)
{
	this->ddio = ddio;
	this->suppress_messages = suppress_messages;
}

RVS::Disturbance::DisturbanceDriver::~DisturbanceDriver()
{

}

int* RVS::Disturbance::DisturbanceDriver::DisturbanceMain(int year, RVS::DataManagement::AnalysisPlot* ap)
{
	this->ap = ap;

	if (ap->PLOT_ID() == 36)
	{
		int asdas = 0;
	}

	vector<DisturbAction> disturbances = ap->getDisturbancesForYear(year);

	if (disturbances.size() == 0)
	{
		ap->disturbed = false;
		ap->burned = false;
		ap->biomassReductionTotal = 0;
	}
	else
	{
		const string fire = "FIRE";
		const string graze = "GRAZE";
			
		ap->disturbed = true;

		for (auto &d : disturbances)
		{
			const string action = d.getActionType();

			if (action.compare(fire) == 0)
			{
				burnPlot(d.getActionSubType(), 0.0);
				ap->burned = true;
			}
			else if (action.compare(graze) == 0)
			{
				const string grazeStr = d.getActionSubType();
				map<string, double> params = d.getParameters();
				GRAZE_TYPE grazeType;

				if (grazeStr.compare("SHEEP") == 0)
				{
					grazeType = sheep;
				}
				else if (grazeStr.compare("GOAT") == 0)
				{
					grazeType = goat;
				}
				else
				{
					grazeType = cow;
				}

				// $TODO get the parameter names from the database
				grazePlot(grazeType, params["NUMBER"], params["AREA"], params["LENGTH"]);
			}
		}
	}
	return RC;
}

void RVS::Disturbance::DisturbanceDriver::burnPlot(const string fireType, float intensity)
{
	ap->herbHeight = 1;
	ap->herbCover = 1;
	ap->primaryProduction = 0;
	ap->herbHoldoverBiomass = 0;
	ap->previousHerbProductions[0] = 0;
	ap->previousHerbProductions[1] = 0;
	ap->previousHerbProductions[2] = 0;


	for (auto &s : *(ap->SHRUB_RECORDS()))
	{
		s->height = 0.01;
		s->cover = 0.01;
	}
}

void RVS::Disturbance::DisturbanceDriver::grazePlot(GRAZE_TYPE grazeType, double numberGrazers, double plotArea, double grazeTime)
{
	// The removeAmount is calculated in lbs/ac and saved as g/ac
	double removeAmount;

	switch (grazeType)
	{
	case cow:
		removeAmount = (cow_multiplier * numberGrazers * grazeTime) / plotArea;
		break;
	case sheep:
	case goat:
		removeAmount = (small_multiplier * numberGrazers * grazeTime) / plotArea;
		break;
	default:
		break;
	}

	ap->biomassReductionTotal = removeAmount * ap->POUNDS_TO_GRAMS;
}

