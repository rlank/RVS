/// ********************************************************** ///
/// Name: DIO.h                                                ///
/// Desc: Data Input/output class. Contains functions and      ///
/// methods for access data (primarily from the RVSDB)         ///
/// Base Class(es): none                                       ///
/// Requires: string, RVS_TypeDefs                             ///
/// ********************************************************** ///

#pragma once

#ifndef DIO_ACCESS_H
#define DIO_ACCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>

#ifdef _WIN64
	#include <windows.h>
#elif _WIN32
	#include <windows.h>
#elif __APPLE__
#elif __linux
	
#endif

#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

#include "DataTable.h"
#include "RVS_TypeDefs.h"



namespace RVS
{
namespace DataManagement
{
	static std::string DSN = "RVS_DB";
	static std::string PATHTODB;  // Path to the RVS Input Database

	/// <summary>
	/// Static class for querying the database (and eventually writing output database)
	/// </summary>
	class DIO_Access
	{
	public:
		//## Query functions ##//

		/// <summary>
		/// Returns an array of unique analysis plot values. Use this array to control main driver
        /// iteration. Each analysis plot can contain multiple records for different EVT/SPP combos
		/// </summary>
		static int* query_analysis_plots();
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
		
		static bool CHECK(SQLRETURN rc, char* msg, bool printSucceededMsg=false, bool quit=true);
		static void status(SQLSMALLINT handleType, SQLHANDLE theHandle, int line);

	private:
		/// <summary>
		/// Base query function. All the public functions only define the selection string.
		/// This function contains the actual OleDB stuff.
		/// </summary>
		static DataTable query_base(std::string selectString);
		
	};
}
}

#endif