#pragma once

#include <string>
#include <vector>

#include "Plant.h"

using namespace std;

namespace rvs
{
class Plot
	{

	public:
		virtual ~Plot();

		inline const int PLOT_ID() { return plot_id; }
		inline const string PLOT_NAME() { return plot_name; }
		inline const int EVT_NUM() { return evt_num; }
		inline const int BPS_NUM() { return bps_num; }
		inline const string BPS_MODEL_NUM() { return bps_model_num; }
		inline const string GRP_ID() { return grp_id; }

		inline const double LATITUDE() { return latitude; }
		inline const double LONGITUDE() { return longitude; }

		inline shared_ptr<vector<shared_ptr<Plant>>> PLANT_RECORDS() { return plant_records; }

		inline int CURRENT_SUCCESSION_STAGE() { return currentStage; }
		inline int TIME_IN_SUCCESSION_STAGE() { return timeInSuccessionStage; }
		inline string CURRENT_STAGE_TYPE() { return currentStageType; }

		// Get NDVI for the requested level
		double getNDVI(string level, bool useRand);
		// Get precipitation for the requested level
		double getPPT(string level, bool useRand);

	private:
		Plot();

		int plot_id;
		std::string plot_name;
		int evt_num;
		std::string evt_name;
		int bps_num;
		string bps_model_num;
		string grp_id;
		double latitude;
		double longitude;

		// Total biomass (all plants) (lbs/ac)
		double totalBiomass;

		shared_ptr<vector<shared_ptr<Plant>>> plant_records;

		vector<double> ndviValues;   // NDVI values for all climates to be simulated
		vector<double> precipValues; // PPT values for all climates to be simulated
		
		int currentStage = 0;
		string currentStageType;
		int timeInSuccessionStage = 0;
	};
}