/// ********************************************************** ///
/// Name: AnalysisPlot.h                                       ///
/// Desc: Holds a vector of all records in an analysis plot    ///
/// Base Class(es): none                                       ///
/// ********************************************************** ///

#pragma once

#include <string>
#include <queue>
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
		virtual ~AnalysisPlot(void);

		inline const int PLOT_ID() { return plot_id; }
		inline const string PLOT_NAME() { return plot_name; }
		inline const int EVT_NUM() { return evt_num; }
		inline const int BPS_NUM() { return bps_num; }
		inline const string BPS_MODEL_NUM() { return bps_model_num; }
		inline const string GRP_ID() { return grp_id; }
		inline const bool ISDRY() { return dryClimate; }

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

		inline void HERB_RESET_TEST_ONLY() { previousHerbProductions[0] = 0; }

		// Total herb biomass (lbs/ac)
		inline double HERBBIOMASS() { return herbBiomass; }
		inline const double RAWPRODUCTION() { return rawProduction; }
		inline double PRIMARYPRODUCTION() { return primaryProduction; }
		inline double HERBHOLDOVER() { return herbHoldoverBiomass; }

		// Total shrub biomass (lbs/ac)
		inline double SHRUBBIOMASS() { return shrubBiomass * GRAMS_TO_POUNDS; }

		inline double SHRUBSINGLESTEM() { return shrubAvgStem * GRAMS_TO_POUNDS; }
		// Total biomass (shrubs + herbs) (lbs/ac)
		inline double TOTALBIOMASS() { return totalBiomass; }

		// Fuels returns.  CONVERTS TO LBS/AC
		inline const double SHRUB_1HR_WB() { return shrub1HourWB * GRAMS_TO_POUNDS; }
		inline const double SHRUB_1HR_FOLIAGE() { return shrub1HourFoliage * GRAMS_TO_POUNDS; }
		inline const double SHRUB_10HR() { return shrub10Hour * GRAMS_TO_POUNDS; }
		inline const double SHRUB_100HR() { return shrub100Hour * GRAMS_TO_POUNDS; }
		inline const double SHRUB_1000HR() { return shrub1000Hour * GRAMS_TO_POUNDS; }
		inline const double FUEL_TOTAL_1HR() { return total1HrFuel * GRAMS_TO_POUNDS; }
		inline const double HERB_FUEL() { return herbFuel * GRAMS_TO_POUNDS; }
		inline const double FUEL_TOTAL() { return (shrub1HourWB + shrub1HourFoliage + shrub10Hour + shrub100Hour + shrub1000Hour + herbFuel) * GRAMS_TO_POUNDS; }

		
		// Fuel model for the plot
		inline int FBFM() { return calcFBFM == 0 ? defaultFBFM : calcFBFM; }
		inline const string FBFM_NAME() { return fbfmName; }

		inline int CURRENT_SUCCESSION_STAGE() { return currentStage; }
		inline string CURRENT_STAGE_TYPE() { return currentStageType; }
		inline int PLOT_AGE() { return plotAge; }

		const float GRAMS_TO_POUNDS = 0.00220462f;
		const float POUNDS_TO_GRAMS = 453.592f;

		void push_shrub(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		void push_shrub(RVS::DataManagement::SppRecord* record);
		void update_shrubvalues();


		// Get NDVI for the requested level
		double getNDVI(string level, bool useRand);
		// Get precipitation for the requested level
		double getPPT(string level, bool useRand);

		std::vector<RVS::Disturbance::DisturbAction> getDisturbancesForYear(int year);
		inline void setDisturbances(vector<RVS::Disturbance::DisturbAction> dists) { disturbances = dists; }
		// Returns the reduction amount in lbs/ac from grazing
		inline double BIOMASS_DISTURB_AMOUNT() { return biomassReductionTotal * GRAMS_TO_POUNDS; }

	private:
		int plot_id;
		std::string plot_name;
		int evt_num;
		std::string evt_name;
		int bps_num;
		string bps_model_num;
		int fallback_bps_num;
		std::string grp_id;
		double latitude;
		double longitude;

		bool doNotModel;

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
		// Raw primary production, not reduced by shrub cover
		double rawProduction;
		// Production allocated for herbs, reduced by shrub cover
		double primaryProduction;
		// Hold 3 previous years of primary production for holdover calculation
		double* previousHerbProductions;
		// Shrub biomass for whole plot (g/ac)
		double shrubBiomass;

		double shrubAvgStem;

		// Sum of 1, 10, 100 hour shrub fuels (g)
		//double shrubFuels;
		// Herb fuel of the plot. Calculation :: production + holdover (lbs/ac)
		//double herbFuels;
		
		//////////////// FUELS //////////////////
		//all values held in grams or grams/ac///
		/////////////////////////////////////////
		int defaultFBFM;	// Default FBFM. Used if FBFM calculation fails
		int calcFBFM;		// Calculated FBFM
		bool dryClimate;	// Dry or humid BPS (true = dry)
		string fbfmName;

		double fuel1HrProp;      // 1 Hr wood + bark proportion
		double fuelFoilageProp;  // 1 Hr foliage proportion
		double fuel10HrProp;     // 10 Hr wood + bark proportion
		double fuel100HrProp;    // 100 Hr wood + bark proportion

		double shrub1HourWB;
		double shrub1HourFoliage;
		double shrub10Hour;
		double shrub100Hour;
		double shrub1000Hour;
		double total1HrFuel;   // 1 Hr Herbs, 1 Hr shrub W+B, 1 Hr shrub foliage
		double herbFuel; 

		std::vector<RVS::DataManagement::SppRecord*> shrubRecords;   // List of shrub records
		std::vector<double> ndviValues;   // NDVI values for all years to be simulated
		std::vector<double> precipValues; // PPT values for all years to be simulated

		int currentStage = 0;
		string currentStageType;
		int plotAge = 0;
		int timeInHerbStage = 0;

		// Builds the AnalysisPlot by querying the appropriate tables(s) in the database
		void buildAnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		// Get basic fuels information (FBFM, climate)
		void buildInitialFuels(RVS::DataManagement::DIO* dio);

		vector<RVS::Disturbance::DisturbAction> disturbances;
		bool disturbed = false;
		bool burned = false;
		// Total biomass to be removed via disturbance in g/ac
		double biomassReductionTotal;

	};
}
}
