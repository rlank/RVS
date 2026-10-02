#include "FuelsDIO.h"

namespace RVS
{
namespace Fuels
{
	FuelsDIO::FuelsDIO() : RVS::DataManagement::DIO()
	{
		this->create_output_table();
		this->create_intermediate_table();
	}

	int* FuelsDIO::create_output_table()
	{
		std::stringstream sqlstream;
		sqlstream << "CREATE TABLE IF NOT EXISTS " << FUELS_OUTPUT_TABLE << "(" <<
			PLOT_NUM_FIELD << " INT NOT NULL," <<
			PLOT_NAME_FIELD << " TEXT, " <<
			YEAR_OUT_FIELD << " INT NOT NULL," <<
			BPS_NUM_FIELD << " INT NOT NULL," <<
			BPS_MODEL_FIELD << " TEXT NOT NULL," <<
			FC_ISDRY_FIELD << " BOOLEAN, " <<
			AVG_SHRUB_HEIGHT_FIELD << " REAL," <<
			TOT_SHRUB_COVER_FIELD << " REAL," <<
			HERB_HEIGHT_FIELD << " REAL," <<
			HERB_COVER_FIELD << " REAL," <<
			FUEL_1HR_SHRUB_WB << " REAL, " <<
			FUEL_1HR_SHRUB_FOLIAGE << " REAL, " <<
			FULE_1HR_HERB << " REAL, " <<
			FUEL_1HR_TOTAL << " REAL, " <<
			FUEL_10HR_FIELD << " REAL, " <<
			FUEL_100HR_FIELD << " REAL, " <<
			FUEL_1000HR_FIELD << " REAL, " <<
			FUEL_TOTAL_FIELD << " REAL, " <<
			FC_FBFM_FIELD << " TEXT); ";

		queuedWrites.push_back(sqlstream.str());

		return RC;
	}

	int FuelsDIO::write_output_record(const int year, const RVS::DataManagement::AnalysisPlot* ap)
	{
		if (!ap)
		{
			return (RC ? *RC : -1);
		}

		std::stringstream sqlstream;
		sqlstream << "INSERT INTO " << FUELS_OUTPUT_TABLE << " (" <<
			PLOT_NUM_FIELD << ", " <<
			PLOT_NAME_FIELD << ", " <<
			YEAR_OUT_FIELD << ", " <<
			BPS_NUM_FIELD << ", " <<
			BPS_MODEL_FIELD << ", " <<
			FC_ISDRY_FIELD << ", " <<
			AVG_SHRUB_HEIGHT_FIELD << "," <<
			TOT_SHRUB_COVER_FIELD << "," <<
			HERB_HEIGHT_FIELD << "," <<
			HERB_COVER_FIELD << "," <<
			FUEL_1HR_SHRUB_WB << ", " <<
			FUEL_1HR_SHRUB_FOLIAGE << ", " <<
			FULE_1HR_HERB << ", " <<
			FUEL_1HR_TOTAL << ", " <<
			FUEL_10HR_FIELD << ", " <<
			FUEL_100HR_FIELD << ", " <<
			FUEL_1000HR_FIELD << ", " <<
			FUEL_TOTAL_FIELD << ", " <<
			FC_FBFM_FIELD << ") " <<
			"VALUES (" <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->PLOT_ID() << ",\"" <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->PLOT_NAME() << "\"," <<
			year << "," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->BPS_NUM() << ",\"" <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->BPS_MODEL_NUM() << "\"," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->ISDRY() << ", " <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->SHRUBHEIGHT() << ", " <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->SHRUBCOVER() << ", " <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->HERBHEIGHT() << ", " <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->HERBCOVER() << ", " <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->SHRUB_1HR_WB() << "," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->SHRUB_1HR_FOLIAGE() << "," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->HERB_FUEL() << "," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->FUEL_TOTAL_1HR() << "," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->SHRUB_10HR() << "," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->SHRUB_100HR() << "," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->SHRUB_1000HR() << "," <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->FUEL_TOTAL() << ", \"" <<
			const_cast<RVS::DataManagement::AnalysisPlot*>(ap)->FBFM_NAME() << "\");";

		queuedWrites.push_back(sqlstream.str());

		return (RC ? *RC : 0);
	}

	int* FuelsDIO::write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap)
	{
		const int y = (year ? *year : 0);
		static_cast<void>(write_output_record(y, ap));
		return RC;
	}

	int* FuelsDIO::create_intermediate_table()
	{
		return RC;
	}

	int* FuelsDIO::write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp)
	{
		return RC;
	}

	std::map<std::string, int> FuelsDIO::query_crosswalk_table(const std::string& spp)
	{
		std::map<std::string, int> equationNumbers;
		const char* const sql = query_base(FUEL_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
		RVS::DataManagement::DataTable* const dt = prep_datatable(sql, rvsdb);
		if (!dt) return equationNumbers;

		const int colNum = dt->numCols();
		int val = 0;
		for (int i = 0; i < colNum; i++)
		{
			const char* const colName = sqlite3_column_name(dt->getStmt(), i);
			if (colName && colName[0] == 'F')
			{
				getVal(dt->getStmt(), i, &val);
				equationNumbers[std::string(colName)] = val;
			}
		}

		return equationNumbers;
	}

	RVS::DataManagement::DataTable* FuelsDIO::query_equation_table(const std::map<std::string, int>& equationNumbers)
	{
		std::stringstream sqlstream;
		sqlstream << "SELECT * FROM " << FUEL_EQUATION_TABLE << " WHERE " << EQUATION_NUMBER_FIELD << " IN (";

		size_t count = 0;
		for (const auto& eq : equationNumbers)
		{
			sqlstream << eq.second;
			if (count < equationNumbers.size() - 1)
			{
				sqlstream << ",";
			}
			else
			{
				sqlstream << ")";
			}
			count++;
		}

		const char* const sql = streamToCharPtr(&sqlstream);
		return prep_datatable(sql, rvsdb);
	}

	RVS::DataManagement::DataTable* FuelsDIO::query_equation_table(const int equationNumber)
	{
		const char* const sql = query_base(FUEL_EQUATION_TABLE, EQUATION_NUMBER_FIELD, equationNumber);
		return prep_datatable(sql, rvsdb);
	}

	RVS::DataManagement::DataTable* FuelsDIO::query_fbfm_rules_selector()
	{
		std::stringstream ss;
		ss << "SELECT * FROM " << FUEL_CLASSRULES_TABLE << ";";
		const char* const sql = streamToCharPtr(&ss);
		return prep_datatable(sql, rvsdb);
	}

	RVS::DataManagement::DataTable* FuelsDIO::query_fbfm_rules(std::string_view classTable)
	{
		std::stringstream ss;
		ss << "SELECT * FROM " << classTable << ";";
		const char* const sql = streamToCharPtr(&ss);
		return prep_datatable(sql, rvsdb);
	}
}
}
