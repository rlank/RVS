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
namespace RVS { namespace Fuels   { class FuelsDriver;   } }

namespace RVS
{
namespace DataManagement
{
	class AnalysisPlot
	{
		friend class RVS::Biomass::BiomassDriver;
		friend class RVS::Fuels::FuelsDriver;

	public:
		AnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		virtual ~AnalysisPlot(void);

		inline int PLOT_ID() { return plot_id; }
		inline int EVT_NUM() { return evt_num; }
		inline int BPS_NUM() { return bps_num; }
		inline int BPS_NUM(bool useFallback) { return useFallback ? fallback_bps_num : bps_num; }
		inline int HUC() { return huc; }

		// Collection of shrub records. Some plots will only have a single record, others many.
		inline std::vector<RVS::DataManagement::SppRecord*>* SHRUB_RECORDS() { return &shrubRecords; }

		// Average shrub height (cm)
		inline double SHRUBHEIGHT() { return shrubHeight; }
		// Total shrub cover (%)
		inline double SHRUBCOVER() { return shrubCover; }
		// Total herb biomass (lbs/ac)
		inline double HERBBIOMASS() { return herbBiomass; }
		// Total shrub biomass (lbs/ac)
		inline double SHRUBBIOMASS() { return shrubBiomass * GRAMS_TO_POUNDS; }
		// Total biomass (shrubs + herbs) (lbs/ac)
		inline double TOTALBIOMASS() { return totalBiomass; }
		// Collection of all calculated fuels values
		inline std::map<std::string, double> TOTALFUELSCOLLECTION() { return totalFuels; }
		// Fuel model for the plot
		inline int FBFM() { return calcFBFM == 0 ? defaultFBFM : calcFBFM; }

		const float GRAMS_TO_POUNDS = 0.00220462f;

		void push_shrub(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);

		// Get NDVI for the requested year (starting at 0)
		double getNDVI(int year);
		// Get precipitation for the requested year (starting at 0)
		double getPPT(int year);

	private:
		int plot_id;
		int evt_num;
		std::string evt_name;
		int bps_num;
		int fallback_bps_num;
		int huc;

		// Average cover-weighted shrub height
		double shrubHeight;
		// Total shrub cover 
		double shrubCover;
		// Total biomass (herbs + shrubs) (lbs/ac)
		double totalBiomass;
		// Herb biomass for whole plot, including holdover (lbs/ac)
		double herbBiomass;
		// Herb holdover biomass (standing dead)
		double herbHoldoverBiomass;
		// Shrub biomass for whole plot (g/ac)
		double shrubBiomass;

		// All calculated fuels records, mapped to the type
		std::map<std::string, double> totalFuels;
		// Sum of 1, 10, 100 hour shrub fuels (g)
		double shrubFuels;
		// Herb fuel of the plot. Calculation :: production + holdover (lbs/ac)
		double herbFuels;
		int defaultFBFM;	// Default FBFM. Used if FBFM calculation fails
		int calcFBFM;		// Calculated FBFM
		bool dryClimate;	// Dry or humid BPS (true = dry)

		std::vector<RVS::DataManagement::SppRecord*> shrubRecords;   // List of shrub records
		std::vector<double> ndviValues;   // NDVI values for all years to be simulated
		std::vector<double> precipValues; // PPT values for all years to be simulated

		// Builds the AnalysisPlot by querying the appropriate tables(s) in the database
		void buildAnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		// Get basic fuels information (FBFM, climate)
		void buildInitialFuels(RVS::DataManagement::DIO* dio);
	};
}
}

#endif