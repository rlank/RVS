/// ********************************************************** ///
/// Name: DataTable.h                                          ///
/// Desc: Wrapper class for an sqlite3 statement object. This  ///
/// assures proper destruction of the statement despite being  ///
/// passed around to different parts of the program.           ///
/// Base Class(es): none                                       ///
/// Requires:                                                  ///
/// ********************************************************** ///

#pragma once

#include <map>

#include <sqlite3.h>

#include "../RVSDEF.h"

namespace RVS
{
namespace DataManagement
{
	class DataTable
	{
	public:
		DataTable(sqlite3_stmt* stmt);
		virtual ~DataTable(void);

		const char* sql;

		// Return the base sqlite3_stmt pointer
		inline sqlite3_stmt* getStmt() { return stmt; }

		inline int* STATUS() { return &sqlStatus; }

		// Maps column names to their index position
		std::map<std::string, int> Columns;

		// std::map<int, boost::any> Data;

		inline int numCols() { return column_count; }
		inline int numRows() { return row_count; }  // Disabled

	private:
		sqlite3_stmt* stmt;
		int column_count;
		int row_count;
		int sqlStatus;
	};
}
}
