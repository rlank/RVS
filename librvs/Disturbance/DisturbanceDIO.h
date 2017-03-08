/// ********************************************************** ///
/// Name: DisturbanceDIO.h                                     ///
/// Desc: 				                                       ///
/// Base Class(es): DIO.h                                      ///
/// ********************************************************** ///

#pragma once

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
namespace Disturbance
{
	class DisturbanceDIO : 
		public RVS::DataManagement::DIO
	{
	public:
		DisturbanceDIO(void);
		virtual ~DisturbanceDIO(void);

		//## DB functions ##//
		int* create_output_table();
		int* create_intermediate_table();
		int* write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap);
		int* write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp);

		//## Query functions ##//

		//virtual DataTable* query_equation_table(int equation_number);
		map<int, vector<DisturbAction>> query_disturbance_input();
		
	private:
		void query_parameters_table();
		map<string, map<string, vector<string>>> availableActions;
	};
}
}

