#include "ShrubFactory.h"


rvs::ShrubFactory::ShrubFactory()
{
}


rvs::ShrubFactory::~ShrubFactory()
{
}

shared_ptr<rvs::Plant> rvs::ShrubFactory::create(Biomass bio)
{
	shared_ptr<Plant> shrubPtr(new Shrub());

	shrubPtr->calcBiomass = &(bio.calcShrubBiomass);

	return shrubPtr;
}
