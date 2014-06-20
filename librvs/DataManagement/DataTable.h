/// ********************************************************** ///
/// Name: DataTable.h                                          ///
/// Desc: Wrapper class for an sqlite3 statement object. This  ///
/// assures proper destruction of the statement despite being  ///
/// passed around to different parts of the program.           ///
/// Base Class(es): none                                       ///
/// Requires:                                                  ///
/// ********************************************************** ///

#pragma once

#ifndef DATATABLE_H
#define DATATABLE_H

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

		// Return the base sqlite3_stmt pointer
		inline sqlite3_stmt* getStmt() { return stmt; }

		// Maps column names to their index position
		std::map<std::string, int> Columns;

		inline int numCols() { return column_count; }
		inline int numRows() { return row_count; }  // Disabled

	private:
		sqlite3_stmt* stmt;
		int column_count;
		int row_count;
	};
}
}

#endif