#include "SuccessionDIO.h"

RVS::Succession::SuccessionDIO::SuccessionDIO(void) : RVS::DataManagement::DIO()
{
	this->create_output_table();
	this->create_intermediate_table();
}

RVS::Succession::SuccessionDIO::~SuccessionDIO(void)
{

}

int* RVS::Succession::SuccessionDIO::create_output_table()
{


	return RC;
}

int* RVS::Succession::SuccessionDIO::write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap)
{


	return RC;
}

int* RVS::Succession::SuccessionDIO::create_intermediate_table()
{

	return RC;
}

int* RVS::Succession::SuccessionDIO::write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* record)
{


	return RC;
}

RVS::DataManagement::DataTable* RVS::Succession::SuccessionDIO::query_equation_table(int equation_number)
{
	return nullptr;
}

RVS::DataManagement::DataTable* RVS::Succession::SuccessionDIO::query_succession_table(int bps_code)
{
	const char* sql = query_base(SUCCESSION_TABLE, BPS_NUM_FIELD, bps_code);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	return dt;
}

bool RVS::Succession::SuccessionDIO::get_succession_data(int bps_code, std::map<string, string>* stringVals, std::map<string, double>* numVals)
{
	RVS::DataManagement::DataTable* succDt;
	try
	{
		succDt = query_succession_table(bps_code);
	}
	catch (RVS::DataManagement::DataNotFoundException dex)
	{
		succDt = query_succession_table(10800);
	}

	double cohort;
	getVal(succDt->getStmt(), succDt->Columns["COHORT"], &cohort);
	double startAge;
	double endAge;
	getVal(succDt->getStmt(), succDt->Columns["StartAge"], &startAge);
	getVal(succDt->getStmt(), succDt->Columns["EndAge"], &endAge);
	double midpoint;
	double gr_ht;
	double gr_cov;
	double max_ht;
	double max_cov;
	getVal(succDt->getStmt(), succDt->Columns["MIDPOINT"], &midpoint);
	getVal(succDt->getStmt(), succDt->Columns["GR_HT_YR"], &gr_ht);
	getVal(succDt->getStmt(), succDt->Columns["GR_COV_YR"], &gr_cov);
	getVal(succDt->getStmt(), succDt->Columns["MAX_HT"], &max_ht);
	getVal(succDt->getStmt(), succDt->Columns["MAX_CC"], &max_cov);
	string cohort_type;
	getVal(succDt->getStmt(), succDt->Columns["COHORT_TYPE"], &cohort_type);
	string species1;
	string species2;
	string species3;
	string species4;
	getVal(succDt->getStmt(), succDt->Columns["Species_1"], &species1);
	getVal(succDt->getStmt(), succDt->Columns["Species_2"], &species2);
	getVal(succDt->getStmt(), succDt->Columns["Species_3"], &species3);
	getVal(succDt->getStmt(), succDt->Columns["Species_4"], &species4);
	string cover_type;
	getVal(succDt->getStmt(), succDt->Columns["COVER_TYPE"], &cover_type);
	
	(*stringVals)["cohort_type"] = cohort_type;
	(*stringVals)["species1"] = species1;
	(*stringVals)["species2"] = species2;
	(*stringVals)["species3"] = species3;
	(*stringVals)["species4"] = species4;
	(*stringVals)["cover_type"] = cover_type;

	(*numVals)["cohort"] = cohort;
	(*numVals)["startAge"] = startAge;
	(*numVals)["endAge"] = endAge;
	(*numVals)["midpoint"] = midpoint;
	(*numVals)["gr_ht"] = gr_ht;
	(*numVals)["gr_cov"] = gr_cov;
	(*numVals)["max_ht"] = max_ht;
	(*numVals)["max_cov"] = max_cov;

	

	bool last_stage = false;

	if (strcmp(cover_type.c_str(), "Late") == 0)
	{
		last_stage = true;
		*RC = sqlite3_reset(succDt->getStmt());
	}

	*RC = sqlite3_step(succDt->getStmt());

	return last_stage;
}

bool RVS::Succession::SuccessionDIO::check_shrub_data_exists(string spp_code)
{
	bool dataExists = true;
	if (spp_code.compare("") == 0)
	{
		dataExists = false;
	}
	const char* sql = query_base("NRCSPlantsDB", "Plants Code", spp_code);
	try
	{
		RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	}
	catch (RVS::DataManagement::DataNotFoundException dex)
	{
		dataExists = false;
	}
	return dataExists;
}

string RVS::Succession::SuccessionDIO::get_scientific_name(string spp_code)
{
	const char* sql = query_base("NRCSPlantsDB", "Plants Code", spp_code);

	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	string dom_spp;
	getVal(dt->getStmt(), dt->Columns["Scientific Name"], &dom_spp);
	return dom_spp;
}