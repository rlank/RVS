#pragma once



#include <memory>

#include "Biomass.h"
#include "Herb.h"
#include "Plant.h"
#include "PlantFactory.h"

namespace rvs
{
	class HerbFactory :
		public PlantFactory
	{
	public:
		HerbFactory();
		virtual ~HerbFactory();

		virtual shared_ptr<Plant> create(Biomass bio);
	};

}