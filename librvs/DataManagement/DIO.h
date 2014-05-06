/// ********************************************************** ///
/// Name: DIO.h                                                ///
/// Desc: Data Input/output class. Contains functions and      ///
/// methods for access data (primarily from the RVSDB)         ///
/// Base Class(es): none                                       ///
/// Requires: string, RVS_TypeDefs                             ///
/// ********************************************************** ///

#pragma once

#ifndef DIO_H
#define DIO_H

#include <sstream>
#include <string>
#include <vector>

#include <boost/any.hpp>

#include "DataTable.h"
#include "../RVSDBNAMES.h"
#include "../RVSDEF.h"
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
		std::vector<int> query_analysis_plots();
		
		
	protected:
		int* create_output_db();
		int* create_output_db(char* path);
		int* create_table(char* sql);
		
		virtual int* create_output_table();
		virtual int* create_intermediate_table();
		//virtual int* write_output_record();
		//virtual int* write_intermediate_record();

		int* exec_sql(char* sql);

		inline sqlite3* get_outdb() { return outdb; }

		// Converts a std::stringstream to a char pointer (array)
		char* streamToCharPtr(std::stringstream* stream);
		// Opens the database connection. Will remain open until DIO destructs
		int* open_db_connection(char* pathToDb, sqlite3** db);

		/// Base query function. All the public functions only define the selection string.
		/// This function contains the actual OleDB stuff.
		sqlite3_stmt* query_base(char* selectString);
		sqlite3_stmt* query_base(char* table, char* field);
		sqlite3_stmt* query_base(char* table, char* field, boost::any whereclause);

		static int callback(void* nu, int argc, char** argv, char** azColName);

	private:
		static sqlite3* rvsdb;  // SQLite database object
		static sqlite3* outdb;  // SQLite output database object
	};
}
}

#endif
