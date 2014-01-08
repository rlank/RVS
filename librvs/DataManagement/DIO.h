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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <RVSDEF.h>
#include "DataTable.h"
#include "RVS_TypeDefs.h"


namespace RVS
{
namespace DataManagement
{
	/// <summary>
	/// Static class for querying the database (and eventually writing output database)
	/// </summary>
	class DIO
	{
	public:
		DIO(char* pathToDb);
		virtual ~DIO();

		static int open_db_connection(char* pathToDb);
		static inline void setPathToDB(char* path) { PATHTODB = path; }
		
		//## Query functions ##//

		/// <summary>
		/// Returns an array of unique analysis plot values. Use this array to control main driver
		/// iteration. Each analysis plot can contain multiple records for different EVT/SPP combos
		/// </summary>
		static std::vector<int> query_analysis_plots();
		/// <summary>
		/// Queries Bio_Crosswalk for equation determination for species without
		/// expicit equations in Bio_Equations.
		/// </summary>
		static DataTable query_biomass_crosswalk_table(std::string spp);
		static DataTable query_biomass_equation_table(std::string spp_code);
		static DataTable query_biomass_equation_table(int equation_number);
		/// <summary>
		/// Queries Bio_Herbs table to lookup herbaceous biomass. Takes a BPS number to query against.
		/// </summary>
		static DataTable query_biomass_herbs_table(int baseBPS);
		/// <summary>
		/// Queries Biomass_Input table from RVSDB for records matching a passed
		/// plot number. Biomass_Input should contain information about dominant
		/// species, codes for input variables, and values of input variables.
		/// </summary>
		static DataTable query_biomass_input_table(int plot_num);
		
		//## Data Access Functions ##//
		#if USESQLITE
		//static int callback(void *notUsed, int argc, char** argv, char** azColName);
		#else
		static bool CHECK(SQLRETURN rc, char* msg, bool printSucceededMsg = false, bool quit = true);
		static void status(SQLSMALLINT handleType, SQLHANDLE theHandle, int line);
		#endif
	private:
		static sqlite3* rvsdb;  // SQLite database object
		static char* PATHTODB;  // Path to the RVS Input Database

		static void columnTraverse(sqlite3_stmt* stmt);
		static char* statementPrep(char* base, int whereInt);
		static char* statementPrep(char* base, const char* whereStr);

		/// <summary>
		/// Base query function. All the public functions only define the selection string.
		/// This function contains the actual OleDB stuff.
		/// </summary>
		static sqlite3_stmt* query_base(char* selectString);
		static DataTable query_base_old(char* selectString);
	};
}
}

#endif
