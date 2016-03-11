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
	ap->shrubRecords.clear();
}

void RVS::Disturbance::DisturbanceDriver::grazePlot(GRAZE_TYPE grazeType, double numberGrazers, double plotArea, double grazeTime)
{
	double totalcover = ap->SHRUBCOVER() + ap->HERBCOVER();
	double herbCoverRatio = ap->HERBCOVER() / totalcover;
	double shrubCoverRatio = ap->SHRUBCOVER() / totalcover;

	double removeAmount;

	switch (grazeType)
	{
	case cow:
		removeAmount = (cow_multiplier * numberGrazers * grazeTime) / plotArea;
		ap->herbBiomassReduction = removeAmount;
		break;
	case sheep:
	case goat:
		removeAmount = (small_multiplier * numberGrazers * grazeTime) / plotArea;
		ap->herbBiomassReduction = removeAmount * herbCoverRatio;
		ap->shrubBiomassReduction = (removeAmount * ap->POUNDS_TO_GRAMS) * shrubCoverRatio;
		break;
	default:
		break;
	}
}

