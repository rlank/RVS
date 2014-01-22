/// ********************************************************** ///
/// Name: BiomassEquations.h                                   ///
/// Desc: Equations for calculating biomass. These are hard    ///
/// coded for now but will eventually implement an equation    ///
/// parser for reading equations from the database (only to    ///
/// be used if changed to modified to limit DB reads).         ///
/// ********************************************************** ///

#ifndef BIOMASSEQUATIONS_H
#define BIOMASSEQUATIONS_H

#include <cmath>
#include <string>

namespace RVS
{
namespace Biomass
{
	static const float PCH_MULT = 4046.85942f;

	class BiomassEquations
	{
	public:
		static double eq_AFN(double cf1, double cf2, double dba);
		static double eq_AFN(double cf1, double cf2, double cf3, double dba);

		static double eq_PCH(double cf1, double cf2, double height);
		static double eq_pch_acre(double pch);
	};
}
}

#endif

