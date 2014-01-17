/// ********************************************************** ///
/// Name: DIO.h                                                ///
/// Desc: Data Input/output class. Contains functions and      ///
/// methods for access data (primarily from the RVSDB)         ///
/// Base Class(es): none                                       ///
/// Requires: string, RVS_TypeDefs                             ///
/// ********************************************************** ///

#pragma once
#pragma warning(disable:4996)

#ifndef DIO_H
#define DIO_H

#include <sstream>
#include <string>
#include <vector>

#include <boost/any.hpp>

#include "DataTable.h"
#include <RVSDBNAMES.h>
#include <RVSDEF.h>
#include "RVS_TypeDefs.h"

namespace RVS
{
namespace DataManagement
{
	/// Static class for querying the database (and eventually writing output database)
	class DIO
	{
	public:
		DIO(void);
		virtual ~DIO(void);

		//## Query functions ##//

		/// Returns an array of unique analysis plot values. Use this array to control main driver
		/// iteration. Each analysis plot can contain multiple records for different EVT/SPP combos
		static std::vector<int> query_analysis_plots();
		
		
	protected:
		static int* create_output_db();
		static int* create_output_db(char* path);
		static int* create_table(char* sql);
		
		static int* exec_sql(char* sql);

		static inline sqlite3* get_outdb() { return outdb; }

		// Converts a std::stringstream to a char pointer (array)
		static char* streamToCharPtr(std::stringstream* stream);
		// Opens the database connection. Will remain open until DIO destructs
		static int* open_db_connection(char* pathToDb, sqlite3** db);

		/// Base query function. All the public functions only define the selection string.
		/// This function contains the actual OleDB stuff.
		static sqlite3_stmt* query_base(char* selectString);
		static sqlite3_stmt* query_base(char* table, char* field);
		static sqlite3_stmt* query_base(char* table, char* field, boost::any whereclause);

		static int callback(void* nu, int argc, char** argv, char** azColName);

	private:
		static sqlite3* rvsdb;  // SQLite database object
		static sqlite3* outdb;  // SQLite output database object
		//static DataTable query_base_old(char* selectString);
	};
}
}

#endif
