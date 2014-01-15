/// ********************************************************** ///
/// Name: BiomassDIO.h                                         ///
/// Desc: Data Input/output class for dealing with Biomass     ///
/// data and queries.                                          ///
/// Base Class(es): none                                       ///
/// Requires: string, RVS_TypeDefs                             ///
/// ********************************************************** ///

#ifndef BIOMASSDIO_H
#define BIOMASSDIO_H


#include <string>

#include <boost/any.hpp>

#include <DataManagement/DataTable.h>
#include <DataManagement/DIO.h>
#include <RVSDBNAMES.h>
#include <RVSDEF.h>


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
		/// Queries Biomass_Input table from RVSDB for records matching a passed
		/// plot number. Biomass_Input should contain information about dominant
		/// species, codes for input variables, and values of input variables.
		static RVS::DataManagement::DataTable* query_biomass_input_table(int plot_num);
	};
}
}






#endif