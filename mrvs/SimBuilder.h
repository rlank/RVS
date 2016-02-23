#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Biomass.h"
#include "Simulator.h"
#include "Succession.h"

namespace rvs
{
class SimBuilder
{

	public:
		
		virtual ~SimBuilder();

		static Simulator create(int years, string climate);
	private:
		SimBuilder();

};
}