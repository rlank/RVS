/// ********************************************************** ///
/// Name: AnalysisPlot.h                                       ///
/// Desc: Holds a vector of all records in an analysis plot    ///
/// Base Class(es): none                                       ///
/// ********************************************************** ///

#pragma once

#ifndef ANALYSIS_PLOT_H
#define ANALYSIS_PLOT_H

#include <string>
#include <vector>

#include "DataTable.h"
#include "DIO.h"
#include "SppRecord.h"

namespace RVS { namespace Biomass { class BiomassDriver; } }
namespace RVS { namespace Biomass { class BiomassEqDriver; } }
namespace RVS { namespace Fuels   { class FuelsDriver;   } }
namespace RVS { namespace Succession { class SuccessionDriver; } }

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

	public:
		AnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		virtual ~AnalysisPlot(void);

		inline const int PLOT_ID() { return plot_id; }
		inline const string PLOT_NAME() { return plot_name; }
		inline const int EVT_NUM() { return evt_num; }
		inline const int BPS_NUM() { return bps_num; }
		inline const string BPS_MODEL_NUM() { return bps_model_num; }
		inline const int BPS_NUM(bool useFallback) { return useFallback ? fallback_bps_num : bps_num; }
		inline const int HUC() { return huc; }
		inline const string GRP_ID() { return grp_id; }
		//inline const bool ISDRY() { return dryClimate; }

		inline const double LOWER_BOUND() { return lower_confidence; }
		inline const double UPPER_BOUND() { return upper_confidence; }
		inline const double S2Y() { return s2y; }

		inline const double LATITUDE() { return latitude; }
		inline const double LONGITUDE() { return longitude; }

		// Collection of shrub records. Some plots will only have a single record, others many.
		inline std::vector<RVS::DataManagement::SppRecord*>* SHRUB_RECORDS() { return &shrubRecords; }

		// Average shrub height (cm)
		inline double SHRUBHEIGHT() { return shrubHeight; }
		// Total shrub cover (%)
		inline double SHRUBCOVER() { return shrubCover; }
		// Herbaceous height (cm)
		inline double HERBHEIGHT() { return herbHeight; }
		// Herbaceous cover (%)
		inline double HERBCOVER() { return herbCover; }

		// Total herb biomass (lbs/ac)
		inline double HERBBIOMASS() { return herbBiomass; }

		inline double PRIMARYPRODUCTION() { return primaryProduction; }
		inline double HERBHOLDOVER() { return herbHoldoverBiomass; }

		// Total shrub biomass (lbs/ac)
		inline double SHRUBBIOMASS() { return shrubBiomass * GRAMS_TO_POUNDS; }
		// Total biomass (shrubs + herbs) (lbs/ac)
		inline double TOTALBIOMASS() { return totalBiomass; }
		// Collection of all calculated fuels values
		//inline std::map<std::string, double> TOTALFUELSCOLLECTION() { return totalFuels; }
		// Fuel model for the plot
		//inline int FBFM() { return calcFBFM == 0 ? defaultFBFM : calcFBFM; }

		inline int CURRENT_SUCCESSION_STAGE() { return currentStage; }
		inline int TIME_IN_SUCCESSION_STAGE() { return timeInSuccessionStage; }
		inline string CURRENT_STAGE_TYPE() { return currentStageType; }
		inline int YEAR_OFFSET() { return startingYearsOffset; }

		const float GRAMS_TO_POUNDS = 0.00220462f;

		void push_shrub(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		void push_shrub(RVS::DataManagement::SppRecord* record);

		// Get NDVI for the requested level
		double getNDVI(string level, bool useRand);
		// Get precipitation for the requested level
		double getPPT(string level, bool useRand);

	private:
		int plot_id;
		std::string plot_name;
		int evt_num;
		std::string evt_name;
		int bps_num;
		string bps_model_num;
		int fallback_bps_num;
		int huc;
		std::string grp_id;
		double latitude;
		double longitude;

		double lower_confidence;
		double upper_confidence;
		double s2y;

		// Average cover-weighted shrub height
		double shrubHeight;
		// Total shrub cover 
		double shrubCover;
		// Average herb height
		double herbHeight;
		// Total herb cover 
		double herbCover;
		// Total biomass (herbs + shrubs) (lbs/ac)
		double totalBiomass;
		// Herb biomass for whole plot, including holdover (lbs/ac)
		double herbBiomass;
		// Herb holdover biomass (standing dead)
		double herbHoldoverBiomass;

		double primaryProduction;
		// Shrub biomass for whole plot (g/ac)
		double shrubBiomass;

		// All calculated fuels records, mapped to the type
		//std::map<std::string, double> totalFuels;
		// Sum of 1, 10, 100 hour shrub fuels (g)
		//double shrubFuels;
		// Herb fuel of the plot. Calculation :: production + holdover (lbs/ac)
		//double herbFuels;
		//int defaultFBFM;	// Default FBFM. Used if FBFM calculation fails
		//int calcFBFM;		// Calculated FBFM
		//bool dryClimate;	// Dry or humid BPS (true = dry)

		std::vector<RVS::DataManagement::SppRecord*> shrubRecords;   // List of shrub records
		std::vector<double> ndviValues;   // NDVI values for all years to be simulated
		std::vector<double> precipValues; // PPT values for all years to be simulated

		int currentStage = 0;
		string currentStageType;
		int timeInSuccessionStage = 0;
		int startingYearsOffset = 0;

		// Builds the AnalysisPlot by querying the appropriate tables(s) in the database
		void buildAnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		// Get basic fuels information (FBFM, climate)
		//void buildInitialFuels(RVS::DataManagement::DIO* dio);
	};
}
}

#endif