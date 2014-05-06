#include "BiomassDIO.h"

RVS::Biomass::BiomassDIO::BiomassDIO(void) : RVS::DataManagement::DIO()
{
	this->create_output_table();
	this->create_intermediate_table();
}

RVS::Biomass::BiomassDIO::~BiomassDIO(void)
{

}

int* RVS::Biomass::BiomassDIO::create_output_table()
{
	std::stringstream sqlstream;
	sqlstream << "CREATE TABLE " << BIOMASS_OUTPUT_TABLE << "(" << \
		PLOT_NUM_FIELD << " INT NOT NULL," << \
		YEAR_OUT_FIELD << " INT NOT NULL," << \
		EVT_NUM_FIELD << " INT NOT NULL," << \
		BPS_NUM_FIELD << " CHAR(12) NOT NULL," << \
		RET_CODE_FIELD << " CHAR(12) NOT NULL," << \
		BIOMASS_SHRUB_OUT_FIELD << " REAL," << \
		BIOMASS_HERB_OUT_FIELD << " REAL," << \
		BIOMASS_TOTAL_OUT_FIELD << " REAL NOT NULL);";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	RC = create_table(sql);
	delete[] sql;

	return RC;
}

int* RVS::Biomass::BiomassDIO::write_biomass_output_record(int* plot_num, int* year, int* evt_num, std::string* bps, char* ret_code, double* totalBiomass, double* herbBiomass, double* shrubBiomass)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << BIOMASS_OUTPUT_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << YEAR_OUT_FIELD << ", " << EVT_NUM_FIELD << ", " << BPS_NUM_FIELD << ", " << \
		RET_CODE_FIELD << ", " << BIOMASS_TOTAL_OUT_FIELD << ", " << \
		BIOMASS_SHRUB_OUT_FIELD << ", " << BIOMASS_HERB_OUT_FIELD << ") " << \
		"VALUES (" << *plot_num << "," << *year << "," << *evt_num << ",\"" << *bps << "\",\"" << \
		ret_code << "\"," << *totalBiomass << "," << *shrubBiomass << "," << \
		*herbBiomass << ");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	RC = exec_sql(sql);
	delete[] sql;
	return RC;
}

int* RVS::Biomass::BiomassDIO::create_intermediate_table()
{
	std::stringstream sqlstream;
	sqlstream << "CREATE TABLE " << BIOMASS_INTERMEDIATE_TABLE << " (" << \
		PLOT_NUM_FIELD << " int NOT NULL," << \
		YEAR_OUT_FIELD << " int NOT NULL," << \
		EVT_NUM_FIELD << " int NOT NULL," << \
		BPS_NUM_FIELD << " char(12) NOT NULL," << \
		DOM_SPP_FIELD << " char(255)," << \
		SPP_CODE_FIELD << " char(255)," << \
		RET_CODE_FIELD << " char(12)," << \
		BIOMASS_SHRUB_OUT_FIELD << " REAL);";

	char* sql = new char; 
	sql = streamToCharPtr(&sqlstream);
	RC = create_table(sql);
	delete[] sql;
	return RC;
}

int* RVS::Biomass::BiomassDIO::write_biomass_intermediate_record(RVS::Biomass::BiomassEVT* bioEVT, int* year, int* plot_num, double* shrubBiomass)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << BIOMASS_INTERMEDIATE_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << YEAR_OUT_FIELD << ", " << EVT_NUM_FIELD << ", " << BPS_NUM_FIELD << " ," << \
		DOM_SPP_FIELD << ", " << SPP_CODE_FIELD << ", " << \
		RET_CODE_FIELD << ", " << BIOMASS_SHRUB_OUT_FIELD << ") " << \
		"VALUES (" << *plot_num << "," << *year << "," << bioEVT->EVT_NUM() << ",\"" << bioEVT->BPS_NUM() << "\",\"" << \
		bioEVT->DOM_SPP() << "\",\"" << bioEVT->SPP_CODE() << "\",\"" << \
		ENUMPARSE(bioEVT->RETURN_TYPE()) << "\"," << *shrubBiomass << ");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	RC = RVS::DataManagement::DIO::exec_sql(sql);
	delete[] sql;
	return RC;
}


RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_crosswalk_table(std::string spp)
{
	sqlite3_stmt* stmt = query_base(BIOMASS_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}

int RVS::Biomass::BiomassDIO::query_biomass_crosswalk_table(std::string spp, std::string returnType)
{
	// Create the sqlite3 statment to query biomass crosswalk table on species
	sqlite3_stmt* stmt = query_base(BIOMASS_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
	int colNum = sqlite3_column_count(stmt);
	// Initiate the query
	*RC = sqlite3_step(stmt);
	// Step over the columns, looking for the requested return type
	int ret = 0;
	for (int i = 0; i < colNum; i++)
	{
		// If the column name matches the return type, return the equation number found there
		if (strcmp(sqlite3_column_name(stmt, i), returnType.c_str()) == 0)
		{
			ret = sqlite3_column_int(stmt, i);
		}
	}
	// Cleanup
	*RC = sqlite3_finalize(stmt);
	return ret;
}

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_equation_table(std::string spp_code)
{
	sqlite3_stmt* stmt = query_base(BIOMASS_EQUATION_TABLE, SPP_CODE_FIELD, spp_code.c_str());
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_equation_table(int equation_number)
{
	sqlite3_stmt* stmt = query_base(BIOMASS_EQUATION_TABLE, EQUATION_NUMBER_FIELD, equation_number);
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}

void RVS::Biomass::BiomassDIO::query_biomass_equation_coefficients(int equation_number, float* cf1, float* cf2, float* cf3, float* cf4)
{
	// Get the datatable object for the requested equation number
	RVS::DataManagement::DataTable* dt = query_biomass_equation_table(equation_number);
	int colCount = sqlite3_column_count(dt->getStmt());
	// Initiate the query
	*RC = sqlite3_step(dt->getStmt());
	// Move over the columns, putting the coefficient return values into their
	// appropriate bin
	for (int i = 0; i < colCount; i++)
	{
		const char* colName = sqlite3_column_name(dt->getStmt(), i);
		if (strcmp(colName, BIOMASS_EQN_COEF_1_FIELD) == 0)
		{
			*cf1 = (float)sqlite3_column_double(dt->getStmt(), i);
		}
		else if (strcmp(colName, BIOMASS_EQN_COEF_2_FIELD) == 0)
		{
			*cf2 = (float)sqlite3_column_double(dt->getStmt(), i);
		}
		else if (strcmp(colName, BIOMASS_EQN_COEF_3_FIELD) == 0)
		{
			*cf3 = (float)sqlite3_column_double(dt->getStmt(), i);
		}
		else if (strcmp(colName, BIOMASS_EQN_COEF_4_FIELD) == 0)
		{
			*cf4 = (float)sqlite3_column_double(dt->getStmt(), i);
		}
	}

	delete dt;
}

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_herbs_table(int baseBPS)
{
	sqlite3_stmt* stmt = query_base(BIOMASS_HERBS_TABLE, BPS_NUM_FIELD, baseBPS);
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}

double RVS::Biomass::BiomassDIO::query_biomass_herbs_table(int baseBPS, char* level)
{
	// Get the datatable for the herb biomass
	RVS::DataManagement::DataTable* dt = query_biomass_herbs_table(baseBPS);

	double biomass = 0;

	int colCount = sqlite3_column_count(dt->getStmt());
	*RC = sqlite3_step(dt->getStmt());

	// Step through the returned record set. Match the column name to the requested level
	for (int i = 0; i < colCount; i++)
	{
		// If the column name matches the requested return level, get the value
		// and stop iteration.
		const char* colName = sqlite3_column_name(dt->getStmt(), i);
		if (strcmp(colName, level) == 0)
		{
			biomass = sqlite3_column_int(dt->getStmt(), i);
			break;
		}
	}

	delete dt;
	return biomass;
}

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_input_table(int plot_num)
{
	sqlite3_stmt* stmt = query_base(BIOMASS_INPUT_TABLE, PLOT_NUM_FIELD, plot_num);
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}

void RVS::Biomass::BiomassDIO::query_biogroup_coefs(std::string bps, double* group_const, double* ndvi_grp_interact, double* ppt_grp_interact)
{
	sqlite3_stmt* stmt1 = query_base(BIOMASS_MACROGROUP_TABLE, BPS_NUM_FIELD, bps.substr(2));

	int colCount = sqlite3_column_count(stmt1);
	*RC = sqlite3_step(stmt1);
	char* groupID = NULL;

	for (int i = 0; i < colCount; i++)
	{
		const char* colName = sqlite3_column_name(stmt1, i);
		if (strcmp(colName, GROUP_ID_FIELD) == 0)
		{
			groupID = (char*)sqlite3_column_text(stmt1, i);
		}
	}

	if (groupID == NULL)
	{
		//$$ Throw new DATANOTFOUND error here
	}

	
	std::stringstream sqlstream;
	sqlstream << "SELECT * FROM " << BIOMASS_GROUP_COEFS_TABLE << \
		" WHERE " << GROUP_ID_FIELD << "='" << groupID << "';";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	sqlite3_stmt* stmt2 = query_base(sql);
	*RC = sqlite3_step(stmt2);
	colCount = sqlite3_column_count(stmt2);

	for (int i = 0; i < colCount; i++)
	{
		const char* colName = sqlite3_column_name(stmt2, i);
		if (strcmp(colName, GROUP_CONST_FIELD) == 0)
		{
			double GC = sqlite3_column_double(stmt2, i);
			*group_const = sqlite3_column_double(stmt2, i);
		}
		else if (strcmp(colName, NDVI_INTERACT_FIELD) == 0)
		{
			*ndvi_grp_interact = sqlite3_column_double(stmt2, i);
		}
		else if (strcmp(colName, PRCP_INTERACT_FIELD) == 0)
		{
			*ppt_grp_interact = sqlite3_column_double(stmt2, i);
		}
	}

	delete[] sql;
	*RC = sqlite3_finalize(stmt1);
	*RC = sqlite3_finalize(stmt2);
}