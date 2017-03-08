#include "DisturbanceDIO.h"

RVS::Disturbance::DisturbanceDIO::DisturbanceDIO(void) : RVS::DataManagement::DIO()
{
	query_parameters_table();
}

RVS::Disturbance::DisturbanceDIO::~DisturbanceDIO(void)
{

}

int* RVS::Disturbance::DisturbanceDIO::create_output_table()
{
	return RC;
}

int* RVS::Disturbance::DisturbanceDIO::write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap)
{
	return RC;
	
}

int* RVS::Disturbance::DisturbanceDIO::create_intermediate_table()
{
	return RC;
}


int* RVS::Disturbance::DisturbanceDIO::write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp)
{
	return RC;
}

map<int, vector<RVS::Disturbance::DisturbAction>> RVS::Disturbance::DisturbanceDIO::query_disturbance_input()
{
	const char* sql = query_base(DISTURBANCE_PLOT_TABLE);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb, true);

	sqlite3_stmt* stmt = dt->getStmt();

	map<int, vector<RVS::Disturbance::DisturbAction>> allActions = map<int, vector<RVS::Disturbance::DisturbAction>>();

	while (*RC == SQLITE_ROW)
	{
		int plot_id;
		string actionType;
		string actionSubType;
		int startYear;
		int stopYear;
		int freq;
		double p1_val;
		double p2_val;
		double p3_val;

		getVal(dt->getStmt(), dt->Columns[PLOT_NUM_FIELD], &plot_id);
		getVal(dt->getStmt(), dt->Columns[DIST_TYPE_FIELD], &actionType);
		getVal(dt->getStmt(), dt->Columns[DIST_SUBTYPE_FIELD], &actionSubType);
		getVal(dt->getStmt(), dt->Columns[DIST_BEGIN_FIELD], &startYear);
		getVal(dt->getStmt(), dt->Columns[DIST_END_FIELD], &stopYear);
		getVal(dt->getStmt(), dt->Columns[DIST_FREQ_FIELD], &freq);
		getVal(dt->getStmt(), dt->Columns[DIST_VAL1_FIELD], &p1_val);
		getVal(dt->getStmt(), dt->Columns[DIST_VAL2_FIELD], &p2_val);
		getVal(dt->getStmt(), dt->Columns[DIST_VAL3_FIELD], &p3_val);

		if (allActions.find(plot_id) == allActions.end())
		{
			allActions[plot_id] = vector<RVS::Disturbance::DisturbAction>();
		}

		
		map<string, double> params;

		params.insert(pair<string, double>(availableActions[actionType][actionSubType][0], p1_val));
		params.insert(pair<string, double>(availableActions[actionType][actionSubType][1], p2_val));
		params.insert(pair<string, double>(availableActions[actionType][actionSubType][2], p3_val));

		if (freq == 0)
		{
			DisturbAction d = DisturbAction(startYear, actionType, actionSubType, params);
			allActions[plot_id].push_back(d);
		}
		else
		{
			for (int y = startYear; y <= stopYear; y += freq)
			{
				DisturbAction d = DisturbAction(y, actionType, actionSubType, params);
				allActions[plot_id].push_back(d);
			}
		}

		*RC = sqlite3_step(stmt);
	}

	return allActions;
}

void RVS::Disturbance::DisturbanceDIO::query_parameters_table()
{	
	const char* sql = query_base(DISTURBANCE_TABLE);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb, true);
	sqlite3_stmt* stmt = dt->getStmt();
	while (*RC == SQLITE_ROW)
	{
		string dist_type;
		string dist_subtype;

		getVal(stmt, dt->Columns[DIST_TYPE_FIELD], &dist_type);
		getVal(stmt, dt->Columns[DIST_SUBTYPE_FIELD], &dist_subtype);

		vector<string> params = vector<string>();
		string p;
		getVal(dt->getStmt(), dt->Columns[DIST_VAL1_NAME_FIELD], &p);
		params.push_back(p);
		getVal(dt->getStmt(), dt->Columns[DIST_VAL2_NAME_FIELD], &p);
		params.push_back(p);
		getVal(dt->getStmt(), dt->Columns[DIST_VAL3_NAME_FIELD], &p);
		params.push_back(p);

		availableActions[dist_type][dist_subtype] = params;

		*RC = sqlite3_step(stmt);
	}
}