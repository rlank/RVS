#include "SuccessionDIO.h"

#include <algorithm>

namespace RVS
{
namespace Succession
{
	SuccessionDIO::SuccessionDIO() : RVS::DataManagement::DIO()
	{
		this->create_output_table();
		this->create_intermediate_table();
	}

	int* SuccessionDIO::create_output_table()
	{
		std::stringstream sqlstream;
		sqlstream << "CREATE TABLE IF NOT EXISTS Succession_Output (" <<
			PLOT_NUM_FIELD << " INTEGER NOT NULL, " <<
			PLOT_NAME_FIELD << " TEXT, " <<
			YEAR_OUT_FIELD << " INTEGER NOT NULL, " <<
			"BPS_MODEL TEXT NOT NULL, " <<
			"STAGE INTEGER, " <<
			"COHORT_TYPE TEXT, " <<
			"PLOT_AGE REAL);";

		char* sql = streamToCharPtr(&sqlstream);
		queuedWrites.push_back(sql);

		return RC;
	}

	int SuccessionDIO::write_output_record(const int year, const RVS::DataManagement::AnalysisPlot* ap)
	{
		if (!ap) return (RC ? *RC : -1);

		std::stringstream sqlstream;
		sqlstream << "INSERT INTO Succession_Output (" <<
			PLOT_NUM_FIELD << ", " <<
			PLOT_NAME_FIELD << ", " <<
			YEAR_OUT_FIELD << ", " <<
			"BPS_MODEL, " <<
			"STAGE, " <<
			"COHORT_TYPE, " <<
			"PLOT_AGE) " <<
			"VALUES (" <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->PLOT_ID() << ",\"" <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->PLOT_NAME() << "\"," <<
			year << ",\"" <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->BPS_MODEL_NUM() << "\"," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->CURRENT_SUCCESSION_STAGE() << ",\"" <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->CURRENT_STAGE_TYPE() << "\"," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->PLOT_AGE() << ");";

		char* sql = streamToCharPtr(&sqlstream);
		queuedWrites.push_back(sql);

		return (RC ? *RC : 0);
	}

	int* SuccessionDIO::write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap)
	{
		const int y = (year ? *year : 0);
		const int res = write_output_record(y, ap);
		(void)res;
		return RC;
	}

	int* SuccessionDIO::create_intermediate_table()
	{
		return RC;
	}

	int* SuccessionDIO::write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* record)
	{
		return RC;
	}

	RVS::DataManagement::DataTable* SuccessionDIO::query_succession_table(const std::string& bps_model_code)
	{
		const char* sql = query_base(SUCCESSION_TABLE, "BPS_MODEL", bps_model_code, "COHORT");
		RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
		return dt;
	}

	bool SuccessionDIO::get_succession_data(
		const std::string& bps_model_code,
		std::map<std::string, std::string>& stringVals,
		std::map<std::string, double>& numVals,
		bool& doNotModel)
	{
		RVS::DataManagement::DataTable* succDt = query_succession_table(bps_model_code);
		if (!succDt || *(succDt->STATUS()) != SQLITE_ROW)
		{
			doNotModel = true;
			return true;
		}

		double cohort = 0.0;
		getVal(succDt->getStmt(), succDt->Columns["COHORT"], &cohort);
		double startAge = 0.0;
		double endAge = 0.0;
		getVal(succDt->getStmt(), succDt->Columns["StartAge"], &startAge);
		getVal(succDt->getStmt(), succDt->Columns["EndAge"], &endAge);
		double midpoint = 0.0;
		double gr_ht = 0.0;
		double gr_cov = 0.0;
		double max_ht = 0.0;
		double max_cov = 0.0;
		double min_ht = 0.0;
		double min_cov = 0.0;

		std::string str_cohort;
		std::string cohort_type;
		std::string cover_type;
		std::string species1;
		std::string species2;
		std::string species3;
		std::string species4;

		getVal(succDt->getStmt(), succDt->Columns["MidPoint_cov"], &midpoint);
		getVal(succDt->getStmt(), succDt->Columns["GR_HT_m_yr"], &gr_ht);
		getVal(succDt->getStmt(), succDt->Columns["GR_CC_yr"], &gr_cov);
		getVal(succDt->getStmt(), succDt->Columns["Max_HT_m"], &max_ht);
		getVal(succDt->getStmt(), succDt->Columns["Max_CC"], &max_cov);
		getVal(succDt->getStmt(), succDt->Columns["Min_HT_m"], &min_ht);
		getVal(succDt->getStmt(), succDt->Columns["Min_CC"], &min_cov);

		getVal(succDt->getStmt(), succDt->Columns["COHORT"], &str_cohort);
		getVal(succDt->getStmt(), succDt->Columns["COHORT_TYPE"], &cohort_type);
		getVal(succDt->getStmt(), succDt->Columns["COVER_TYPE"], &cover_type);
		getVal(succDt->getStmt(), succDt->Columns["SPECIES1"], &species1);
		getVal(succDt->getStmt(), succDt->Columns["SPECIES2"], &species2);
		getVal(succDt->getStmt(), succDt->Columns["SPECIES3"], &species3);
		getVal(succDt->getStmt(), succDt->Columns["SPECIES4"], &species4);

		stringVals["cohort"] = str_cohort;
		stringVals["cohort_type"] = cohort_type;
		stringVals["cover_type"] = cover_type;
		stringVals["species1"] = species1;
		stringVals["species2"] = species2;
		stringVals["species3"] = species3;
		stringVals["species4"] = species4;

		numVals["cohort"] = cohort;
		numVals["startAge"] = startAge;
		numVals["endAge"] = endAge;
		numVals["midpoint_cov"] = midpoint;
		numVals["gr_ht"] = gr_ht;
		numVals["gr_cov"] = gr_cov;
		numVals["max_ht"] = max_ht;
		numVals["max_cov"] = max_cov;
		numVals["min_ht"] = min_ht;
		numVals["min_cov"] = min_cov;

		sqlite3_step(succDt->getStmt());
		return (*(succDt->STATUS()) != SQLITE_ROW);
	}

	bool SuccessionDIO::check_shrub_data_exists(const std::string& spp_code)
	{
		if (spp_code.empty())
		{
			return false;
		}
		const char* sql = query_base(PLANTS_TABLE, "Plants_Code", spp_code);
		return (prep_datatable(sql, rvsdb) != nullptr);
	}

	bool SuccessionDIO::check_code_is_shrub(const std::string& spp_code)
	{
		if (spp_code.empty()) return false;
		const char* sql = query_base(PLANTS_TABLE, "Plants_Code", spp_code);
		RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
		if (!dt) return false;

		std::string plantType;
		getVal(dt->getStmt(), dt->Columns[LIFEFORM_FIELD], &plantType);
		std::transform(plantType.begin(), plantType.end(), plantType.begin(), ::tolower);
		return (plantType.find("shrub") != std::string::npos);
	}

	std::string SuccessionDIO::get_scientific_name(const std::string& spp_code)
	{
		if (spp_code.empty()) return "";
		const char* sql = query_base(PLANTS_TABLE, "Plants_Code", spp_code);
		RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
		if (!dt) return "";

		std::string dom_spp;
		getVal(dt->getStmt(), dt->Columns[DOM_SPP_FIELD], &dom_spp);
		return dom_spp;
	}

	void SuccessionDIO::query_herb_growth_coefs(const std::string& bps_model, double& cov_rate, double& ht_rate)
	{
		const char* sql = query_base(HERB_GROWTH_TABLE, BPS_MODEL_FIELD, bps_model);
		cov_rate = 0.0;
		ht_rate = 0.0;

		RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb, true);
		if (dt && *(dt->STATUS()) == SQLITE_ROW)
		{
			getVal(dt->getStmt(), dt->Columns[HERB_CC_GROWTH_FIELD], &cov_rate);
			getVal(dt->getStmt(), dt->Columns[HERB_HT_GROWTH_FIELD], &ht_rate);
		}
		else
		{
			std::stringstream s;
			s << "Herb growth not found for BPS " << bps_model << ", using defaults";
			const char* c = streamToCharPtr(&s);
			write_debug_msg(c);

			cov_rate = 0.020334387;
			ht_rate = 0.000419084;
		}
	}

	std::vector<std::vector<double>> SuccessionDIO::query_covariance_matrix()
	{
		const char* sql = query_base(COVARIANCE_TABLE);
		RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
		if (!dt)
		{
			return {};
		}

		const int colNum = dt->numCols();
		std::vector<std::vector<double>> matrix(colNum, std::vector<double>(colNum, 0.0));

		for (int row = 0; row < colNum; ++row)
		{
			for (int col = 0; col < colNum; ++col)
			{
				double val = 0.0;
				getVal(dt->getStmt(), col, &val);
				matrix[row][col] = val;
			}
			sqlite3_step(dt->getStmt());
		}

		return matrix;
	}
}
}
