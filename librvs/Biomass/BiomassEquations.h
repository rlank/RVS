

#pragma once

#ifndef BIOMASSEQUATIONS_H
#define BIOMASSEQUATIONS_H

#include <cmath>
#include <string>

namespace RVS
{
namespace Biomass
{
	static float const PCH_MULT = 4046.85942f;

	class BiomassEquations
	{
	public:
		
		static double eq_PCH(double cf1, double cf2, double height);
		static double eq_pch_acre(double pch);
	};
}
}

#endif

