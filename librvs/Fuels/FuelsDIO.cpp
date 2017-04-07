#include "FuelsDIO.h"

RVS::Fuels::FuelsDIO::FuelsDIO(void) : RVS::DataManagement::DIO()
{
	this->create_output_table();
	this->create_intermediate_table();
}

RVS::Fuels::FuelsDIO::~FuelsDIO(void)
{

}

int* RVS::Fuels::FuelsDIO::create_output_table()
{
	std::stringstream sqlstream;
	sqlstream << "CREATE TABLE " << FUELS_OUTPUT_TABLE << "(" << \
		PLOT_NUM_FIELD << " INT NOT NULL," << \
		PLOT_NAME_FIELD << " TEXT, " << \
		YEAR_OUT_FIELD << " INT NOT NULL," << \
		BPS_NUM_FIELD << " INT NOT NULL," << \
		BPS_MODEL_FIELD << " TEXT NOT NULL," << \
		FC_ISDRY_FIELD << " BOOLEAN, " << \
		AVG_SHRUB_HEIGHT_FIELD << " REAL," << \
		TOT_SHRUB_COVER_FIELD << " REAL," << \
		HERB_HEIGHT_FIELD << " REAL," << \
		HERB_COVER_FIELD << " REAL," << \
		FUEL_1HR_SHRUB_WB << " REAL, " << \
		FUEL_1HR_SHRUB_FOLIAGE << " REAL, " << \
		FULE_1HR_HERB << " REAL, " << \
		FUEL_1HR_TOTAL << " REAL, " << \
		FUEL_10HR_FIELD << " REAL, " << \
		FUEL_100HR_FIELD << " REAL, " << \
		FUEL_1000HR_FIELD << " REAL, " << \
		FUEL_TOTAL_FIELD << " REAL, " << \
		FC_FBFM_FIELD << " TEXT); ";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);

	return RC;
}

int* RVS::Fuels::FuelsDIO::write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap)
{
	std::stringstream sqlstream;
	
	sqlstream << "INSERT INTO " << FUELS_OUTPUT_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << \
		PLOT_NAME_FIELD << ", " << \
		YEAR_OUT_FIELD << ", " << \
		BPS_NUM_FIELD << ", " << \
		BPS_MODEL_FIELD << ", " << \
		FC_ISDRY_FIELD << ", " << \
		AVG_SHRUB_HEIGHT_FIELD << "," << \
		TOT_SHRUB_COVER_FIELD << "," << \
		HERB_HEIGHT_FIELD << "," << \
		HERB_COVER_FIELD << "," << \
		FUEL_1HR_SHRUB_WB << ", " << \
		FUEL_1HR_SHRUB_FOLIAGE << ", " << \
		FULE_1HR_HERB << ", " << \
		FUEL_1HR_TOTAL << ", " << \
		FUEL_10HR_FIELD << ", " << \
		FUEL_100HR_FIELD << ", " << \
		FUEL_1000HR_FIELD << ", " << \
		FUEL_TOTAL_FIELD << ", " << \
		FC_FBFM_FIELD << ") " << \
		"VALUES (" << \
		ap->PLOT_ID() << ",\"" << \
		ap->PLOT_NAME() << "\"," << \
		*year << "," << \
		ap->BPS_NUM() << "," << \
		ap->BPS_MODEL_NUM() << "," << \
		ap->ISDRY() << ", " << \
		ap->SHRUBHEIGHT() << ", " << \
		ap->SHRUBCOVER() << ", " << \
		ap->HERBHEIGHT() << ", " << \
		ap->HERBCOVER() << ", " << \
		ap->SHRUB_1HR_WB() << "," << \
		ap->SHRUB_1HR_FOLIAGE() << "," << \
		ap->HERB_FUEL() << "," << \
		ap->FUEL_TOTAL_1HR() << "," << \
		ap->SHRUB_10HR() << "," << \
		ap->SHRUB_100HR() << "," << \
		ap->SHRUB_1000HR() << "," << \
		ap->FUEL_TOTAL() << ", \"" << \
		ap->FBFM_NAME() << "\");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);
	
	return RC;
	
}

int* RVS::Fuels::FuelsDIO::create_intermediate_table()
{
	std::stringstream sqlstream;
	/*
	sqlstream << "CREATE TABLE " << FUELS_INTERMEDIATE_TABLE << "(" << \
		PLOT_NUM_FIELD << " INT NOT NULL, " << \
		PLOT_NAME_FIELD << " TEXT, " << \
		YEAR_OUT_FIELD << " INT NOT NULL, " << \
		BPS_NUM_FIELD << " INT NOT NULL, " << \
		FC_ISDRY_FIELD << " BOOLEAN, " << \
		DOM_SPP_FIELD << " TEXT, " << \
		SPP_CODE_FIELD << " TEXT, " << \
		FUEL_1HR_OUT_FIELD << "_EQ INT, " << \
		FUEL_1HR_OUT_FIELD << " REAL, " << \
		FUEL_10HR_OUT_FIELD << "_EQ INT, " << \
		FUEL_10HR_OUT_FIELD << " REAL, " << \
		FUEL_100HR_OUT_FIELD << "_EQ INT, " << \
		FUEL_100HR_OUT_FIELD << " REAL, " << \
		FUEL_1HRLIVE_OUT_FIELD << "_EQ INT, " << \
		FUEL_1HRLIVE_OUT_FIELD << " REAL, " << \
		FUEL_10HRLIVE_OUT_FIELD << "_EQ INT, " << \
		FUEL_10HRLIVE_OUT_FIELD << " REAL, " << \
		FUEL_100HRLIVE_OUT_FIELD << "_EQ INT, " << \
		FUEL_100HRLIVE_OUT_FIELD << " REAL, " << \
		FUEL_1HRDEAD_OUT_FIELD << "_EQ INT, " << \
		FUEL_1HRDEAD_OUT_FIELD << " REAL, " << \
		FUEL_10HRDEAD_OUT_FIELD << "_EQ INT, " << \
		FUEL_10HRDEAD_OUT_FIELD << " REAL, " << \
		FUEL_100HRDEAD_OUT_FIELD << "_EQ INT, " << \
		FUEL_100HRDEAD_OUT_FIELD << " REAL); ";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);
	*/
	return RC;
}


int* RVS::Fuels::FuelsDIO::write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp)
{
	/*
	std::map<std::string, int> fuelsEqs = spp->FUEL_EQUS();
	std::map<std::string, double> fuelsVals = spp->FUEL_VALUES();

	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << FUELS_INTERMEDIATE_TABLE << \
		" VALUES (" << \
		ap->PLOT_ID() << ",\"" << \
		ap->PLOT_NAME() << "\"," << \
		*year << "," << \
		ap->BPS_NUM() << "," << \
		ap->ISDRY() << ",\"" << \
		spp->DOM_SPP() << "\", \"" << \
		spp->SPP_CODE() << "\"," << \
		fuelsEqs[FUEL_1HR_OUT_FIELD] << "," << \
		fuelsVals[FUEL_1HR_OUT_FIELD] / 2000 << "," << \
		fuelsEqs[FUEL_10HR_OUT_FIELD] << "," << \
		fuelsVals[FUEL_10HR_OUT_FIELD] / 2000 << "," << \
		fuelsEqs[FUEL_100HR_OUT_FIELD] << "," << \
		fuelsVals[FUEL_100HR_OUT_FIELD] / 2000 << "," << \
		fuelsEqs[FUEL_1HRLIVE_OUT_FIELD] << "," << \
		fuelsVals[FUEL_1HRLIVE_OUT_FIELD] / 2000 << "," << \
		fuelsEqs[FUEL_10HRLIVE_OUT_FIELD] << "," << \
		fuelsVals[FUEL_10HRLIVE_OUT_FIELD] / 2000 << "," << \
		fuelsEqs[FUEL_100HRLIVE_OUT_FIELD] << "," << \
		fuelsVals[FUEL_100HRLIVE_OUT_FIELD] / 2000 << "," << \
		fuelsEqs[FUEL_1HRDEAD_OUT_FIELD] << "," << \
		fuelsVals[FUEL_1HRDEAD_OUT_FIELD] / 2000 << "," << \
		fuelsEqs[FUEL_10HRDEAD_OUT_FIELD] << "," << \
		fuelsVals[FUEL_10HRDEAD_OUT_FIELD] / 2000 << "," << \
		fuelsEqs[FUEL_100HRDEAD_OUT_FIELD] << "," << \
		fuelsVals[FUEL_100HRDEAD_OUT_FIELD] / 2000 << ");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);
	*/
	return RC;
}

std::map<std::string, int> RVS::Fuels::FuelsDIO::query_crosswalk_table(std::string spp)
{
	map<string, int> equationNumbers = map<string, int>();
	// Create the sqlite3 statment to query biomass crosswalk table on species
	const char* sql = query_base(FUEL_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);

	int colNum = dt->numCols();
	// Step over the columns, looking for the requested return type
	int val = 0;
	for (int i = 0; i < colNum; i++)
	{
		const char* colName = sqlite3_column_name(dt->getStmt(), i);
		string colStr = string(colName);
		// If the column name matches the return type, return the equation number found there
		if (colName[0] == 'F')
		{
			getVal(dt->getStmt(), i, &val);
			equationNumbers[colStr] = val;
		}
	}

	return equationNumbers;
}

RVS::DataManagement::DataTable* RVS::Fuels::FuelsDIO::query_equation_table(std::map<std::string, int> equationNumbers)
{
	std::stringstream sqlstream;
	sqlstream << "SELECT * FROM " << FUEL_EQUATION_TABLE << " WHERE " << EQUATION_NUMBER_FIELD << " IN (";

	int count = 0;
	for (auto eq : equationNumbers)
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

	char* sql = streamToCharPtr(&sqlstream);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	return dt;
}

RVS::DataManagement::DataTable* RVS::Fuels::FuelsDIO::query_equation_table(int equationNumber)
{
	const char* sql = query_base(FUEL_EQUATION_TABLE, EQUATION_NUMBER_FIELD, equationNumber);
	DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	return dt;
}

RVS::DataManagement::DataTable* RVS::Fuels::FuelsDIO::query_fbfm_rules_selector(void)
{
	std::stringstream ss;
	ss << "SELECT * FROM " << FUEL_CLASSRULES_TABLE << ";";
	char* sql = streamToCharPtr(&ss);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	return dt;
}

RVS::DataManagement::DataTable* RVS::Fuels::FuelsDIO::query_fbfm_rules(std::string classTable)
{
	std::stringstream ss;
	ss << "SELECT * FROM " << classTable << ";";
	char* sql = streamToCharPtr(&ss);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	return dt;
}