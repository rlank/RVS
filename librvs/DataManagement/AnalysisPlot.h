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

		// Collection of shrub records. Some plots will only have a single record, others many.
		inline std::vector<RVS::DataManagement::SppRecord*>* SHRUB_RECORDS() { return &shrubRecords; }

		inline double SHRUBCOVER() { return shrubCover; }
		inline double HERBBIOMASS() { return herbBiomass; }
		inline double SHRUBBIOMASS() { return shrubBiomass; }
		inline double TOTALBIOMASS() { return totalBiomass; }
		inline std::map<std::string, double> TOTALFUELS() { return totalFuels; }
		inline int FBFM() { return calcFBFM == 0 ? defaultFBFM : calcFBFM; }


		// Get NDVI for the requested year (starting at 0)
		double getNDVI(int year);
		// Get precipitation for the requested year (starting at 0)
		double getPPT(int year);

	private:
		int plot_id;
		int evt_num;
		std::string evt_name;
		int bps_num;

		// Total shrub cover
		double shrubCover;
		// Total biomass (herbs + shrubs)
		double totalBiomass;
		// Herb biomass for whole plot
		double herbBiomass;
		// Shrub biomass for whole plot
		double shrubBiomass;

		// All calculated fuels records, mapped to the type
		std::map<std::string, double> totalFuels;
		int defaultFBFM;	// Default FBFM. Used if FBFM calculation fails
		int calcFBFM;		// Calculated FBFM
		bool dryClimate;	// Dry or humid BPS (true = dry)

		std::vector<RVS::DataManagement::SppRecord*> shrubRecords;   // List of shrub records
		std::vector<double> ndviValues;   // NDVI values for all years to be simulated
		std::vector<double> precipValues; // PPT values for all years to be simulated

		// Builds the AnalysisPlot by querying the appropriate tables(s) in the database
		void buildAnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		// Create shrub records
		void buildShrubRecords(RVS::DataManagement::DIO* dio, int plot_num);

		void buildInitialFuels(RVS::DataManagement::DIO* dio);
	};
}
}

#endif