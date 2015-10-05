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
	shrubFuels = 0;
	herbFuels = 0;
	defaultFBFM = 0;
	calcFBFM = 0;
	dryClimate = false;
	shrubRecords = std::vector<SppRecord*>();
	ndviValues = std::vector<double>();
	precipValues = std::vector<double>();
	totalFuels = std::map<std::string, double>();
	
	// Order matters here!
	buildAnalysisPlot(dio, dt);
	fallback_bps_num = dio->query_backup_bps(huc);
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
	dio->getVal(stmt, dt->Columns["BPS_MODEL"], &bps_model_num);
	dio->getVal(stmt, dt->Columns[HUC_FIELD], &huc);
	dio->getVal(stmt, dt->Columns[HERB_COVER_FIELD], &herbCover);
	dio->getVal(stmt, dt->Columns[HERB_HEIGHT_FIELD], &herbHeight);

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
	return;
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

void AnalysisPlot::buildInitialFuels(RVS::DataManagement::DIO* dio)
{
	try
	{
		dio->query_fuels_basic_info(&bps_num, &defaultFBFM, &dryClimate);
	}
	catch (RVS::DataManagement::DataNotFoundException &ex)
	{
		dio->query_fuels_basic_info(&fallback_bps_num, &defaultFBFM, &dryClimate);
	}
}

double AnalysisPlot::getNDVI(int year)
{
	if (year >= ndviValues.size())
	{
		return ndviValues.at(year % ndviValues.size());
	}
	else
	{
		return ndviValues.at(year);
	}
}

double AnalysisPlot::getPPT(int year)
{
	if (year >= precipValues.size())
	{
		return precipValues.at(year % precipValues.size());
	}
	else
	{
		return precipValues.at(year);
	}
}
