#pragma once

#include <memory>

#include "Biomass.h"
#include "Plant.h"

using namespace std;

namespace rvs
{

class PlantFactory
{
public:
	PlantFactory();
	~PlantFactory();

	virtual shared_ptr<Plant> create(Biomass bio) = 0;
};

}
