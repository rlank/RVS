#include "HerbFactory.h"


rvs::HerbFactory::HerbFactory()
{
}


rvs::HerbFactory::~HerbFactory()
{
}

shared_ptr<rvs::Plant> rvs::HerbFactory::create(Biomass bio)
{
	shared_ptr<Plant> shrubPtr(new Herb());

	shrubPtr->calcBiomass = &(bio.calcHerbBiomass);

	return shrubPtr;
}