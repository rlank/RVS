/// ********************************************************** ///
/// Name: FuelsDIO.h                                           ///
/// Desc: Data Input/output class for dealing with Biomass     ///
/// data and queries.                                          ///
/// Base Class(es): DIO.h                                      ///
/// ********************************************************** ///

#ifndef FUELSDIO_H
#define FUELSDIO_H

#include <map>
#include <string>


#include "../DataManagement/DataTable.h"
#include "../DataManagement/DIO.h"
#include "../DataManagement/SppRecord.h"
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

		//## DB functions ##//
		int* create_output_table();
		int* create_intermediate_table();
		int* write_fuel_output_record(int* plot_num, int* year, int* evt_num, int* bps, std::map<std::string, double> fuelsValues);
		int* write_fuel_intermediate_record(int* plot_num, int* year, int* evt_num, int* bps, std::string* dom_spp, std::string* spp_code, std::map<std::string, double> fuelsValues);


		//## Query functions ##//

		// Queries the fuels crosswalk table for all the equations available for a species.
		std::map<std::string, int> query_crosswalk_table(std::string spp);
		RVS::DataManagement::DataTable* query_equation_table(std::map<std::string, int> equationNumbers);
		RVS::DataManagement::DataTable* query_equation_table(int equationNumber);
		
	};
}
}






#endif