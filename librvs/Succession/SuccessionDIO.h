/// ********************************************************** ///
/// Name: SuccessionDIO.h                                      ///
/// Desc: Data Input/output class for Succession module.        ///
/// Standard: C++20                                            ///
/// Base Class(es): DIO.h                                      ///
/// ********************************************************** ///

#ifndef SUCCESSIONDIO_H
#define SUCCESSIONDIO_H

#include <map>
#include <string>
#include <string_view>
#include <vector>

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
	class SuccessionDIO : public RVS::DataManagement::DIO
	{
	public:
		SuccessionDIO();
		virtual ~SuccessionDIO() = default;

		//## DB functions ##//
		int* create_output_table() override;
		int* create_intermediate_table() override;
		int* write_output_record(int* year, RVS::DataManagement::AnalysisPlot* ap) override;
		int* write_intermediate_record(int* year, RVS::DataManagement::AnalysisPlot* ap, RVS::DataManagement::SppRecord* record) override;

		// Modern value-type overload
		[[nodiscard]] int write_output_record(const int year, const RVS::DataManagement::AnalysisPlot* ap);

		//## Query functions ##//
		[[nodiscard]] bool get_succession_data(
			const std::string& bps_model_code,
			std::map<std::string, std::string>& stringVals,
			std::map<std::string, double>& numVals,
			bool& doNotModel);

		// Backward-compatible pointer overload
		bool get_succession_data(
			const std::string& bps_model_code,
			std::map<std::string, std::string>* stringVals,
			std::map<std::string, double>* numVals,
			bool* doNotModel)
		{
			if (!stringVals || !numVals || !doNotModel) return false;
			return get_succession_data(bps_model_code, *stringVals, *numVals, *doNotModel);
		}

		[[nodiscard]] bool check_shrub_data_exists(const std::string& spp_code);
		[[nodiscard]] bool check_code_is_shrub(const std::string& spp_code);
		[[nodiscard]] std::string get_scientific_name(const std::string& spp_code);

		// Modern container type for covariance matrix
		[[nodiscard]] std::vector<std::vector<double>> query_covariance_matrix();

		// Modern reference signature eliminating raw pointer leaks
		void query_herb_growth_coefs(const std::string& bps_model, double& cov_rate, double& ht_rate);

		// Backward-compatible pointer overload
		void query_herb_growth_coefs(const std::string& bps_model, double* cov_rate, double* ht_rate)
		{
			double c = 0.0, h = 0.0;
			query_herb_growth_coefs(bps_model, c, h);
			if (cov_rate) *cov_rate = c;
			if (ht_rate) *ht_rate = h;
		}

	private:
		[[nodiscard]] RVS::DataManagement::DataTable* query_succession_table(const std::string& bps_model_code);
	};
}
}

#endif // SUCCESSIONDIO_H
