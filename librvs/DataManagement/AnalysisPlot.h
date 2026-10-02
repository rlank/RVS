/// ********************************************************** ///
/// Name: AnalysisPlot.h                                       ///
/// Desc: Holds all records in an analysis plot                ///
/// Base Class(es): none                                       ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#pragma once

#ifndef ANALYSISPLOT_H
#define ANALYSISPLOT_H

#include <array>
#include <memory>
#include <queue>
#include <string>
#include <string_view>
#include <vector>

#include "DataTable.h"
#include "DIO.h"
#include "SppRecord.h"
#include "../Disturbance/DisturbAction.h"

namespace RVS { namespace Biomass { class BiomassDriver; } }
namespace RVS { namespace Biomass { class BiomassEqDriver; } }
namespace RVS { namespace Fuels   { class FuelsDriver;   } }
namespace RVS { namespace Succession { class SuccessionDriver; } }
namespace RVS { namespace Disturbance { class DisturbanceDriver; } }

namespace RVS
{
namespace DataManagement
{
	class AnalysisPlot
	{
		friend class RVS::Biomass::BiomassDriver;
		friend class RVS::Fuels::FuelsDriver;
		friend class RVS::Biomass::BiomassEqDriver;
		friend class RVS::Succession::SuccessionDriver;
		friend class RVS::Disturbance::DisturbanceDriver;

	public:
		AnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		virtual ~AnalysisPlot();

		AnalysisPlot(const AnalysisPlot&) = delete;
		AnalysisPlot& operator=(const AnalysisPlot&) = delete;
		AnalysisPlot(AnalysisPlot&&) noexcept = default;
		AnalysisPlot& operator=(AnalysisPlot&&) noexcept = default;

		[[nodiscard]] int PLOT_ID() const noexcept { return plot_id; }
		[[nodiscard]] const std::string& PLOT_NAME() const noexcept { return plot_name; }
		[[nodiscard]] int EVT_NUM() const noexcept { return evt_num; }
		[[nodiscard]] int BPS_NUM() const noexcept { return bps_num; }
		[[nodiscard]] const std::string& BPS_MODEL_NUM() const noexcept { return bps_model_num; }
		[[nodiscard]] const std::string& GRP_ID() const noexcept { return grp_id; }
		[[nodiscard]] bool ISDRY() const noexcept { return dryClimate; }

		[[nodiscard]] double LOWER_BOUND() const noexcept { return lower_confidence; }
		[[nodiscard]] double UPPER_BOUND() const noexcept { return upper_confidence; }
		[[nodiscard]] double S2Y() const noexcept { return s2y; }

		[[nodiscard]] double LATITUDE() const noexcept { return latitude; }
		[[nodiscard]] double LONGITUDE() const noexcept { return longitude; }

		// Collection of shrub records. Returns a synchronized pointer view for backward compatibility
		[[nodiscard]] std::vector<RVS::DataManagement::SppRecord*>* SHRUB_RECORDS();
		[[nodiscard]] const std::vector<std::unique_ptr<RVS::DataManagement::SppRecord>>& SHRUB_UNIQUE_RECORDS() const noexcept
		{
			return shrubRecords;
		}

		// Average shrub height (cm)
		[[nodiscard]] double SHRUBHEIGHT() const noexcept { return shrubHeight; }
		// Total shrub cover (%)
		[[nodiscard]] double SHRUBCOVER() const noexcept { return shrubCover; }
		// Herbaceous height (cm)
		[[nodiscard]] double HERBHEIGHT() const noexcept { return herbHeight; }
		// Herbaceous cover (%)
		[[nodiscard]] double HERBCOVER() const noexcept { return herbCover; }

		void HERB_RESET_TEST_ONLY() noexcept { previousHerbProductions[0] = 0.0; }

		// Total herb biomass (lbs/ac)
		[[nodiscard]] double HERBBIOMASS() const noexcept { return herbBiomass; }
		[[nodiscard]] double RAWPRODUCTION() const noexcept { return rawProduction; }
		[[nodiscard]] double PRIMARYPRODUCTION() const noexcept { return primaryProduction; }
		[[nodiscard]] double HERBHOLDOVER() const noexcept { return herbHoldoverBiomass; }

		// Total shrub biomass (lbs/ac)
		[[nodiscard]] double SHRUBBIOMASS() const noexcept { return shrubBiomass * GRAMS_TO_POUNDS; }
		[[nodiscard]] double SHRUBSINGLESTEM() const noexcept { return shrubAvgStem * GRAMS_TO_POUNDS; }
		// Total biomass (shrubs + herbs) (lbs/ac)
		[[nodiscard]] double TOTALBIOMASS() const noexcept { return totalBiomass; }

		// Fuels returns. CONVERTS TO LBS/AC
		[[nodiscard]] double SHRUB_1HR_WB() const noexcept { return shrub1HourWB * GRAMS_TO_POUNDS; }
		[[nodiscard]] double SHRUB_1HR_FOLIAGE() const noexcept { return shrub1HourFoliage * GRAMS_TO_POUNDS; }
		[[nodiscard]] double SHRUB_10HR() const noexcept { return shrub10Hour * GRAMS_TO_POUNDS; }
		[[nodiscard]] double SHRUB_100HR() const noexcept { return shrub100Hour * GRAMS_TO_POUNDS; }
		[[nodiscard]] double SHRUB_1000HR() const noexcept { return shrub1000Hour * GRAMS_TO_POUNDS; }
		[[nodiscard]] double FUEL_TOTAL_1HR() const noexcept { return total1HrFuel * GRAMS_TO_POUNDS; }
		[[nodiscard]] double HERB_FUEL() const noexcept { return herbFuel * GRAMS_TO_POUNDS; }
		[[nodiscard]] double FUEL_TOTAL() const noexcept
		{
			return (shrub1HourWB + shrub1HourFoliage + shrub10Hour + shrub100Hour + shrub1000Hour + herbFuel) * GRAMS_TO_POUNDS;
		}

		// Fuel model for the plot
		[[nodiscard]] int FBFM() const noexcept { return calcFBFM == 0 ? defaultFBFM : calcFBFM; }
		[[nodiscard]] const std::string& FBFM_NAME() const noexcept { return fbfmName; }

		[[nodiscard]] int CURRENT_SUCCESSION_STAGE() const noexcept { return currentStage; }
		[[nodiscard]] const std::string& CURRENT_STAGE_TYPE() const noexcept { return currentStageType; }
		[[nodiscard]] int PLOT_AGE() const noexcept { return plotAge; }

		static constexpr double GRAMS_TO_POUNDS = 0.00220462;
		static constexpr double POUNDS_TO_GRAMS = 453.592;

		void push_shrub(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		void push_shrub(RVS::DataManagement::SppRecord* record);
		void push_shrub(std::unique_ptr<RVS::DataManagement::SppRecord> record);
		void update_shrubvalues();

		// Get NDVI for the requested level
		[[nodiscard]] double getNDVI(std::string_view level, bool useRand) const;
		[[nodiscard]] double getNDVI(const std::string& level, bool useRand) const
		{
			return getNDVI(std::string_view(level), useRand);
		}

		// Get precipitation for the requested level
		[[nodiscard]] double getPPT(std::string_view level, bool useRand) const;
		[[nodiscard]] double getPPT(const std::string& level, bool useRand) const
		{
			return getPPT(std::string_view(level), useRand);
		}

		[[nodiscard]] std::vector<RVS::Disturbance::DisturbAction> getDisturbancesForYear(int year) const;
		void setDisturbances(std::vector<RVS::Disturbance::DisturbAction> dists) { disturbances = std::move(dists); }
		[[nodiscard]] double BIOMASS_DISTURB_AMOUNT() const noexcept { return biomassReductionTotal * GRAMS_TO_POUNDS; }

		// Publicly accessible for historical production tracking
		std::array<double, 3> previousHerbProductions{};

	private:
		int plot_id = 0;
		std::string plot_name;
		int evt_num = 0;
		std::string evt_name;
		int bps_num = 0;
		std::string bps_model_num;
		int fallback_bps_num = 0;
		std::string grp_id;
		double latitude = 0.0;
		double longitude = 0.0;

		bool doNotModel = false;

		double lower_confidence = 0.0;
		double upper_confidence = 0.0;
		double s2y = 0.0;

		double shrubHeight = 0.0;
		double shrubCover = 0.0;
		double herbHeight = 0.0;
		double herbCover = 0.0;
		double totalBiomass = 0.0;
		double herbBiomass = 0.0;
		double herbHoldoverBiomass = 0.0;
		double rawProduction = 0.0;
		double primaryProduction = 0.0;
		double shrubBiomass = 0.0;
		double shrubAvgStem = 0.0;

		int defaultFBFM = 0;
		int calcFBFM = 0;
		bool dryClimate = true;
		std::string fbfmName;

		double fuel1HrProp = 0.0;
		double fuelFoilageProp = 0.0;
		double fuel10HrProp = 0.0;
		double fuel100HrProp = 0.0;

		double shrub1HourWB = 0.0;
		double shrub1HourFoliage = 0.0;
		double shrub10Hour = 0.0;
		double shrub100Hour = 0.0;
		double shrub1000Hour = 0.0;
		double total1HrFuel = 0.0;
		double herbFuel = 0.0;

		std::vector<std::unique_ptr<RVS::DataManagement::SppRecord>> shrubRecords;
		mutable std::vector<RVS::DataManagement::SppRecord*> rawShrubView;

		std::vector<double> ndviValues;
		std::vector<double> precipValues;

		int currentStage = 0;
		std::string currentStageType;
		int plotAge = 0;
		int timeInHerbStage = 0;

		void buildAnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		void buildInitialFuels(RVS::DataManagement::DIO* dio);
		void syncExternalShrubPointers();

		std::vector<RVS::Disturbance::DisturbAction> disturbances;
		bool disturbed = false;
		bool burned = false;
		double biomassReductionTotal = 0.0;
	};
}
}

#endif // ANALYSISPLOT_H
