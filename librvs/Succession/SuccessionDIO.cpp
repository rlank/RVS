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
	std::stringstream sqlstream;
	sqlstream << "CREATE TABLE " << "Succession_Output" << "(" << \
		PLOT_NUM_FIELD << " INTEGER NOT NULL, " << \
		PLOT_NAME_FIELD << " TEXT, " << \
		YEAR_OUT_FIELD << " INTEGER NOT NULL, " << \
		"BPS_MODEL" << " TEXT NOT NULL, " << \
		"STAGE" << " INTEGER, " << \
		"COHORT_TYPE" << " TEXT, " << \
		"PLOT_AGE" << " REAL);";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);

	return RC;
}

int* RVS::Succession::SuccessionDIO::write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << "Succession_Output" << " (" << \
		PLOT_NUM_FIELD << ", " << \
		PLOT_NAME_FIELD << ", " << \
		YEAR_OUT_FIELD << ", " << \
		"BPS_MODEL" << ", " << \
		"STAGE" << ", " << \
		"COHORT_TYPE" << ", " << \
		"PLOT_AGE" << ") " << \
		"VALUES (" << \
		ap->PLOT_ID() << ",\"" << \
		ap->PLOT_NAME() << "\"," << \
		*year << ",\"" << \
		ap->BPS_MODEL_NUM() << "\"," << \
		ap->CURRENT_SUCCESSION_STAGE() << ",\"" << \
		ap->CURRENT_STAGE_TYPE() << "\"," << \
		ap->PLOT_AGE() << ");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	queuedWrites.push_back(sql);

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

RVS::DataManagement::DataTable* RVS::Succession::SuccessionDIO::query_succession_table(string bps_model_code)
{
	const char* sql = query_base(SUCCESSION_TABLE, "BPS_MODEL", bps_model_code, "COHORT");
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	return dt;
}

bool RVS::Succession::SuccessionDIO::get_succession_data(string bps_model_code, std::map<string, string>* stringVals, std::map<string, double>* numVals, bool* doNotModel)
{
	RVS::DataManagement::DataTable* succDt;

	succDt = query_succession_table(bps_model_code);

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

	double min_ht;
	double min_cov;

	getVal(succDt->getStmt(), succDt->Columns["MIDPOINT"], &midpoint);
	getVal(succDt->getStmt(), succDt->Columns["GR_HT"], &gr_ht);
	getVal(succDt->getStmt(), succDt->Columns["GR_COV"], &gr_cov);
	getVal(succDt->getStmt(), succDt->Columns["MAX_HT"], &max_ht);
	getVal(succDt->getStmt(), succDt->Columns["MAX_CC"], &max_cov);
	getVal(succDt->getStmt(), succDt->Columns["MIN_HT"], &min_ht);
	getVal(succDt->getStmt(), succDt->Columns["MIN_CC"], &min_cov);



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
	
	int goNoGo = -99;
	getVal(succDt->getStmt(), succDt->Columns["GoNoGo"], &goNoGo);
	if (goNoGo < 0)
	{
		*doNotModel = true;
	}
	else
	{
		*doNotModel = false;
	}
	
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
	(*numVals)["min_ht"] = min_ht;
	(*numVals)["min_cov"] = min_cov;
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
	const char* sql = query_base(PLANTS_TABLE, "Plants_Code", spp_code);

	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	
	return dataExists;
}

bool RVS::Succession::SuccessionDIO::check_code_is_shrub(string spp_code)
{
	bool isShrub = false;
	const char* sql = query_base(PLANTS_TABLE, "Plants_Code", spp_code);

	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	string plantType;
	transform(plantType.begin(), plantType.end(), plantType.begin(), ::toupper);
	getVal(dt->getStmt(), dt->Columns[LIFEFORM_FIELD], &plantType);
		
	size_t f = plantType.find("shrub");
	if (f >= 0) { isShrub = true; }

	return isShrub;
}

string RVS::Succession::SuccessionDIO::get_scientific_name(string spp_code)
{
	const char* sql = query_base(PLANTS_TABLE, "Plants_Code", spp_code);

	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	string dom_spp;
	getVal(dt->getStmt(), dt->Columns[DOM_SPP_FIELD], &dom_spp);
	return dom_spp;
}

void RVS::Succession::SuccessionDIO::query_herb_growth_coefs(string bps_model, double* cov_rate, double* ht_rate)
{
	const char* sql = query_base(HERB_GROWTH_TABLE, BPS_MODEL_FIELD, bps_model);
	RVS::DataManagement::DataTable* dt;
	*cov_rate = 0;
	*ht_rate = 0;

	dt = prep_datatable(sql, rvsdb, true);
	if (*(dt->STATUS()) == SQLITE_ROW)
	{
		getVal(dt->getStmt(), dt->Columns[HERB_CC_GROWTH_FIELD], cov_rate);
		getVal(dt->getStmt(), dt->Columns[HERB_HT_GROWTH_FIELD], ht_rate);
	}
	else
	{
		stringstream* s = new stringstream();
		*s << "Herb growth not found for BPS " << bps_model << ", using defaults";
		const char* c = streamToCharPtr(s);
		write_debug_msg(c);

		*cov_rate = 0.020334387;
		*ht_rate = 0.000419084;
	}
}

double** RVS::Succession::SuccessionDIO::query_covariance_matrix()
{
	const char* sql = query_base(COVARIANCE_TABLE);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	std::stringstream sqlstream;

	int colNum = dt->numCols();

	double** covariance_matrix = new double*[3];

	int row = 0;
	int col = 0;
	double val = 0;

	for (row = 0; row < colNum; row++)
	{
		covariance_matrix[row] = new double[3];

		for (col = 0; col < colNum; col++)
		{
			getVal(dt->getStmt(), col, &val);
			covariance_matrix[row][col] = val;
		}
		sqlite3_step(dt->getStmt());
	}
	return covariance_matrix;
}
