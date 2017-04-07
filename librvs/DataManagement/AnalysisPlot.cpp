#include "AnalysisPlot.h"

using RVS::DataManagement::AnalysisPlot;

AnalysisPlot::AnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	plot_id = 0;
	plot_name = "";
	evt_num = 0;
	bps_num = 0;
	bps_model_num = "";
	shrubHeight = 0;
	shrubCover = 0;
	totalBiomass = 0;
	herbBiomass = 0;
	herbHoldoverBiomass = 0;
	shrubBiomass = 0;
	defaultFBFM = 0;
	calcFBFM = 0;
	dryClimate = true;
	shrubRecords = vector<SppRecord*>();
	ndviValues = vector<double>();
	precipValues = vector<double>();
	disturbances = vector<Disturbance::DisturbAction>();
	disturbed = false;

	previousHerbProductions = new double[3];
	previousHerbProductions[0] = 0;
	previousHerbProductions[1] = 0;
	previousHerbProductions[2] = 0;

	// Order matters here!
	buildAnalysisPlot(dio, dt);
	buildInitialFuels(dio);
}

AnalysisPlot::~AnalysisPlot(void)
{
	shrubRecords.clear();
}

void AnalysisPlot::buildAnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	sqlite3_stmt* stmt = dt->getStmt();
	
	dio->getVal(stmt, dt->Columns[PLOT_NUM_FIELD], &plot_id);
	dio->getVal(stmt, dt->Columns[PLOT_NAME_FIELD], &plot_name);
	dio->getVal(stmt, dt->Columns[EVT_NUM_FIELD], &evt_num);
	dio->getVal(stmt, dt->Columns[BPS_NUM_FIELD], &bps_num);
	dio->getVal(stmt, dt->Columns[BPS_MODEL_FIELD], &bps_model_num);
	dio->getVal(stmt, dt->Columns[HERB_COVER_FIELD], &herbCover);
	dio->getVal(stmt, dt->Columns[HERB_HEIGHT_FIELD], &herbHeight);
	dio->getVal(stmt, dt->Columns[SUCCESSION_CLASS_FIELD], &currentStage);
	dio->getVal(stmt, dt->Columns[LATITUDE_FIELD], &latitude);
	dio->getVal(stmt, dt->Columns[LONGITUDE_FIELD], &longitude);

	int colCount = sqlite3_column_count(stmt);
	std::string ndvi = "NDVI";
	std::string ppt = "PPT";

	for (int i = 0; i < colCount; i++)
	{
		// Get the column name
		const char* colName = sqlite3_column_name(stmt, i);
		std::string colStr = std::string(colName);

		// Get the data out of the column using boost::any for type safety
		double* aval = new double();
		dio->getVal(stmt, i, aval);

		if (colStr.compare(0, ndvi.length(), ndvi) == 0)
		{
			ndviValues.push_back(*aval);
		}
		else if (colStr.compare(0, ppt.length(), ppt) == 0)
		{
			precipValues.push_back(*aval);
		}
	}
}

void AnalysisPlot::push_shrub(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	RVS::DataManagement::SppRecord* record = new RVS::DataManagement::SppRecord(dio, dt);
	push_shrub(record);
}

void AnalysisPlot::push_shrub(RVS::DataManagement::SppRecord* record)
{
	shrubRecords.push_back(record);
}

void AnalysisPlot::update_shrubvalues()
{
	shrubCover = 0;
	shrubHeight = 0;

	if (shrubRecords.size() > 0)
	{
		double runningHeight = 0;
		for (auto &s : shrubRecords)
		{
			shrubCover += s->COVER();
			runningHeight += s->HEIGHT();
		}
		shrubHeight = runningHeight / shrubRecords.size();
	}
	else
	{
		shrubHeight = 0.0;
	}
	
}

void AnalysisPlot::buildInitialFuels(RVS::DataManagement::DIO* dio)
{
	dio->query_fuels_basic_info(&bps_num, &defaultFBFM, &dryClimate);
}


double AnalysisPlot::getNDVI(string level, bool useRand)
{
	double ndvi = 0;
	if (level.compare("Dry") == 0)
	{
		ndvi = ndviValues[0];
	}
	else if (level.compare("Mid-Dry") == 0)
	{
		ndvi = ndviValues[1];
	}
	else if (level.compare("Mid-Wet") == 0)
	{
		ndvi = ndviValues[3];
	}
	else if (level.compare("Wet") == 0)
	{
		ndvi = ndviValues[4];
	}
	else if (level.compare("Normal") == 0)
	{
		ndvi = ndviValues[2];
	}
	else
	{
		ndvi = ndviValues[2];
	}

	if (useRand)
	{
		int r = rand() % 15 + 90;
		ndvi = ndvi * (r / 100);
	}

	return ndvi;
}

double AnalysisPlot::getPPT(string level, bool useRand)
{
	double ppt = 0;
	if (level.compare("Dry") == 0)
	{
		ppt = precipValues[0];
	}
	else if (level.compare("Mid-Dry") == 0)
	{
		ppt = precipValues[1];
	}
	else if (level.compare("Mid-Wet") == 0)
	{
		ppt = precipValues[3];
	}
	else if (level.compare("Wet") == 0)
	{
		ppt = precipValues[4];
	}
	else if (level.compare("Normal") == 0)
	{
		ppt = precipValues[2];
	}
	else
	{
		ppt = precipValues[2];
	}


	if (useRand)
	{
		int r = rand() % 15 + 90;
		ppt = ppt * (r / 100);
	}

	return ppt;
}

vector<RVS::Disturbance::DisturbAction> RVS::DataManagement::AnalysisPlot::getDisturbancesForYear(int year)
{
	vector<Disturbance::DisturbAction> yearDisturbances = vector<Disturbance::DisturbAction>();

	for (auto d : disturbances)
	{
		if (d.getActionYear() == year)
		{
			yearDisturbances.push_back(d);
		}
	}

	return yearDisturbances;
}