/// ********************************************************** ///
/// Name: FuelsDIO.h                                           ///
/// Desc: Data Input/output class for dealing with Biomass     ///
/// data and queries.                                          ///
/// Base Class(es): DIO.h                                      ///
/// ********************************************************** ///

#ifndef FUELSDIO_H
#define FUELSDIO_H

#include <string>

#include <boost/any.hpp>

#include "../DataManagement/DataTable.h"
#include "../DataManagement/DIO.h"
#include "../RVSDBNAMES.h"
#include "../RVSDEF.h"


namespace RVS
{
namespace Fuels
{
	class FuelsDIO : 
		public RVS::DataManagement::DIO
	{
	public:
		FuelsDIO(void);
		virtual ~FuelsDIO(void);

		//## DB functins ##//
		int* create_output_table();
		int* create_intermediate_table();
		int* write_fuel_output_record(int* plot_num, int* year, int* evt_num, std::string* bps, double* fuel1Hr, double* fuel10Hr, double* fuel100Hr);
		int* write_fuel_intermediate_record(int* plot_num, int* year, int* evt_num, std::string* bps, std::string* dom_spp, std::string* spp_code, double* fuel1Hr, double* fuel10Hr, double* fuel100Hr);


		//## Query functions ##//

		/// Queries Bio_Crosswalk for equation determination for species without
		/// expicit equations in Bio_Equations.
		RVS::DataManagement::DataTable* query_fuels_crosswalk_table(std::string spp);

		void query_shrubs_equation_coefficients(int equation_number, double* cf1, double* cf2, double* cf3, double* cf4);
		void query_shrubs_equation_parameters(int equation_number, std::string* p1name, std::string* p2name, std::string* p3name);
	};
}
}






#endif