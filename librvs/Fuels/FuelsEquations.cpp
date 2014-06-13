#include "FuelsEquations.h"

double RVS::Fuels::FuelsEquations::calcFuels(int equationType, double* coefs, double* params)
{
	double fuel = 0.0;
	switch (equationType)
	{
	case 1:
		fuel = fuels_eq_1(coefs[0], coefs[1], params[0]);
		break;
	case 2:
		fuel = fuels_eq_2(coefs[0], coefs[1], params[0]);
		break;
	case 13:
		fuel = fuels_eq_13(coefs[0], coefs[1], params[0]);
		break;
	case 15:
		fuel = fuels_eq_15(coefs[0], coefs[1], params[0], params[1]);
		break;
	case 17:
		fuel = fuels_eq_17(coefs[0], coefs[1], coefs[2], params[0], params[1]);
		break;
	case 42:
		fuel = fuels_eq_42(coefs[0], coefs[1], coefs[2], coefs[3], params[0], params[1], params[2]);
		break;
	case 46:
		fuel = fuels_eq_46(coefs[0], coefs[1], coefs[2], params[0], params[1]);
		break;
	}

	return fuel;
}

double RVS::Fuels::FuelsEquations::fuels_eq_1(double cf1, double cf2, double p1)
{
	double fuel = cf1 + cf2 * p1;
	return fuel;
}

double RVS::Fuels::FuelsEquations::fuels_eq_2(double cf1, double cf2, double p1)
{
	double fuel = cf1 + cf2 * log(p1);
	fuel = exp(fuel);
	return fuel;
}

double RVS::Fuels::FuelsEquations::fuels_eq_13(double cf1, double cf2, double p1)
{
	double fuel = cf1 + cf2 * p1;
	fuel = exp(fuel);
	return fuel;
}

double RVS::Fuels::FuelsEquations::fuels_eq_15(double cf1, double cf2, double p1, double p2)
{
	double fuel = cf1 + cf2 * p1 * p2;
	return fuel;
}

double RVS::Fuels::FuelsEquations::fuels_eq_17(double cf1, double cf2, double cf3, double p1, double p2)
{
	double fuel = cf1 + cf2 * log(p1) + cf3 * log(p2);
	fuel = exp(fuel);
	return fuel;
}

double RVS::Fuels::FuelsEquations::fuels_eq_42(double cf1, double cf2, double cf3, double cf4, double p1, double p2, double p3)
{
	double fuel = cf1 + cf2 * log(p1) + cf3 * log(p2) + cf4 * log(p3);
	fuel = exp(fuel);
	return fuel;
}

double RVS::Fuels::FuelsEquations::fuels_eq_46(double cf1, double cf2, double cf3, double p1, double p2)
{
	double fuel = cf1 + cf2 * p1 + cf3 * p2;
	return fuel;
}