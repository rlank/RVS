/// ********************************************************** ///
/// Name: DataTable.h                                          ///
/// Desc: Wrapper class for an sqlite3 statement object. This  ///
/// assures proper destruction of the statement despite being  ///
/// passed around to different parts of the program.           ///
/// Base Class(es): none                                       ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#pragma once

#ifndef DATATABLE_H
#define DATATABLE_H

#include <map>
#include <string>
#include <utility>

#include <sqlite3.h>

#include "../RVSDEF.h"

namespace RVS
{
namespace DataManagement
{
	class DataTable
	{
	public:
		explicit DataTable(sqlite3_stmt* stmt);
		virtual ~DataTable();

		// Non-copyable (Rule of Five)
		DataTable(const DataTable&) = delete;
		DataTable& operator=(const DataTable&) = delete;

		// Moveable
		DataTable(DataTable&& other) noexcept;
		DataTable& operator=(DataTable&& other) noexcept;

		const char* sql = nullptr;

		// Return the base sqlite3_stmt pointer
		[[nodiscard]] sqlite3_stmt* getStmt() const noexcept { return stmt; }

		[[nodiscard]] int* STATUS() noexcept { return &sqlStatus; }
		[[nodiscard]] int status() const noexcept { return sqlStatus; }
		void setStatus(const int s) noexcept { sqlStatus = s; }

		// Maps column names to their index position
		std::map<std::string, int> Columns;

		[[nodiscard]] int numCols() const noexcept { return column_count; }
		[[nodiscard]] int numRows() const noexcept { return row_count; }

	private:
		sqlite3_stmt* stmt = nullptr;
		int column_count = 0;
		int row_count = 0;
		int sqlStatus = SQLITE_OK;
	};
}
}

#endif // DATATABLE_H
