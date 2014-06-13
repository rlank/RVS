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

using namespace std;

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

		/// Querys the main input table ("Plots")
		DataTable* query_input_table(int plot_num);
		/// Querys the shrub input table ("Shrubs")
		DataTable* query_shrubs_table(int plot_num);

		/// Returns a value from a specified column in a sqlite statement object
		//template<typename double*>
		virtual void getVal(sqlite3_stmt* stmt, int column, boost::any* retVal);
		virtual void getVal(sqlite3_stmt* stmt, int column, double* retval);
		virtual void getVal(sqlite3_stmt* stmt, int column, std::string* retVal);
		virtual void getVal(sqlite3_stmt* stmt, int column, int* retVal);
		
		virtual DataTable* query_equation_table(int equation_number) = 0;
		virtual void query_equation_coefficients(int equation_number, double* coefs);
		virtual void query_equation_parameters(int equation_number, string* params);
		virtual void query_equation_parameters(int equation_number, string* params, double* coefs);
		virtual void query_equation_parameters(int equation_number, string* params, double* coefs, int* equation_type);

	protected:
		int* create_output_db();
		int* create_output_db(char* path);
		int* create_table(char* sql);
		
		virtual int* create_output_table() = 0;
		virtual int* create_intermediate_table() = 0;
		//virtual int* write_output_record();
		//virtual int* write_intermediate_record();

		// Directly execute a SQL statement against the input database
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
