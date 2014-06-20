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

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/any.hpp>

#include "DataTable.h"
#include "../RVSDBNAMES.h"
#include "../RVSDEF.h"

// Need to avoid circular reference here, so declare empty classes
namespace RVS { namespace DataManagement { class AnalysisPlot; } }
namespace RVS { namespace DataManagement { class SppRecord; } }

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
		void getVal(sqlite3_stmt* stmt, int column, boost::any* retVal);
		void getVal(sqlite3_stmt* stmt, int column, double* retval);
		void getVal(sqlite3_stmt* stmt, int column, std::string* retVal);
		void getVal(sqlite3_stmt* stmt, int column, int* retVal);
		void getVal(sqlite3_stmt* stmt, int column, bool* retVal);
		
		virtual DataTable* query_equation_table(int equation_number) = 0;
		virtual void query_equation_coefficients(int equation_number, double* coefs);
		virtual void query_equation_parameters(int equation_number, string* params);
		virtual void query_equation_parameters(int equation_number, string* params, double* coefs);
		virtual void query_equation_parameters(int equation_number, string* params, double* coefs, int* equation_type);

		virtual void query_fuels_basic_info(const int* bps, int* fbfm, bool* isDry);

	protected:
		int* create_output_db();
		int* create_output_db(char* path);
		int* create_table(char* sql);
		
		virtual int* create_output_table() = 0;
		virtual int* create_intermediate_table() = 0;
		virtual int* write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap) = 0;
		virtual int* write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp) = 0;

		// Directly execute a SQL statement against the OUTPUT database
		int* exec_sql(char* sql);

		// Converts a std::stringstream to a char pointer (array)
		char* streamToCharPtr(std::stringstream* stream);
		// Opens the database connection. Will remain open until DIO destructs
		int* open_db_connection(char* pathToDb, sqlite3** db);

		/// Base query function. All the public functions only define the selection string.
		sqlite3_stmt* query_base(const char* selectString);
		sqlite3_stmt* query_base(const char* table, const char* field);
		sqlite3_stmt* query_base(const char* table, const char* field, boost::any whereclause);

		// Bogus function for sqlite3_exec
		static int callback(void* nu, int argc, char** argv, char** azColName);


		bool checkDBStatus(sqlite3* db, const char* sql = "", const char* err = "");
		
	private:
		static sqlite3* rvsdb;  // SQLite database object
		static sqlite3* outdb;  // SQLite output database object
	};
}
}

#endif
