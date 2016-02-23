#include "SimBuilder.h"


rvs::SimBuilder::SimBuilder()
{
}


rvs::SimBuilder::~SimBuilder()
{
}

rvs::Simulator rvs::SimBuilder::create(int years, string climate)
{
	Simulator sim = Simulator();
	
	sim.sim_years = years;
	sim.climate = climate; 

	sim.bio = Biomass();
	sim.succ = Succession();


	return sim;
}