#include "BiomassEquations.h"

double RVS::Biomass::BiomassEquations::eq_PCH(double cf1, double cf2, double height)
{
	double result = 0;
	result = cf1 + cf2 * log10(height);
	result = pow(10, result);
	return result;
}

double RVS::Biomass::BiomassEquations::eq_pch_acre(double pch)
{
	double pch_acre = (pch / 1000) * PCH_MULT;
	return pch_acre;
}



