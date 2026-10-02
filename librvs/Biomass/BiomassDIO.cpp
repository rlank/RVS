#include "BiomassDIO.h"

#include <sstream>

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

	queuedWrites.push_back(sqlstream.str());

	return RC;
}

int* RVS::Biomass::BiomassDIO::write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap)
{
	if (!year || !ap) return RC;

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

	queuedWrites.push_back(sqlstream.str());

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

	queuedWrites.push_back(sqlstream.str());

	return RC;
}

int* RVS::Biomass::BiomassDIO::write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* record)
{
	if (!year || !ap || !record) return RC;

	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << BIOMASS_INTERMEDIATE_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << \
		PLOT_NAME_FIELD << ", " << \
		YEAR_OUT_FIELD << ", " << \
		BPS_NUM_FIELD << ", " << \
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
		record->STEMS_PER_ACRE() << "," << \
		record->PCHEQNUM() << "," << \
		record->WIDTH() << "," << \
		record->BATEQNUM() << ");";

	queuedWrites.push_back(sqlstream.str());

	return RC;
}

int RVS::Biomass::BiomassDIO::query_crosswalk_table(std::string spp, std::string returnType)
{
	const char* const sql = query_base(BIOMASS_CROSSWALK_TABLE, SPP_CODE_FIELD, spp);
	RVS::DataManagement::DataTable* const dt = prep_datatable(sql, rvsdb);
	if (!dt) return 0;

	int ret = 0;
	if (dt->Columns.contains(returnType))
	{
		getVal(dt->getStmt(), dt->Columns[returnType], &ret);
	}
	return ret;
}

RVS::DataManagement::DataTable* RVS::Biomass::BiomassDIO::query_equation_table(int equation_number)
{
	const char* const sql = query_base(BIOMASS_EQUATION_TABLE, EQUATION_NUMBER_FIELD, equation_number);
	DataManagement::DataTable* const dt = prep_datatable(sql, rvsdb);
	return dt;
}

void RVS::Biomass::BiomassDIO::query_biogroup_coefs(const std::string& bps_model, double* group_const, double* ndvi_grp_interact, double* ppt_grp_interact, std::string* grp_id, bool covariance)
{
	if (!grp_id) return;

	if (bps_model == "base")
	{
		*grp_id = "Rip2";
	}
	else
	{
		const char* const sql1 = query_base(BIOMASS_MACROGROUP_TABLE, BPS_MODEL_FIELD, bps_model);
		RVS::DataManagement::DataTable* const dt1 = prep_datatable(sql1, rvsdb);
		if (dt1)
		{
			getVal(dt1->getStmt(), dt1->Columns[GROUP_ID_FIELD], grp_id);
		}
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

	const char* const sql = streamToCharPtr(&sqlstream);
	RVS::DataManagement::DataTable* const dt2 = prep_datatable(sql, rvsdb);
	if (dt2)
	{
		getVal(dt2->getStmt(), dt2->Columns[GROUP_CONST_FIELD], group_const);
		getVal(dt2->getStmt(), dt2->Columns[NDVI_INTERACT_FIELD], ndvi_grp_interact);
		getVal(dt2->getStmt(), dt2->Columns[PRCP_INTERACT_FIELD], ppt_grp_interact);
	}
}

int RVS::Biomass::BiomassDIO::find_group_index(std::string* grp_id)
{
	if (!grp_id) return 0;
	const char* const sql = query_base(BIOMASS_GROUP_COEFS_TABLE);
	RVS::DataManagement::DataTable* const dt = prep_datatable(sql, rvsdb);
	if (!dt) return 0;

	std::string val;
	int row = 0;
	while (*(dt->STATUS()) == SQLITE_ROW)
	{
		row += 1;
		getVal(dt->getStmt(), dt->Columns[GROUP_ID_FIELD], &val);
		if (val == *grp_id)
		{
			break;
		}
		sqlite3_step(dt->getStmt());
	}
	sqlite3_reset(dt->getStmt());
	sqlite3_step(dt->getStmt());
	return row;
}
