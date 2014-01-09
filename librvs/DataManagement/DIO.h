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

#include <RVSDEF.h>
#include "DataTable.h"
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
		/// Queries Bio_Crosswalk for equation determination for species without
		/// expicit equations in Bio_Equations.
		static DataTable query_biomass_crosswalk_table(std::string spp);
		static DataTable query_biomass_equation_table(std::string spp_code);
		static DataTable query_biomass_equation_table(int equation_number);
		/// Queries Bio_Herbs table to lookup herbaceous biomass. Takes a BPS number to query against.
		static DataTable query_biomass_herbs_table(int baseBPS);
		/// Queries Biomass_Input table from RVSDB for records matching a passed
		/// plot number. Biomass_Input should contain information about dominant
		/// species, codes for input variables, and values of input variables.
		static DataTable* query_biomass_input_table(int plot_num);
		
	private:
		static sqlite3* rvsdb;  // SQLite database object

		static char* statementPrep(char* base, int whereInt);
		static char* statementPrep(char* base, const char* whereStr);

		static int open_db_connection(char* pathToDb);

		/// Base query function. All the public functions only define the selection string.
		/// This function contains the actual OleDB stuff.
		static sqlite3_stmt* query_base(char* selectString);
		static DataTable query_base_old(char* selectString);
	};
}
}

#endif
