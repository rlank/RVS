#include "Simulator.h"


rvs::Simulator::Simulator()
{
}


rvs::Simulator::~Simulator()
{
}

void rvs::Simulator::simulate()
{
	for (int y = 0; y < sim_years; y++)
	{
		for (vector<Plot>::iterator plotIt = plot_list.begin(); plotIt != plot_list.end(); ++plotIt)
		{
			Plot currentPlot = *plotIt;

			shared_ptr<vector<shared_ptr<Plant>>> plotPlants = currentPlot.PLANT_RECORDS();
			for (vector<shared_ptr<Plant>>::iterator plantIt = plotPlants->begin(); plantIt != plotPlants->end(); ++plantIt)
			{
				shared_ptr<Plant> currentPlant = *plantIt;

				map<string, shared_ptr<void>> args = map<string, shared_ptr<void>>();
				string nm = "shrubcover";
				shared_ptr<void> val(new double(.2));

				args.insert(pair<string, shared_ptr<void>>(nm, val));

				currentPlant->calcBiomass(args);
				currentPlant->
			}
		}
	}
}