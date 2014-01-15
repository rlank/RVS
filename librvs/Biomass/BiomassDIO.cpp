#include "BiomassDIO.h"

RVS::Biomass::BiomassDIO::BiomassDIO(void) : RVS::DataManagement::DIO()
{

}

RVS::Biomass::BiomassDIO::~BiomassDIO(void)
{

}

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_crosswalk_table(std::string spp)
{
	sqlite3_stmt* stmt = RVS::DataManagement::DIO::query_base(BIOMASS_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}

int RVS::Biomass::BiomassDIO::query_biomass_crosswalk_table(std::string spp, std::string returnType)
{
	sqlite3_stmt* stmt = RVS::Biomass::BiomassDIO::query_base(BIOMASS_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
	int colNum = sqlite3_column_count(stmt);
	int rc = sqlite3_step(stmt);
	int ret = 0;
	if (rc)
	{
		for (int i = 0; i < colNum; i++)
		{
			if (strcmp(sqlite3_column_name(stmt, i), returnType.c_str()) == 0)
			{
				ret = sqlite3_column_int(stmt, i);
			}
		}
	}
	rc = sqlite3_finalize(stmt);
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
	RVS::DataManagement::DataTable* dt = query_biomass_equation_table(equation_number);
	int colCount = sqlite3_column_count(dt->getStmt());
	int rc = sqlite3_step(dt->getStmt());
	if (rc)
	{
		for (int i = 0; i < colCount; i++)
		{
			const char* colName = sqlite3_column_name(dt->getStmt(), i);
			if (strcmp(colName, "CF1") == 0)
			{
				*cf1 = sqlite3_column_double(dt->getStmt(), i);
			}
			else if (strcmp(colName, "CF2") == 0)
			{
				*cf2 = sqlite3_column_double(dt->getStmt(), i);
			}
			else if (strcmp(colName, "CF3") == 0)
			{
				*cf3 = sqlite3_column_double(dt->getStmt(), i);
			}
			else if (strcmp(colName, "CF4") == 0)
			{
				*cf4 = sqlite3_column_double(dt->getStmt(), i);
			}
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

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_biomass_input_table(int plot_num)
{
	sqlite3_stmt* stmt = RVS::DataManagement::DIO::query_base(BIOMASS_INPUT_TABLE, PLOT_NUM_FIELD, plot_num);
	DataManagement::DataTable* dt = new DataManagement::DataTable(stmt);
	return dt;
}