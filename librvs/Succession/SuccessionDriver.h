/// ********************************************************** ///
/// Name: SuccessionDriver.h                                   ///
/// Desc: Main driver for the Succession module. Models stage  ///
///       transitions, herbaceous growth, and allometrics.     ///
/// Standard: C++20                                            ///
/// Base Class(es): none                                       ///
/// ********************************************************** ///

#pragma once

#ifndef SUCCESSIONDRIVER_H
#define SUCCESSIONDRIVER_H

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "SuccessionDIO.h"
#include "../DataManagement/AnalysisPlot.h"
#include "../DataManagement/SppRecord.h"

namespace RVS
{
namespace Succession
{
	/// \brief Struct encapsulating herbaceous growth result
	struct HerbGrowthResult
	{
		double cover = 0.0;   ///< Herbaceous canopy cover (%)
		double height = 0.0;  ///< Herbaceous height (cm)
	};

	class SuccessionDriver
	{
	public:
		explicit SuccessionDriver(RVS::Succession::SuccessionDIO* sdio, const bool suppress_messages = false);
		virtual ~SuccessionDriver() = default;

		/// \brief Main succession driver entry point.
		/// \param year Current simulation year
		/// \param climate Climate scenario name ("Normal", "Dry", "Wet", etc.)
		/// \param ap Pointer to analysis plot
		/// \return Status code (0 on success)
		[[nodiscard]] int SuccessionMain(const int year, std::string_view climate, RVS::DataManagement::AnalysisPlot* ap);

	private:
		RVS::Succession::SuccessionDIO* sdio = nullptr;
		RVS::DataManagement::AnalysisPlot* ap = nullptr;
		std::vector<RVS::DataManagement::SppRecord*>* shrubs = nullptr;
		bool suppress_messages = false;
		std::string current_climate;

		static constexpr float MSE = 0.1276825f;
		static constexpr float SMEAR = 1.06431775f;

		// Replaced static double** with modern 2D vector
		static std::vector<std::vector<double>> covariance_matrix;

		std::vector<std::map<std::string, std::string>> successionStrParameters;
		std::vector<std::map<std::string, double>> successionNumParameters;

		void loadSuccessionVals(bool& doNotModel);

		[[nodiscard]] int determineCurrentClass();
		
		[[nodiscard]] int plotAge(const int sclass);
		[[nodiscard]] int plotAgeCalculation(
			const double cover,
			const std::map<std::string, std::string>& strVals,
			const std::map<std::string, double>& numVals) const;

		void growStage(
			const std::map<std::string, std::string>& strVals,
			const std::map<std::string, double>& numVals);

		/// \brief Eliminates out-parameter raw pointers; computes herb growth.
		[[nodiscard]] HerbGrowthResult growHerbs(const double production);

		/// \brief Reference overload for in-place growth update.
		void growHerbs(double& herbCover, double& herbHeight, const double production)
		{
			const HerbGrowthResult res = growHerbs(production);
			herbCover = res.cover;
			herbHeight = res.height;
		}

		[[nodiscard]] std::list<std::string> makeSpeciesList(const std::map<std::string, std::string>& strVals) const;
		void addNewSpecies(const std::vector<std::string>& sClassSppCodes);

		[[nodiscard]] double calcProduction(const int year);
		void calcConfidence(
			const int year,
			const double biomass,
			double& lower,
			double& upper,
			const double level = 0.0);

		[[nodiscard]] double calc_s2b(const double lnNDVI, const double lnPPT);

		// Modern vector-based matrix helpers
		[[nodiscard]] static std::vector<std::vector<double>> matrix_mult(
			const std::vector<std::vector<double>>& A,
			const std::vector<std::vector<double>>& B);
		[[nodiscard]] static std::vector<std::vector<double>> matrix_trans(
			const std::vector<std::vector<double>>& A);
		[[nodiscard]] static std::vector<std::vector<double>> generate_dummy_variables(
			const int index,
			const double lnPPT,
			const double lnNDVI);
	};
}
}

#endif // SUCCESSIONDRIVER_H
