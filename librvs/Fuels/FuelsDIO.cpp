#include "FuelsDIO.h"

RVS::Fuels::FuelsDIO::FuelsDIO(void) : RVS::DataManagement::DIO()
{
	this->create_fuel_output_table();
	this->create_fuel_intermediate_table();
}

RVS::Fuels::FuelsDIO::~FuelsDIO(void)
{

}

int* RVS::Fuels::FuelsDIO::create_fuel_output_table()
{
	std::stringstream sqlstream;
	sqlstream << "CREATE TABLE " << FUELS_OUTPUT_TABLE << "(" << \
		PLOT_NUM_FIELD << " INT PRIMARY KEY NOT NULL," << \
		EVT_NUM_FIELD << " INT NOT NULL," << \
		BPS_NUM_FIELD << " CHAR(12) NOT NULL," << \
		FUELS_1HR_OUT_FIELD << " REAL," << \
		FUELS_10HR_OUT_FIELD << " REAL," << \
		FUELS_100HR_OUT_FIELD << " REAL );";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	RC = RVS::DataManagement::DIO::create_table(sql);
	delete[] sql;

	return RC;
}

int* RVS::Fuels::FuelsDIO::create_fuel_intermediate_table()
{
	return 0;
}

int* RVS::Fuels::FuelsDIO::write_fuel_output_record(int* plot_num, int* evt_num, std::string* bps, double* fuel1Hr, double* fuel10Hr, double* fuel100Hr)
{
	std::stringstream sqlstream;
	sqlstream << "INSERT INTO " << FUELS_OUTPUT_TABLE << " (" << \
		PLOT_NUM_FIELD << ", " << EVT_NUM_FIELD << ", " << BPS_NUM_FIELD << ", " << \
		FUELS_1HR_OUT_FIELD << ", " << 	FUELS_10HR_OUT_FIELD << ", " << FUELS_100HR_OUT_FIELD << ") " << \
		"VALUES (" << *plot_num << "," << *evt_num << ",\"" << *bps << "\"," << \
		*fuel1Hr << "," << *fuel10Hr << "," << *fuel100Hr << ");";

	char* sql = new char;
	sql = streamToCharPtr(&sqlstream);
	RC = RVS::DataManagement::DIO::exec_sql(sql);
	delete[] sql;
	return RC;
}