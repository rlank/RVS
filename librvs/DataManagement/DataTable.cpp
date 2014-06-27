#include "DataTable.h"

RVS::DataManagement::DataTable::DataTable(sqlite3_stmt* stmt)
{
	this->stmt = stmt;
	this->sql = sqlite3_sql(stmt);

	Columns = std::map<std::string, int>();

	int colCount = sqlite3_column_count(stmt);
	for (int c = 0; c < colCount; c++)
	{
		std::string colName = std::string(sqlite3_column_name(stmt, c));
		Columns[colName] = c;
	}

	column_count = colCount;

	//$$ Comment the row count business out when released. For debugging only
	row_count = 0;
	/*
	*RC = sqlite3_step(stmt);
	while (*RC == SQLITE_ROW)
	{
		row_count++;
		*RC = sqlite3_step(stmt);
	}
	*RC = sqlite3_reset(stmt);
	*/
}


RVS::DataManagement::DataTable::~DataTable()
{
	*RC = sqlite3_finalize(stmt);
}