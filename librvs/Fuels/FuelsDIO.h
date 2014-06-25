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

#include "../DataManagement/AnalysisPlot.h"
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
		int* write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap);
		int* write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp);

		//## Query functions ##//

		// Queries the fuels crosswalk table for all the equations available for a species.
		std::map<std::string, int> query_crosswalk_table(std::string spp);
		RVS::DataManagement::DataTable* query_equation_table(std::map<std::string, int> equationNumbers);
		RVS::DataManagement::DataTable* query_equation_table(int equationNumber);
		RVS::DataManagement::DataTable* query_fbfm_rules_selector(void);
		RVS::DataManagement::DataTable* query_fbfm_rules(std::string classTable);
		
	};
}
}






#endif