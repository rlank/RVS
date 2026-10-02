#include "SuccessionDriver.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

namespace RVS
{
namespace Succession
{
	std::vector<std::vector<double>> SuccessionDriver::covariance_matrix = {};

	SuccessionDriver::SuccessionDriver(RVS::Succession::SuccessionDIO* sdio, const bool suppress_messages)
		: sdio(sdio), suppress_messages(suppress_messages)
	{
		if (sdio)
		{
			SuccessionDriver::covariance_matrix = sdio->query_covariance_matrix();
		}
	}

	int SuccessionDriver::SuccessionMain(const int year, std::string_view climate, RVS::DataManagement::AnalysisPlot* ap)
	{
		this->ap = ap;
		if (!ap || !sdio)
		{
			return -1;
		}

		this->shrubs = ap->SHRUB_RECORDS();
		this->current_climate = std::string(climate);

		// Flag for BPS modeling eligibility
		bool doNotModel = false;

		// Load the (up to) 3 succession stages' values
		loadSuccessionVals(doNotModel);

		// Calculate herbaceous production on the stack
		const double yhat = calcProduction(year);
		double lower = 0.0;
		double upper = 0.0;
		calcConfidence(year, yhat, lower, upper, 0.0);
		const double production = std::exp(yhat) * SMEAR;

		ap->primaryProduction = production;
		ap->lower_confidence = lower;
		ap->upper_confidence = upper;

		// Get current succession stage (0-3, 0 = unclassified, -1 = uncharacteristic)
		int sclass = ap->CURRENT_SUCCESSION_STAGE();
		if (sclass > 3 && sclass < 100)
		{
			sclass = 3;
		}

		int ageOfPlot = ap->plotAge;

		// When sclass is 0 or plot burned, re-detect
		if (sclass == 0 || ap->burned)
		{
			sclass = determineCurrentClass();
			if (sclass != -1)
			{
				ageOfPlot = plotAge(sclass);
				ap->plotAge = ageOfPlot;
			}
			ap->currentStage = sclass;
		}

		if (sclass == -1 || sclass == -99)
		{
			if (doNotModel)
			{
				std::stringstream s;
				s << "PLOT_ID: " << ap->PLOT_ID() << ". RVS does not model this BPS.";
				const char* c = sdio->streamToCharPtr(&s);
				sdio->write_debug_msg(c);
				ap->currentStage = -99;
			}

			ap->plotAge += 1;
			ap->currentStageType = "U";

			const int rc = sdio->write_output_record(year, ap);
			return rc;
		}

		// When using sclass to get parameters, stages are (1-3) but 0-indexed vector is (0-2)
		const size_t stageIndex = static_cast<size_t>(sclass - 1);
		if (stageIndex < successionStrParameters.size())
		{
			ap->currentStageType = successionStrParameters[stageIndex]["cohort_type"];
		}

		// Check if this type of succession stage is supported
		if (ap->currentStageType != "S" && ap->currentStageType != "H")
		{
			std::stringstream s;
			s << "PLOT_ID: " << ap->PLOT_ID() << ". Cohort type " << ap->currentStageType << " not supported in RVS.";
			const char* c = sdio->streamToCharPtr(&s);
			sdio->write_debug_msg(c);

			ap->plotAge += 1;
			const int rc = sdio->write_output_record(year, ap);
			return rc;
		}

		// Check if transitioning to next stage
		const bool nextStage = (stageIndex < successionNumParameters.size()) &&
			(ageOfPlot == static_cast<int>(successionNumParameters[stageIndex]["endAge"]));

		if (!nextStage)
		{
			if (stageIndex < successionNumParameters.size() && stageIndex < successionStrParameters.size())
			{
				growStage(successionStrParameters[stageIndex], successionNumParameters[stageIndex]);
			}
		}
		else
		{
			const size_t nextIndex = static_cast<size_t>(sclass);
			if (nextIndex < successionNumParameters.size() && nextIndex < successionStrParameters.size())
			{
				growStage(successionStrParameters[nextIndex], successionNumParameters[nextIndex]);
			}
			else if (stageIndex < successionNumParameters.size() && stageIndex < successionStrParameters.size())
			{
				growStage(successionStrParameters[stageIndex], successionNumParameters[stageIndex]);
			}
		}

		// Grow herbs using modernized HerbGrowthResult
		const HerbGrowthResult herbResult = growHerbs(ap->primaryProduction);
		ap->herbCover = herbResult.cover;
		ap->herbHeight = herbResult.height;

		// Attenuate shrubs and herbs if total cover > 100
		if (ap->herbCover + ap->shrubCover > 100.0)
		{
			const double total = ap->herbCover + ap->shrubCover;
			ap->herbCover = (ap->herbCover / total) * 100.0;
			ap->shrubCover = (ap->shrubCover / total) * 100.0;
		}

		ap->currentStage = sclass;

		const int rc = sdio->write_output_record(year, ap);

		ap->plotAge += 1;

		if (stageIndex < successionNumParameters.size() &&
			ageOfPlot == static_cast<int>(successionNumParameters[stageIndex]["endAge"]))
		{
			ap->currentStage += 1;
		}

		return rc;
	}

	void SuccessionDriver::loadSuccessionVals(bool& doNotModel)
	{
		successionStrParameters.clear();
		successionNumParameters.clear();

		std::map<std::string, std::string> strVals_primary;
		std::map<std::string, double> numVals_primary;
		std::map<std::string, std::string> strVals_secondary;
		std::map<std::string, double> numVals_secondary;
		std::map<std::string, std::string> strVals_tertiary;
		std::map<std::string, double> numVals_tertiary;

		bool lastCohort = sdio->get_succession_data(ap->BPS_MODEL_NUM(), strVals_primary, numVals_primary, doNotModel);
		if (!lastCohort)
		{
			lastCohort = sdio->get_succession_data(ap->BPS_MODEL_NUM(), strVals_secondary, numVals_secondary, doNotModel);
		}
		if (!lastCohort)
		{
			lastCohort = sdio->get_succession_data(ap->BPS_MODEL_NUM(), strVals_tertiary, numVals_tertiary, doNotModel);
			(void)lastCohort;
		}

		successionNumParameters.push_back(std::move(numVals_primary));
		successionNumParameters.push_back(std::move(numVals_secondary));
		successionNumParameters.push_back(std::move(numVals_tertiary));

		successionStrParameters.push_back(std::move(strVals_primary));
		successionStrParameters.push_back(std::move(strVals_secondary));
		successionStrParameters.push_back(std::move(strVals_tertiary));
	}

	int SuccessionDriver::determineCurrentClass()
	{
		int sclass = 0;
		int bestMidPointDist = 9999;

		const double herbCover = ap->HERBCOVER();
		const double shrubCover = ap->SHRUBCOVER();
		const double shrubHeight = ap->SHRUBHEIGHT();

		for (size_t i = 0; i < successionNumParameters.size(); ++i)
		{
			const auto& numVals = successionNumParameters[i];
			const auto& strVals = successionStrParameters[i];

			auto ctypeIt = strVals.find("cohort_type");
			if (ctypeIt == strVals.end()) continue;
			const std::string& ctype = ctypeIt->second;

			if (ctype == "H")
			{
				if (herbCover >= numVals.at("min_cov") && herbCover <= numVals.at("max_cov"))
				{
					const int midPointDist = static_cast<int>(std::abs(herbCover - numVals.at("midpoint_cov")));
					if (midPointDist < bestMidPointDist)
					{
						bestMidPointDist = midPointDist;
						sclass = static_cast<int>(numVals.at("cohort"));
					}
				}
			}
			else
			{
				if (shrubCover >= numVals.at("min_cov") && shrubCover <= numVals.at("max_cov") &&
					shrubHeight >= numVals.at("min_ht") && shrubHeight <= numVals.at("max_ht"))
				{
					const int midPointDist = static_cast<int>(std::abs(shrubCover - numVals.at("midpoint_cov")));
					if (midPointDist < bestMidPointDist)
					{
						bestMidPointDist = midPointDist;
						sclass = static_cast<int>(numVals.at("cohort"));
					}
				}
			}
		}

		if (sclass == 0)
		{
			std::stringstream s;
			s << "PLOT_ID: " << ap->PLOT_ID() << ". Plot uncharacteristic for succession.";
			const char* c = sdio->streamToCharPtr(&s);
			sdio->write_debug_msg(c);
			sclass = -1;
		}

		return sclass;
	}

	int SuccessionDriver::plotAge(const int sclass)
	{
		double cover = 0.0;
		const size_t stageIdx = static_cast<size_t>(sclass - 1);
		if (stageIdx >= successionStrParameters.size() || stageIdx >= successionNumParameters.size())
		{
			return 0;
		}

		if (successionStrParameters[stageIdx]["cohort_type"] == "H")
		{
			cover = ap->HERBCOVER();
		}
		else
		{
			cover = ap->SHRUBCOVER();
		}

		return plotAgeCalculation(cover, successionStrParameters[stageIdx], successionNumParameters[stageIdx]);
	}

	int SuccessionDriver::plotAgeCalculation(
		const double cover,
		const std::map<std::string, std::string>& strVals,
		const std::map<std::string, double>& numVals) const
	{
		const double stageStartingCover = numVals.count("min_cov") ? numVals.at("min_cov") : 0.0;
		const double coverGrowthRate = numVals.count("gr_cov") ? numVals.at("gr_cov") : 1.0;
		const double startAge = numVals.count("startAge") ? numVals.at("startAge") : 0.0;

		if (coverGrowthRate <= 0.0)
		{
			return static_cast<int>(startAge);
		}

		const int ageOfPlot = static_cast<int>((cover - stageStartingCover) / coverGrowthRate) + static_cast<int>(startAge);
		return ageOfPlot;
	}

	void SuccessionDriver::growStage(
		const std::map<std::string, std::string>& strVals,
		const std::map<std::string, double>& numVals)
	{
		auto ctypeIt = strVals.find("cohort_type");
		const std::string growthStage = (ctypeIt != strVals.end()) ? ctypeIt->second : "";

		const double ht_growth = numVals.count("gr_ht") ? numVals.at("gr_ht") : 0.0;
		const double cov_growth = numVals.count("gr_cov") ? numVals.at("gr_cov") : 0.0;
		const double max_height = numVals.count("max_ht") ? numVals.at("max_ht") : 1000.0;
		const double max_cover = numVals.count("max_cov") ? numVals.at("max_cov") : 100.0;

		if (growthStage != "H")
		{
			if (shrubs && shrubs->empty())
			{
				const std::list<std::string> newspecies = makeSpeciesList(strVals);
				for (const std::string& s : newspecies)
				{
					const bool modelSpp = sdio->check_shrub_data_exists(s);
					const bool isShrub = sdio->check_code_is_shrub(s);
					if (modelSpp && isShrub)
					{
						const std::string spp_name = sdio->get_scientific_name(s);
						shrubs->push_back(new RVS::DataManagement::SppRecord(s, 0.0, 0.0, spp_name));
					}
				}
			}

			if (!shrubs || shrubs->empty())
			{
				return;
			}

			const double shrubCount = static_cast<double>(shrubs->size());
			double divCover = 0.0;
			double divHeight = 0.0;

			if (ap->shrubCover + cov_growth >= 100.0)
			{
				divCover = (99.0 - ap->SHRUBCOVER()) / shrubCount;
			}
			else if (ap->shrubCover + cov_growth < max_cover)
			{
				divCover = cov_growth / shrubCount;
			}
			else
			{
				divCover = (max_cover - ap->SHRUBCOVER()) / shrubCount;
			}

			if (ap->shrubHeight < max_height)
			{
				divHeight = ht_growth / shrubCount;
			}

			for (auto& record : *shrubs)
			{
				if (record)
				{
					record->height += divHeight;
					record->cover = std::clamp(record->cover + divCover, 0.0, 100.0);
				}
			}
		}
	}

	HerbGrowthResult SuccessionDriver::growHerbs(const double production)
	{
		HerbGrowthResult result;
		double coverRate = 0.0;
		double herbRate = 0.0;

		sdio->query_herb_growth_coefs(ap->BPS_MODEL_NUM(), coverRate, herbRate);

		const double cover = coverRate * production;
		const double height = herbRate * production * 100.0;

		if (cover + ap->SHRUBCOVER() > 98.0)
		{
			result.cover = std::max(0.0, 100.0 - ap->SHRUBCOVER() - 1.0);
		}
		else
		{
			result.cover = cover;
		}
		result.height = height;
		return result;
	}

	std::list<std::string> SuccessionDriver::makeSpeciesList(const std::map<std::string, std::string>& strVals) const
	{
		std::list<std::string> species;
		for (int i = 1; i <= 4; ++i)
		{
			const std::string key = "species" + std::to_string(i);
			auto it = strVals.find(key);
			if (it != strVals.end() && !it->second.empty())
			{
				species.push_back(it->second);
			}
		}
		return species;
	}

	void SuccessionDriver::addNewSpecies(const std::vector<std::string>& sClassSppCodes)
	{
		if (!shrubs) return;
		for (const auto& s : sClassSppCodes)
		{
			if (sdio->check_shrub_data_exists(s) && sdio->check_code_is_shrub(s))
			{
				const std::string spp_name = sdio->get_scientific_name(s);
				shrubs->push_back(new RVS::DataManagement::SppRecord(s, 0.0, 0.0, spp_name));
			}
		}
	}

	double SuccessionDriver::calcProduction(const int year)
	{
		double ndvi = ap->getNDVI(current_climate, false);
		double ppt = ap->getPPT(current_climate, false);

		const double ln_ndvi = std::log(std::max(ndvi, 0.001));
		const double ln_ppt = std::log(std::max(ppt, 0.001));

		const double rawProduction = -5.2058235 + (ln_ppt * 0.1088213) + (ln_ndvi * 1.386304);
		ap->rawProduction = std::exp(rawProduction) * SMEAR;

		const double adjust = 1.0 - (ap->SHRUBCOVER() / 100.0);
		ndvi *= adjust;
		ppt *= adjust;

		const double adj_ln_ndvi = std::log(std::max(ndvi, 0.001));
		const double adj_ln_ppt = std::log(std::max(ppt, 0.001));

		const double biomass = -5.2058235 + (adj_ln_ppt * 0.1088213) + (adj_ln_ndvi * 1.386304);
		return biomass;
	}

	void SuccessionDriver::calcConfidence(
		const int year,
		const double biomass,
		double& lower,
		double& upper,
		const double level)
	{
		double ndvi = ap->getNDVI(current_climate, false);
		double ppt = ap->getPPT(current_climate, false);

		const double adjust = 1.0 - (ap->SHRUBCOVER() / 100.0);
		ndvi *= adjust;
		ppt *= adjust;

		const double ln_ndvi = std::log(std::max(ndvi, 0.001));
		const double ln_ppt = std::log(std::max(ppt, 0.001));

		const double ln_yhat = biomass;
		constexpr double tval = 1.961961;

		const double s2y = calc_s2b(ln_ndvi, ln_ppt);
		const double sy = std::sqrt(std::max(0.0, s2y));

		const double ln_yhat_lower = ln_yhat - tval * sy;
		const double yhat_lower = std::exp(ln_yhat_lower) * SMEAR;

		const double ln_yhat_upper = ln_yhat + tval * sy;
		const double yhat_upper = std::exp(ln_yhat_upper) * SMEAR;

		lower = yhat_lower;
		upper = yhat_upper;
	}

	double SuccessionDriver::calc_s2b(const double lnNDVI, const double lnPPT)
	{
		// Direct evaluation of quadratic form: s2b = x * Sigma * x^T
		// where x = [1.0, lnPPT, lnNDVI]
		const std::array<double, 3> x = { 1.0, lnPPT, lnNDVI };
		double sum = 0.0;

		if (!covariance_matrix.empty())
		{
			const size_t rows = std::min<size_t>(3, covariance_matrix.size());
			for (size_t i = 0; i < rows; ++i)
			{
				const size_t cols = std::min<size_t>(3, covariance_matrix[i].size());
				for (size_t j = 0; j < cols; ++j)
				{
					sum += x[i] * covariance_matrix[i][j] * x[j];
				}
			}
		}

		ap->s2y = sum;
		return sum;
	}

	std::vector<std::vector<double>> SuccessionDriver::matrix_mult(
		const std::vector<std::vector<double>>& A,
		const std::vector<std::vector<double>>& B)
	{
		if (A.empty() || B.empty()) return {};
		const size_t ROW = A.size();
		const size_t INNER = A[0].size();
		const size_t COL = B[0].size();

		std::vector<std::vector<double>> C(ROW, std::vector<double>(COL, 0.0));
		for (size_t r = 0; r < ROW; ++r)
		{
			for (size_t c = 0; c < COL; ++c)
			{
				double s = 0.0;
				for (size_t k = 0; k < INNER; ++k)
				{
					s += A[r][k] * B[k][c];
				}
				C[r][c] = s;
			}
		}
		return C;
	}

	std::vector<std::vector<double>> SuccessionDriver::matrix_trans(
		const std::vector<std::vector<double>>& A)
	{
		if (A.empty()) return {};
		const size_t ROW = A.size();
		const size_t COL = A[0].size();

		std::vector<std::vector<double>> B(COL, std::vector<double>(ROW, 0.0));
		for (size_t c = 0; c < COL; ++c)
		{
			for (size_t r = 0; r < ROW; ++r)
			{
				B[c][r] = A[r][c];
			}
		}
		return B;
	}

	std::vector<std::vector<double>> SuccessionDriver::generate_dummy_variables(
		const int index,
		const double lnPPT,
		const double lnNDVI)
	{
		std::vector<std::vector<double>> dummy(1, std::vector<double>(102, 0.0));
		dummy[0][0] = 1.0;
		dummy[0][1] = lnPPT;
		dummy[0][2] = lnNDVI;

		int counter = 1;
		for (size_t i = 3; i < 102; ++i)
		{
			if (counter == index && i < 33)
			{
				dummy[0][i] = 1.0;
			}
			else if (counter == index && i < 66)
			{
				dummy[0][i] = lnPPT;
			}
			else if (counter == index && i < 99)
			{
				dummy[0][i] = lnNDVI;
			}

			if (counter == 33)
			{
				counter = 0;
			}
			counter += 1;
		}
		return dummy;
	}
}
}
