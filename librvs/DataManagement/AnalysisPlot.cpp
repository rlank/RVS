#include "AnalysisPlot.h"

using RVS::DataManagement::AnalysisPlot;

AnalysisPlot::AnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	plot_id = 0;
	evt_num = 0;
	bps_num = 0;
	totalBiomass = 0;
	herbBiomass = 0;
	shrubRecords = std::vector<SppRecord*>();
	ndviValues = std::vector<double>();
	precipValues = std::vector<double>();

	buildAnalysisPlot(dio, dt);
	buildShrubRecords(dio, plot_id);
}

AnalysisPlot::~AnalysisPlot(void)
{
	shrubRecords.clear();
}

void AnalysisPlot::buildAnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	sqlite3_stmt* stmt = dt->getStmt();
	
	int colCount = sqlite3_column_count(stmt);
	std::string ndvi = "NDVI";
	std::string ppt = "PPT";

	for (int i = 0; i < colCount; i++)
	{
		// Get the column name
		const char* colName = sqlite3_column_name(stmt, i);
		std::string colStr = std::string(colName);

		// Get the data out of the column using boost::any for type safety
		boost::any* aval = new boost::any();
		dio->getVal(stmt, i, aval);

		if (strcmp(colName, PLOT_NUM_FIELD) == 0)
		{
			plot_id = boost::any_cast<int>(*aval);
		}
		else if (strcmp(colName, EVT_NUM_FIELD) == 0)
		{
			evt_num = boost::any_cast<int>(*aval);
		}
		else if (strcmp(colName, BPS_NUM_FIELD) == 0)
		{
			bps_num = boost::any_cast<int>(*aval);
		}
		else if (colStr.compare(0, ndvi.length(), ndvi) == 0)
		{
			ndviValues.push_back(boost::any_cast<double>(*aval));
		}
		else if (colStr.compare(0, ppt.length(), ppt) == 0)
		{
			precipValues.push_back(boost::any_cast<double>(*aval));
		}

	}
	return;
}


void AnalysisPlot::buildShrubRecords(RVS::DataManagement::DIO* dio, int plot_num)
{
	// read in data. the query_biomass_input_table function returns a DataTable
	// with only records for that plot number
	RVS::DataManagement::DataTable* dt = dio->query_shrubs_table(plot_num);
	*RC = sqlite3_step(dt->getStmt());
	RVS::DataManagement::SppRecord* spr = NULL;
	while (*RC == SQLITE_ROW)
	{
		spr = new RVS::DataManagement::SppRecord(dio, dt);
		shrubRecords.push_back(spr);
		*RC = sqlite3_step(dt->getStmt());
	}
	delete dt;
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