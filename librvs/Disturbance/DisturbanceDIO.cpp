#include "DisturbanceDIO.h"

namespace RVS
{
namespace Disturbance
{
	DisturbanceDIO::DisturbanceDIO() : RVS::DataManagement::DIO()
	{
		query_parameters_table();
	}

	int* DisturbanceDIO::create_output_table()
	{
		return RC;
	}

	int* DisturbanceDIO::write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap)
	{
		return RC;
	}

	int* DisturbanceDIO::create_intermediate_table()
	{
		return RC;
	}

	int* DisturbanceDIO::write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp)
	{
		return RC;
	}

	std::map<int, std::vector<DisturbAction>> DisturbanceDIO::query_disturbance_input()
	{
		const char* sql = query_base(DISTURBANCE_PLOT_TABLE);
		RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb, true);
		if (!dt)
		{
			return {};
		}

		sqlite3_stmt* stmt = dt->getStmt();
		std::map<int, std::vector<DisturbAction>> allActions;

		while (RC && *RC == SQLITE_ROW)
		{
			int plot_id = 0;
			std::string actionType;
			std::string actionSubType;
			int startYear = 0;
			int stopYear = 0;
			int freq = 0;
			double p1_val = 0.0;
			double p2_val = 0.0;
			double p3_val = 0.0;

			getVal(dt->getStmt(), dt->Columns[PLOT_NUM_FIELD], &plot_id);
			getVal(dt->getStmt(), dt->Columns[DIST_TYPE_FIELD], &actionType);
			getVal(dt->getStmt(), dt->Columns[DIST_SUBTYPE_FIELD], &actionSubType);
			getVal(dt->getStmt(), dt->Columns[DIST_BEGIN_FIELD], &startYear);
			getVal(dt->getStmt(), dt->Columns[DIST_END_FIELD], &stopYear);
			getVal(dt->getStmt(), dt->Columns[DIST_FREQ_FIELD], &freq);
			getVal(dt->getStmt(), dt->Columns[DIST_VAL1_FIELD], &p1_val);
			getVal(dt->getStmt(), dt->Columns[DIST_VAL2_FIELD], &p2_val);
			getVal(dt->getStmt(), dt->Columns[DIST_VAL3_FIELD], &p3_val);

			std::map<std::string, double> params;
			const auto& typeActions = availableActions[actionType];
			auto subIt = typeActions.find(actionSubType);
			if (subIt != typeActions.end() && subIt->second.size() >= 3)
			{
				params[subIt->second[0]] = p1_val;
				params[subIt->second[1]] = p2_val;
				params[subIt->second[2]] = p3_val;
			}
			else
			{
				params["NUMBER"] = p1_val;
				params["AREA"] = p2_val;
				params["LENGTH"] = p3_val;
			}

			if (freq == 0)
			{
				allActions[plot_id].emplace_back(startYear, actionType, actionSubType, params);
			}
			else
			{
				for (int y = startYear; y <= stopYear; y += freq)
				{
					allActions[plot_id].emplace_back(y, actionType, actionSubType, params);
				}
			}

			*RC = sqlite3_step(stmt);
		}

		return allActions;
	}

	void DisturbanceDIO::query_parameters_table()
	{
		const char* sql = query_base(DISTURBANCE_TABLE);
		RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb, true);
		if (!dt) return;

		sqlite3_stmt* stmt = dt->getStmt();
		while (RC && *RC == SQLITE_ROW)
		{
			std::string dist_type;
			std::string dist_subtype;

			getVal(stmt, dt->Columns[DIST_TYPE_FIELD], &dist_type);
			getVal(stmt, dt->Columns[DIST_SUBTYPE_FIELD], &dist_subtype);

			std::vector<std::string> params;
			std::string p;
			getVal(dt->getStmt(), dt->Columns[DIST_VAL1_NAME_FIELD], &p);
			params.push_back(p);
			getVal(dt->getStmt(), dt->Columns[DIST_VAL2_NAME_FIELD], &p);
			params.push_back(p);
			getVal(dt->getStmt(), dt->Columns[DIST_VAL3_NAME_FIELD], &p);
			params.push_back(p);

			availableActions[dist_type][dist_subtype] = std::move(params);

			*RC = sqlite3_step(stmt);
		}
	}
}
}
