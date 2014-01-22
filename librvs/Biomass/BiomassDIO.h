/// ********************************************************** ///
/// Name: BiomassDIO.h                                         ///
/// Desc: Data Input/output class for dealing with Biomass     ///
/// data and queries.                                          ///
/// Base Class(es): DIO.h                                      ///
/// ********************************************************** ///

#ifndef BIOMASSDIO_H
#define BIOMASSDIO_H

#include <string>

#include <boost/any.hpp>

#include "../DataManagement/DataTable.h"
#include "../DataManagement/DIO.h"
#include "BiomassEVT.h"
#include "../RVSDBNAMES.h"
#include "../RVSDEF.h"


namespace RVS
{
namespace Biomass
{
	class BiomassDIO : 
		public RVS::DataManagement::DIO
	{
	public:
		BiomassDIO(void);
		virtual ~BiomassDIO(void);


		//## DB functins ##//
		static int* create_biomass_output_table();
		static int* create_biomass_intermediate_table();
		static int* write_biomass_output_record(int* plot_num, int* evt_num, std::string* bps, char* ret_code, double* totalBiomass, double* herbBiomass, double* shrubBiomass);
		static int* write_biomass_intermediate_record(RVS::Biomass::BiomassEVT* bioEVT, int* plot_num, double* shrubBiomass);

		//## Query functions ##//

		/// Queries Bio_Crosswalk for equation determination for species without
		/// expicit equations in Bio_Equations.
		static RVS::DataManagement::DataTable* query_biomass_crosswalk_table(std::string spp);
		static int query_biomass_crosswalk_table(std::string spp, std::string returnType);
		static RVS::DataManagement::DataTable* query_biomass_equation_table(std::string spp_code);
		static RVS::DataManagement::DataTable* query_biomass_equation_table(int equation_number);
		static void query_biomass_equation_coefficients(int equation_number, float* cf1, float* cf2, float* cf3, float* cf4);
		/// Queries Bio_Herbs table to lookup herbaceous biomass. Takes a BPS number to query against.
		static RVS::DataManagement::DataTable* query_biomass_herbs_table(int baseBPS);
		static double query_biomass_herbs_table(int baseBPS, char* level);
		/// Queries Biomass_Input table from RVSDB for records matching a passed
		/// plot number. Biomass_Input should contain information about dominant
		/// species, codes for input variables, and values of input variables.
		static RVS::DataManagement::DataTable* query_biomass_input_table(int plot_num);
	};
}
}






#endif