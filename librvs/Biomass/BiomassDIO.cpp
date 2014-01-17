#include "BiomassDIO.h"

RVS::Biomass::BiomassDIO::BiomassDIO(void) : RVS::DataManagement::DIO()
{
	this->create_biomass_output_table();
	this->create_biomass_intermediate_table();
}

RVS::Biomass::BiomassDIO::~BiomassDIO(void)
{

}

int* RVS::Biomass::BiomassDIO::create_biomass_output_table()
{
	std::stringstream sqlstream;
	sqlstream << "CREATE TABLE " << BIOMASS_OUTPUT_TABLE << "(" << \
		PLOT_NUM_FIELD << " INT PRIMARY KEY NOT NULL," << \
		EVT_NUM_FIELD << " INT NOT NULL," << \
		BPS_NUM_FIELD << " CHAR(12) NOT NULL," << \
		RET_CODE_FIELD << " CHAR(12) NOT NULL," << \
		BIOMASS_SHRUB_OUT_FIELD << " REAL," << \
		BIOMASS_HERB_OUT_FIELD << " REAL," << \
		BIOMASS_TOTAL_OUT_FIELD << " REAL NOT NULL);";

	char* sql = streamToCharPtr(&sqlstream);

	RC = RVS::DataManagement::DIO::create_table(sql);

	return RC;
}

int* RVS::Biomass::BiomassDIO::write_biomass_output_record(int* plot_num, int* evt_num, std::string* bps, char* ret_code, double* totalBiomass, double* herbBiomass, double* shrubBiomass)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << BIOMASS_OUTPUT_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << EVT_NUM_FIELD << ", " << BPS_NUM_FIELD << ", " << \
		RET_CODE_FIELD << ", " << BIOMASS_TOTAL_OUT_FIELD << ", " << \
		BIOMASS_SHRUB_OUT_FIELD << ", " << BIOMASS_HERB_OUT_FIELD << ") " << \
		"VALUES (" << *plot_num << "," << *evt_num << ",\"" << *bps << "\",\"" << \
		ret_code << "\"," << *totalBiomass << "," << *shrubBiomass << "," << \
		*herbBiomass << ");";

	char* sql = streamToCharPtr(&sqlstream);
	RC = RVS::DataManagement::DIO::exec_sql(sql);
	return RC;
}

int* RVS::Biomass::BiomassDIO::create_biomass_intermediate_table()
{
	std::stringstream sqlstream;
	sqlstream << "CREATE TABLE " << BIOMASS_INTERMEDIATE_TABLE << " (" << \
		PLOT_NUM_FIELD << " int NOT NULL," << \
		EVT_NUM_FIELD << " int NOT NULL," << \
		BPS_NUM_FIELD << " char(12) NOT NULL," << \
		DOM_SPP_FIELD << " char(255)," << \
		SPP_CODE_FIELD << " char(255)," << \
		RET_CODE_FIELD << " char(12)," << \
		BIOMASS_SHRUB_OUT_FIELD << " REAL);";

	char* sql = streamToCharPtr(&sqlstream);

	RC = RVS::DataManagement::DIO::create_table(sql);

	return RC;
}

int* RVS::Biomass::BiomassDIO::write_biomass_intermediate_record(RVS::Biomass::BiomassEVT* bioEVT, int* plot_num, double* shrubBiomass)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << BIOMASS_INTERMEDIATE_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << EVT_NUM_FIELD << ", " << BPS_NUM_FIELD << " ," << \
		DOM_SPP_FIELD << ", " << SPP_CODE_FIELD << ", " << \
		RET_CODE_FIELD << ", " << BIOMASS_SHRUB_OUT_FIELD << ") " << \
		"VALUES (" << *plot_num << "," << bioEVT->EVT_NUM() << ",\"" << bioEVT->BPS_NUM() << "\",\"" << \
		bioEVT->DOM_SPP() << "\",\"" << bioEVT->SPP_CODE() << "\",\"" << \
		ENUMPARSE(bioEVT->RETURN_TYPE()) << "\"," << *shrubBiomass << ");";

	char* sql = streamToCharPtr(&sqlstream);
	RC = RVS::DataManagement::DIO::exec_sql(sql);
	return RC;
}


RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_crosswalk_table(std::string spp)
{
	sqlite3_stmt* stmt = RVS::DataManagement::DIO::query_base(BIOMASS_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}

int RVS::Biomass::BiomassDIO::query_biomass_crosswalk_table(std::string spp, std::string returnType)
{
	// Create the sqlite3 statment to query biomass crosswalk table on species
	sqlite3_stmt* stmt = RVS::Biomass::BiomassDIO::query_base(BIOMASS_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
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
	sqlite3_stmt* stmt = RVS::DataManagement::DIO::query_base(BIOMASS_EQUATION_TABLE, SPP_CODE_FIELD, spp_code.c_str());
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_equation_table(int equation_number)
{
	sqlite3_stmt* stmt = RVS::DataManagement::DIO::query_base(BIOMASS_EQUATION_TABLE, EQUATION_NUMBER_FIELD, equation_number);
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
			*cf1 = sqlite3_column_double(dt->getStmt(), i);
		}
		else if (strcmp(colName, BIOMASS_EQN_COEF_2_FIELD) == 0)
		{
			*cf2 = sqlite3_column_double(dt->getStmt(), i);
		}
		else if (strcmp(colName, BIOMASS_EQN_COEF_3_FIELD) == 0)
		{
			*cf3 = sqlite3_column_double(dt->getStmt(), i);
		}
		else if (strcmp(colName, BIOMASS_EQN_COEF_4_FIELD) == 0)
		{
			*cf4 = sqlite3_column_double(dt->getStmt(), i);
		}
	}

	delete dt;
}

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_herbs_table(int baseBPS)
{
	sqlite3_stmt* stmt = RVS::DataManagement::DIO::query_base(BIOMASS_HERBS_TABLE, BPS_NUM_FIELD, baseBPS);
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}

double RVS::Biomass::BiomassDIO::query_biomass_herbs_table(int baseBPS, char* level)
{
	// Get the datatable for the herb biomass
	RVS::DataManagement::DataTable* dt = RVS::Biomass::BiomassDIO::query_biomass_herbs_table(baseBPS);

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
	sqlite3_stmt* stmt = RVS::DataManagement::DIO::query_base(BIOMASS_INPUT_TABLE, PLOT_NUM_FIELD, plot_num);
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}