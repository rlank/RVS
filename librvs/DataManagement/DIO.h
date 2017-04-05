/// ********************************************************** ///
/// Name: DIO.h                                                ///
/// Desc: Data Input/output class. Contains functions and      ///
/// methods for access data (primarily from the RVSDB)         ///
/// ********************************************************** ///

#pragma once

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/any.hpp>


#include "../RVSDBNAMES.h"
#include "../RVSDEF.h"
#include "DataTable.h"
#include "RVSException.h"

// Need to avoid circular reference here, so declare empty classes
namespace RVS { namespace DataManagement { class AnalysisPlot; } }
namespace RVS { namespace DataManagement { class SppRecord; } }

using namespace std;

namespace RVS
{
namespace DataManagement
{
	// Class for querying the database (and eventually writing output database)
	class DIO
	{
	public:
		DIO(void);
		virtual ~DIO(void);

		//## Query functions ##//

		// Returns an array of unique analysis plot values. Use this array to control main driver
		// iteration. Each analysis plot can contain multiple records for different EVT/SPP combos
		std::vector<int> query_analysis_plots();

		DataTable* query_input_table(void);

		// Querys the main input table ("Plots")
		DataTable* query_input_table(int plot_num);
		// Querys the shrub input table ("Shrubs")
		DataTable* query_shrubs_table(void);

		// Returns a value from a specified column in a sqlite statement object
		void getVal(sqlite3_stmt* stmt, int column, boost::any* retVal);
		void getVal(sqlite3_stmt* stmt, int column, double* retval);
		void getVal(sqlite3_stmt* stmt, int column, std::string* retVal);
		void getVal(sqlite3_stmt* stmt, int column, int* retVal);
		void getVal(sqlite3_stmt* stmt, int column, bool* retVal);
		
		virtual DataTable* query_equation_table(int equation_number);
		virtual void query_equation_coefficients(int equation_number, double* coefs);
		virtual void query_equation_parameters(int equation_number, string* params);
		virtual void query_equation_parameters(int equation_number, string* params, double* coefs);
		virtual void query_equation_parameters(int equation_number, string* params, double* coefs, int* equation_type);

		virtual void query_fuels_basic_info(const int* bps, int* fbfm, bool* isDry);

		int* write_output(void);
		void write_debug_msg(const char* msg);

		// Converts a std::stringstream to a char pointer (array)
		char* streamToCharPtr(std::stringstream* stream);

	protected:
		virtual int* create_output_table() = 0;
		virtual int* create_intermediate_table() = 0;
		virtual int* write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap) = 0;
		virtual int* write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp) = 0;

		RVS::DataManagement::DataTable* prep_datatable(const char* sql, sqlite3* db, bool addToActive=true, bool reset=false);
		
		/// Base query function. All the public functions only define the selection string.
		const char* query_base(const char* table);
		const char* query_base(const char* table, const char* field);
		const char* query_base(const char* table, const char* field, int whereClause);
		const char* query_base(const char* table, const char* field, string whereClause);
		const char* query_base(const char* table, const char* field, int whereClause, string order);
		const char* query_base(const char* table, const char* field, string whereClause, string order);

		// Bogus function for sqlite3_exec
		static int callback(void* nu, int argc, char** argv, char** azColName);

		bool checkDBStatus(sqlite3* db, const char* sql = "", const char* err = "");
		int* finalizeQueries(void);

		static vector<const char*> queuedWrites;
		
		static sqlite3* rvsdb;  // SQLite database object
		static sqlite3* outdb;  // SQLite output database object

	private:
		static map<string, shared_ptr<DataTable>> activeQueries;
		static int buildInMemDB(sqlite3 *pInMemory, const char *zFilename, int isSave);

		int* create_output_db();
		int* create_output_db(char* path);

		bool isQueryActive(string sql);
		// Opens the database connection. Will remain open until DIO destructs
		int* open_db_connection(char* pathToDb, sqlite3** db);
	};
}
}
