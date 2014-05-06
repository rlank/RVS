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
		int* create_output_table();
		int* create_intermediate_table();
		int* write_biomass_output_record(int* plot_num, int* year, int* evt_num, std::string* bps, char* ret_code, double* totalBiomass, double* herbBiomass, double* shrubBiomass);
		int* write_biomass_intermediate_record(RVS::Biomass::BiomassEVT* bioEVT, int* plot_num, int* year, double* shrubBiomass);

		//## Query functions ##//

		/// Queries Bio_Crosswalk for equation determination for species without
		/// expicit equations in Bio_Equations.
		RVS::DataManagement::DataTable* query_biomass_crosswalk_table(std::string spp);
		int query_biomass_crosswalk_table(std::string spp, std::string returnType);
		RVS::DataManagement::DataTable* query_biomass_equation_table(std::string spp_code);
		RVS::DataManagement::DataTable* query_biomass_equation_table(int equation_number);
		void query_biomass_equation_coefficients(int equation_number, float* cf1, float* cf2, float* cf3, float* cf4);
		/// Queries Bio_Herbs table to lookup herbaceous biomass. Takes a BPS number to query against.
		RVS::DataManagement::DataTable* query_biomass_herbs_table(int baseBPS);
		double query_biomass_herbs_table(int baseBPS, char* level);
		/// Queries Biomass_Input table from RVSDB for records matching a passed
		/// plot number. Biomass_Input should contain information about dominant
		/// species, codes for input variables, and values of input variables.
		RVS::DataManagement::DataTable* query_biomass_input_table(int plot_num);


		/// Return a collection of required values for herbaceous biomass calculation.
		/// Takes the macro group ID as parameter
		void query_biogroup_coefs(std::string bps, double* group_const, double* ndvi_grp_interact, double* ppt_grp_interact);
	};
}
}






#endif