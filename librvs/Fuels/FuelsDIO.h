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
#include "FuelsEVT.h"
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
		static int* create_fuel_output_table();
		static int* create_fuel_intermediate_table();
		static int* write_fuel_output_record(int* plot_num, int* evt_num, std::string* bps, double* fuel1Hr, double* fuel10Hr, double* fuel100Hr);

		//## Query functions ##//

	};
}
}






#endif