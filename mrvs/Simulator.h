#pragma once

#include <vector>

#include "Biomass.h";
#include "Succession.h";
#include "Plot.h";


namespace rvs
{
class Simulator
{
	friend class SimBuilder;
	public:
		virtual ~Simulator();

		void simulate();
	private:
		Simulator();
		int sim_years;

		string climate;

		Biomass bio;
		Succession succ;

		vector<Plot> plot_list;
};

}