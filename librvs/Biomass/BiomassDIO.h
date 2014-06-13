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

#include "../DataManagement/AnalysisPlot.h"
#include "../DataManagement/DataTable.h"
#include "../DataManagement/DIO.h"
#include "../DataManagement/SppRecord.h"
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
		int* write_biomass_output_record(int* plot_num, int* year, int* evt_num,int* bps, double* totalBiomass, double* herbBiomass, double* shrubBiomass);
		int* write_biomass_intermediate_record(RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* record, int* plot_num, int* year, double* shrubBiomass);

		//## Query functions ##//

		// Queries Bio_Crosswalk for which equations to use
		int query_crosswalk_table(std::string spp, std::string returnType);
		// Returns a record from the biomass equation table
		RVS::DataManagement::DataTable* query_equation_table(int equation_number);
		// Queries Bio_Herbs table to lookup herbaceous biomass. Takes a BPS number to query against.
		double query_biomass_pp_table(int baseBPS, char* level);


		// Return a collection of required values for herbaceous biomass calculation.
		// Takes the macro group ID as parameter
		void query_biogroup_coefs(int bps, double* group_const, double* ndvi_grp_interact, double* ppt_grp_interact);
	};
}
}






#endif