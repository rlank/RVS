

#pragma once

#ifndef BIOMASSEQUATIONS_H
#define BIOMASSEQUATIONS_H

#include <cmath>
#include <string>

namespace RVS
{
namespace Biomass
{
	class BiomassEquations
	{
	public:
		static void getToken(std::string inStr);
		static double eq_PCH(double cf1, double cf2, double height);
	};
}
}

#endif

