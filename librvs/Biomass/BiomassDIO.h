/// ********************************************************** ///
/// Name: BiomassDIO.h                                         ///
/// Desc: Data Input/output class for dealing with Biomass     ///
/// data and queries.                                          ///
/// Base Class(es): DIO.h                                      ///
/// ********************************************************** ///

#ifndef BIOMASSDIO_H
#define BIOMASSDIO_H

#include <iomanip>
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
		int* write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap);
		int* write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* record);

		//## Query functions ##//

		// Queries Bio_Crosswalk for which equations to use
		int query_crosswalk_table(std::string spp, std::string returnType);
		// Returns a record from the biomass equation table
		RVS::DataManagement::DataTable* query_equation_table(int equation_number);

		

		int find_group_index(string* grp_id);

		// Return a collection of required values for herbaceous biomass calculation.
		// Takes the macro group ID as parameter
		void query_biogroup_coefs(string bps_model, double* group_const, double* ndvi_grp_interact, double* ppt_grp_interact, std::string* grp_id, bool covariance);

		
	};
}
}

#endif
