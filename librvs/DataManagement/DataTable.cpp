#include "DataTable.h"

#include <utility>

RVS::DataManagement::DataTable::DataTable(sqlite3_stmt* stmt)
	: stmt(stmt), column_count(0), row_count(0), sqlStatus(SQLITE_OK)
{
	if (stmt)
	{
		this->sql = sqlite3_sql(stmt);

		const int colCount = sqlite3_column_count(stmt);
		for (int c = 0; c < colCount; ++c)
		{
			const char* const colNameRaw = sqlite3_column_name(stmt, c);
			const std::string colName = colNameRaw ? colNameRaw : "";
			Columns[colName] = c;
		}

		column_count = colCount;
	}
}

RVS::DataManagement::DataTable::DataTable(DataTable&& other) noexcept
	: sql(std::exchange(other.sql, nullptr)),
	  Columns(std::move(other.Columns)),
	  stmt(std::exchange(other.stmt, nullptr)),
	  column_count(std::exchange(other.column_count, 0)),
	  row_count(std::exchange(other.row_count, 0)),
	  sqlStatus(std::exchange(other.sqlStatus, SQLITE_OK))
{
}

RVS::DataManagement::DataTable& RVS::DataManagement::DataTable::operator=(DataTable&& other) noexcept
{
	if (this != &other)
	{
		if (stmt)
		{
			if (RC)
			{
				*RC = sqlite3_finalize(stmt);
			}
			else
			{
				sqlite3_finalize(stmt);
			}
		}

		stmt = std::exchange(other.stmt, nullptr);
		sql = std::exchange(other.sql, nullptr);
		Columns = std::move(other.Columns);
		column_count = std::exchange(other.column_count, 0);
		row_count = std::exchange(other.row_count, 0);
		sqlStatus = std::exchange(other.sqlStatus, SQLITE_OK);
	}
	return *this;
}

RVS::DataManagement::DataTable::~DataTable()
{
	if (stmt)
	{
		if (RC)
		{
			*RC = sqlite3_finalize(stmt);
		}
		else
		{
			sqlite3_finalize(stmt);
		}
		stmt = nullptr;
	}
}