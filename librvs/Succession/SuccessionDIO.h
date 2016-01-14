/// ********************************************************** ///
/// Name: SuccessionDIO.h                                         ///
/// Desc:                                          ///
/// Base Class(es): DIO.h                                      ///
/// ********************************************************** ///

#ifndef SUCCESSIONDIO_H
#define SUCCESSIONDIO_H

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
namespace Succession
{
	class SuccessionDIO :
		public RVS::DataManagement::DIO
	{
	public:
		SuccessionDIO(void);
		virtual ~SuccessionDIO(void);

		//## DB functins ##//
		int* create_output_table();
		int* create_intermediate_table();
		int* write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap);
		int* write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* record);

		//## Query functions ##//
		RVS::DataManagement::DataTable* query_equation_table(int equation_number);
		
		RVS::DataManagement::DataTable* query_succession_table(string bps_model_code);
		bool get_succession_data(string bps_model_code, std::map<string, string>* stringVals, std::map<string, double>* numVals);

		bool check_shrub_data_exists(string spp_code);
		bool check_code_is_shrub(string spp_code);
		string get_scientific_name(string spp_code);
	};
}
}

#endif
