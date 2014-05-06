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
		YEAR_OUT_FIELD << " INT NOT NULL," << \
		EVT_NUM_FIELD << " INT NOT NULL," << \
		BPS_NUM_FIELD << " CHAR(12) NOT NULL," << \
		FUELS_1HR_OUT_FIELD << " REAL," << \
		FUELS_10HR_OUT_FIELD << " REAL," << \
		FUELS_100HR_OUT_FIELD << " REAL );";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	RC = create_table(sql);
	delete[] sql;

	return RC;
}

int* RVS::Fuels::FuelsDIO::create_intermediate_table()
{
	std::stringstream sqlstream;
	sqlstream << "CREATE TABLE " << FUELS_INTERMEDIATE_TABLE << "(" << \
		PLOT_NUM_FIELD << " INT NOT NULL," << \
		YEAR_OUT_FIELD << " INT NOT NULL," << \
		EVT_NUM_FIELD << " INT NOT NULL," << \
		BPS_NUM_FIELD << " CHAR(12) NOT NULL," << \
		DOM_SPP_FIELD << " char(255)," << \
		SPP_CODE_FIELD << " char(255)," << \
		FUELS_1HR_OUT_FIELD << " REAL," << \
		FUELS_10HR_OUT_FIELD << " REAL," << \
		FUELS_100HR_OUT_FIELD << " REAL );";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	RC = create_table(sql);
	delete[] sql;

	return RC;
}

int* RVS::Fuels::FuelsDIO::write_fuel_output_record(int* plot_num, int* year, int* evt_num, std::string* bps, double* fuel1Hr, double* fuel10Hr, double* fuel100Hr)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << FUELS_OUTPUT_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << YEAR_OUT_FIELD << ", " << EVT_NUM_FIELD << ", " << BPS_NUM_FIELD << ", " << \
		FUELS_1HR_OUT_FIELD << ", " << 	FUELS_10HR_OUT_FIELD << ", " << FUELS_100HR_OUT_FIELD << ") " << \
		"VALUES (" << *plot_num << "," << *year << "," << *evt_num << ",\"" << *bps << "\"," << \
		*fuel1Hr << "," << *fuel10Hr << "," << *fuel100Hr << ");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	RC = exec_sql(sql);
	delete[] sql;
	return RC;
}

int* RVS::Fuels::FuelsDIO::write_fuel_intermediate_record(int* plot_num, int* year, int* evt_num, std::string* bps, std::string* dom_spp, std::string* spp_code, double* fuel1Hr, double* fuel10Hr, double* fuel100Hr)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << FUELS_OUTPUT_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << YEAR_OUT_FIELD << ", " << EVT_NUM_FIELD << ", " << BPS_NUM_FIELD << ", " << \
		BPS_NUM_FIELD << ", " << DOM_SPP_FIELD << ", " << SPP_CODE_FIELD << ", " << \
		FUELS_1HR_OUT_FIELD << ", " << FUELS_10HR_OUT_FIELD << ", " << FUELS_100HR_OUT_FIELD << ") " << \
		"VALUES (" << *plot_num << "," << *year << "," << *evt_num << ",\"" << *bps << "\",\"" << \
		*dom_spp << "\", \"" << *spp_code << "\", " << \
		*fuel1Hr << "," << *fuel10Hr << "," << *fuel100Hr << ");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	RC = exec_sql(sql);
	delete[] sql;
	return RC;
}