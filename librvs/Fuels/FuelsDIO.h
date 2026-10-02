/// ********************************************************** ///
/// Name: FuelsDIO.h                                           ///
/// Desc: Data Input/output class for dealing with Fuels        ///
///       data and queries.                                    ///
/// Standard: C++20                                            ///
/// Base Class(es): DIO.h                                      ///
/// ********************************************************** ///

#ifndef FUELSDIO_H
#define FUELSDIO_H

#include <map>
#include <memory>
#include <string>
#include <string_view>

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
	class FuelsDIO : public RVS::DataManagement::DIO
	{
	public:
		FuelsDIO();
		virtual ~FuelsDIO() = default;

		//## DB functions ##//
		// Base class override (legacy signature)
		int* create_output_table() override;
		int* create_intermediate_table() override;
		int* write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap) override;
		int* write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp) override;

		// Modern value-type overloads
		[[nodiscard]] int write_output_record(const int year, const RVS::DataManagement::AnalysisPlot* ap);

		//## Query functions ##//

		// Queries the fuels crosswalk table for all the equations available for a species.
		[[nodiscard]] std::map<std::string, int> query_crosswalk_table(const std::string& spp);
		[[nodiscard]] RVS::DataManagement::DataTable* query_equation_table(const std::map<std::string, int>& equationNumbers);
		[[nodiscard]] RVS::DataManagement::DataTable* query_equation_table(const int equationNumber);
		[[nodiscard]] RVS::DataManagement::DataTable* query_fbfm_rules_selector();
		[[nodiscard]] RVS::DataManagement::DataTable* query_fbfm_rules(std::string_view classTable);
	};
}
}

#endif // FUELSDIO_H
