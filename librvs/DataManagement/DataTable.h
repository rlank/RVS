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

#include <sqlite3.h>

#include <RVSDEF.h>

namespace RVS
{
namespace DataManagement
{
	class DataTable
	{
	public:
		inline DataTable() {}
		inline DataTable(sqlite3_stmt* stmt) { this->stmt = stmt; }
		virtual inline ~DataTable(void) { *RC = sqlite3_finalize(stmt); }

		inline sqlite3_stmt* getStmt() { return stmt; }

	private:
		sqlite3_stmt* stmt;
	};
}
}

#endif