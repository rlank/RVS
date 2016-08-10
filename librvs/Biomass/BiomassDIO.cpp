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
		PLOT_NUM_FIELD << " INTEGER NOT NULL, " << \
		PLOT_NAME_FIELD << " TEXT, " << \
		YEAR_OUT_FIELD << " INTEGER NOT NULL, " << \
		BPS_NUM_FIELD << " INTEGER NOT NULL, " << \
		BPS_MODEL_FIELD << " TEXT, " << \
		GROUP_ID_FIELD << " TEXT, " << \
		"NDVI REAL, " << "PPT REAL, " << \
		BIOMASS_SHRUB_OUT_FIELD << " REAL, " << \
		BIOMASS_HERB_OUT_FIELD << " REAL, " << \
		BIOMASS_RAW_PRODUCTION_FIELD << " REAL," << \
		BIOMASS_HERB_PP_FIELD << " REAL, " << \
		BIOMASS_HERB_HOLDOVER_FIELD << " REAL, " << \
		BIOMASS_TOTAL_OUT_FIELD << " REAL, " << \
		HERB_COVER_FIELD << " REAL, " << \
		HERB_HEIGHT_FIELD << " REAL, " << \
		AVG_SHRUB_HEIGHT_FIELD << " REAL, " << \
		TOT_SHRUB_COVER_FIELD << " REAL, " << \
		LOWER_BOUND_FIELD << " REAL, " << \
		UPPER_BOUND_FIELD << " REAL, " << \
		"range" << " REAL, " << \
		S2Y_FIELD << " REAL, " << \
		DISTURBANCE_AMOUNT_FIELD << " REAL, " << \
		LATITUDE_FIELD << " FLOAT, " << \
		LONGITUDE_FIELD << " FLOAT);";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);

	return RC;
}

int* RVS::Biomass::BiomassDIO::write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << BIOMASS_OUTPUT_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << \
		PLOT_NAME_FIELD << ", " << \
		YEAR_OUT_FIELD << ", " << \
		BPS_NUM_FIELD << ", " << \
		BPS_MODEL_FIELD << ", " << \
		GROUP_ID_FIELD << ", " << \
		"NDVI, PPT, " << \
		BIOMASS_TOTAL_OUT_FIELD << ", "<< \
		BIOMASS_SHRUB_OUT_FIELD << ", " << \
		BIOMASS_HERB_OUT_FIELD << ", " << \
		BIOMASS_RAW_PRODUCTION_FIELD << ", " << \
		BIOMASS_HERB_PP_FIELD << ", " << \
		BIOMASS_HERB_HOLDOVER_FIELD << ", " << \
		HERB_COVER_FIELD << ", " << \
		HERB_HEIGHT_FIELD << ", " << \
		AVG_SHRUB_HEIGHT_FIELD << ", " << \
		TOT_SHRUB_COVER_FIELD << ", " << \
		LOWER_BOUND_FIELD << ", " << \
		UPPER_BOUND_FIELD << ", " << \
		"range" << ", " << \
		S2Y_FIELD << ", " << \
		DISTURBANCE_AMOUNT_FIELD << ", " << \
		LATITUDE_FIELD << ", " << \
		LONGITUDE_FIELD << ") " << \
		"VALUES (" << \
		ap->PLOT_ID() << ",\"" << \
		ap->PLOT_NAME() << "\"," << \
		*year << "," << \
		ap->BPS_NUM() << ",\"" << \
		ap->BPS_MODEL_NUM() << "\",\"" << \
		ap->GRP_ID() << "\"," << \
		ap->getNDVI(*CLIMATE, false) << "," << \
		ap->getPPT(*CLIMATE, false) << "," << \
		ap->TOTALBIOMASS() << "," << \
		ap->SHRUBBIOMASS() << "," << \
		ap->HERBBIOMASS() << "," << \
		ap->RAWPRODUCTION() << "," << \
		ap->PRIMARYPRODUCTION() << "," << \
		ap->HERBHOLDOVER() << "," << \
		ap->HERBCOVER() << "," << \
		ap->HERBHEIGHT() << "," << \
		ap->SHRUBHEIGHT() << "," << \
		ap->SHRUBCOVER() << "," << \
		ap->LOWER_BOUND() << "," << \
		ap->UPPER_BOUND() << "," << \
		(ap->UPPER_BOUND() - ap->LOWER_BOUND()) << "," << \
		ap->S2Y() << "," << \
		ap->BIOMASS_DISTURB_AMOUNT() << "," << \
		std::setprecision(10) << ap->LATITUDE() << "," << \
		std::setprecision(10) << ap->LONGITUDE() << ");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);

	return RC;
}

int* RVS::Biomass::BiomassDIO::create_intermediate_table()
{
	std::stringstream sqlstream;
	sqlstream << "CREATE TABLE " << BIOMASS_INTERMEDIATE_TABLE << " (" << \
		PLOT_NUM_FIELD << " INTEGER NOT NULL, " << \
		PLOT_NAME_FIELD << " TEXT, " << \
		YEAR_OUT_FIELD << " INTEGER NOT NULL, " << \
		BPS_NUM_FIELD << " INTEGER NOT NULL, " << \
		DOM_SPP_FIELD << " TEXT, " << \
		SPP_CODE_FIELD << " TEXT, " << \
		BIOMASS_HERB_OUT_FIELD << " REAL, " << \
		HERB_HEIGHT_FIELD << " REAL, " << \
		HERB_COVER_FIELD << " REAL, " << \
		BIOMASS_COVER_FIELD << " TEXT, " << \
		BIOMASS_HEIGHT_FIELD << " REAL, " << \
		BIOMASS_SHRUB_CALC_FIELD << " REAL, " << \
		BIOMASS_TOTAL_OUT_FIELD << " REAL, " << \
		BIOMASS_STEMS_PER_ACRE_FIELD << " REAL, " << \
		PCH_EQU_NUM << " INTEGER, " << \
		PCH_CALC_FIELD << " REAL, " << \
		BIOMASS_EQU_NUM << " INTEGER" << ");";

	char* sql = new char; 
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);

	return RC;
}

int* RVS::Biomass::BiomassDIO::write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* record)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << BIOMASS_INTERMEDIATE_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << \
		PLOT_NAME_FIELD << ", " << \
		YEAR_OUT_FIELD << ", " << \
		BPS_NUM_FIELD << " ," << \
		DOM_SPP_FIELD << ", " << \
		SPP_CODE_FIELD << ", " << \
		BIOMASS_HERB_OUT_FIELD << ", " << \
		HERB_HEIGHT_FIELD << ", " << \
		HERB_COVER_FIELD << ", " << \
		BIOMASS_COVER_FIELD << ", " << \
		BIOMASS_HEIGHT_FIELD << ", " << \
		BIOMASS_SHRUB_CALC_FIELD << ", " << \
		BIOMASS_TOTAL_OUT_FIELD << ", " << \
		BIOMASS_STEMS_PER_ACRE_FIELD << ", " << \
		PCH_EQU_NUM << ", " << \
		PCH_CALC_FIELD << ", " << \
		BIOMASS_EQU_NUM << ") " << \
		"VALUES (" << \
		ap->PLOT_ID() << ",\"" << \
		ap->PLOT_NAME() << "\", " << \
		*year << "," << \
		ap->BPS_NUM() << ",\"" << \
		record->DOM_SPP() << "\",\"" << \
		record->SPP_CODE() << "\"," << \
		ap->HERBBIOMASS() << "," << \
		ap->HERBHEIGHT() << "," << \
		ap->HERBCOVER() << "," << \
		record->COVER() << "," << \
		record->HEIGHT() << "," << \
		record->SHRUB_SINGLE_BIOMASS() << "," << \
		record->SHRUB_EX_BIOMASS() << ", " << \
		record->STEMSPERACRE() << "," << \
		record->PCHEQNUM() << "," << \
		record->WIDTH() << "," << \
		record->BATEQNUM() << ");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);

	return RC;
}

int RVS::Biomass::BiomassDIO::query_crosswalk_table(std::string spp, std::string returnType)
{
	// Create the sqlite3 statment to query biomass crosswalk table on species
	const char* sql = query_base(BIOMASS_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);

	int colNum = dt->numCols();

	// Step over the columns, looking for the requested return type
	int ret = 0;
	for (int i = 0; i < colNum; i++)
	{
		// If the column name matches the return type, return the equation number found there
		if (strcmp(sqlite3_column_name(dt->getStmt(), i), returnType.c_str()) == 0)
		{
			getVal(dt->getStmt(), i, &ret);
		}
	}
	return ret;
}

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_equation_table(int equation_number)
{
	const char* sql = query_base(BIOMASS_EQUATION_TABLE, EQUATION_NUMBER_FIELD, equation_number);
	DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	return dt;
}

void RVS::Biomass::BiomassDIO::query_biogroup_coefs(string bps_model, double* group_const, double* ndvi_grp_interact, double* ppt_grp_interact, std::string* grp_id, bool covariance)
{
	if (bps_model.compare("base") == 0)
	{
		*grp_id = "Rip2";
	}
	else
	{
		const char* sql1 = query_base(BIOMASS_MACROGROUP_TABLE, BPS_MODEL_FIELD, bps_model);
		RVS::DataManagement::DataTable* dt1 = prep_datatable(sql1, rvsdb);
		getVal(dt1->getStmt(), dt1->Columns[GROUP_ID_FIELD], grp_id);
		//*grp_id = "G333";
	}

	std::stringstream sqlstream;

	if (covariance)
	{
		sqlstream << "SELECT * FROM " << BIOMASS_GROUP_COVARIANCE_TABLE << \
			" WHERE " << GROUP_ID_FIELD << "='" << *grp_id << "';";
	}
	else
	{
		sqlstream << "SELECT * FROM " << BIOMASS_GROUP_COEFS_TABLE << \
			" WHERE " << GROUP_ID_FIELD << "='" << *grp_id << "';";
	}

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	
	RVS::DataManagement::DataTable* dt2 = prep_datatable(sql, rvsdb);

	getVal(dt2->getStmt(), dt2->Columns[GROUP_CONST_FIELD], group_const);
	getVal(dt2->getStmt(), dt2->Columns[NDVI_INTERACT_FIELD], ndvi_grp_interact);
	getVal(dt2->getStmt(), dt2->Columns[PRCP_INTERACT_FIELD], ppt_grp_interact);
}


int RVS::Biomass::BiomassDIO::find_group_index(string* grp_id)
{
	const char* sql = query_base(BIOMASS_GROUP_COEFS_TABLE);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);

	string val;
	int row = 0;
	while (*(dt->STATUS()) == SQLITE_ROW)
	{
		row += 1;
		getVal(dt->getStmt(), dt->Columns[GROUP_ID_FIELD], &val);
		if (val.compare(*grp_id) == 0)
		{
			break;
		}
		sqlite3_step(dt->getStmt());
	}
	sqlite3_reset(dt->getStmt());
	sqlite3_step(dt->getStmt());
	return row;
}

