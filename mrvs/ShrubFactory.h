#pragma once

#include <memory>

#include "PlantFactory.h"
#include "Biomass.h"
#include "Shrub.h"

namespace rvs
{

class ShrubFactory :
	public PlantFactory
{
public:
	ShrubFactory();
	virtual ~ShrubFactory();

	virtual shared_ptr<Plant> create(Biomass bio);
};

}
