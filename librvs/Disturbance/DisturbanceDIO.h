/// ********************************************************** ///
/// Name: DisturbanceDIO.h                                     ///
/// Desc: Data Input/output class for Disturbance module.      ///
/// Standard: C++20                                            ///
/// Base Class(es): DIO.h                                      ///
/// ********************************************************** ///

#pragma once

#ifndef DISTURBANCEDIO_H
#define DISTURBANCEDIO_H

#include <map>
#include <string>
#include <vector>

#include "../DataManagement/AnalysisPlot.h"
#include "../DataManagement/DataTable.h"
#include "../DataManagement/DIO.h"
#include "../DataManagement/SppRecord.h"
#include "../RVSDBNAMES.h"
#include "../RVSDEF.h"
#include "DisturbAction.h"

namespace RVS
{
namespace Disturbance
{
	class DisturbanceDIO : public RVS::DataManagement::DIO
	{
	public:
		DisturbanceDIO();
		virtual ~DisturbanceDIO() = default;

		//## DB functions ##//
		int* create_output_table() override;
		int* create_intermediate_table() override;
		int* write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap) override;
		int* write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* spp) override;

		//## Query functions ##//
		[[nodiscard]] std::map<int, std::vector<DisturbAction>> query_disturbance_input();
		
	private:
		void query_parameters_table();
		std::map<std::string, std::map<std::string, std::vector<std::string>>> availableActions;
	};
}
}

#endif // DISTURBANCEDIO_H
