#include "FuelsEquations.h"

double RVS::Fuels::FuelsEquations::calc1Hr(float height, float cover, float herbs)
{
	return 0;
}

double RVS::Fuels::FuelsEquations::calc1Hr(double bio, float herbs)
{
	double fuel = 1.1515406 + 0.321030 * bio;
	fuel += herbs;
	return fuel;
}

double RVS::Fuels::FuelsEquations::calc10Hr(float height, float cover)
{
	return 0;
}

double RVS::Fuels::FuelsEquations::calc10Hr(double bio)
{
	double fuel = -2.054662 + 0.448055 * bio;
	return fuel;
}

double RVS::Fuels::FuelsEquations::calc100Hr(float height, float cover)
{
	return 0;
}

double RVS::Fuels::FuelsEquations::calc100Hr(double bio)
{
	double fuel = 3.567246 + (-0.017713 * bio) + (bio * bio * 0.000169);
	return fuel;
}