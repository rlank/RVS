/// ********************************************************** ///
/// Name: FuelsEquations.h                                     ///
/// Desc: Contains equations for calculating 1, 10, and 100    ///
/// hour fuels. For now this only requires height and cover    ///
/// (and herbaceous biomass for 1Hr) and does no equation      ///
/// parsing.  These return SINGLE STEM values.  Expand using   ///
/// the expand function from EVT                               ///
/// ********************************************************** ///

#ifndef FUELSEQUATIONS_H
#define FUELSEQUATIONS_H

#include <cmath>
#include <map>
#include <string>

namespace RVS
{
namespace Fuels
{

	class FuelsEquations
	{
	public:
		static double calcFuels(int equationType, double* coefs, double* params);
		static double calc1HrLivePercent();
		static double calc10HrLivePercent();
		static double calc100HrLivePercent();

	private:
		static double fuels_eq_1(double cf1, double cf2, double p1);
		static double fuels_eq_2(double cf1, double cf2, double p1);
		static double fuels_eq_13(double cf1, double cf2, double p1);
		static double fuels_eq_15(double cf1, double cf2, double p1, double p2);
		static double fuels_eq_17(double cf1, double cf2, double cf3, double p1, double p2);
		static double fuels_eq_42(double cf1, double cf2, double cf3, double cf4, double p1, double p2, double p3);
		static double fuels_eq_46(double cf1, double cf2, double cf3, double p1, double p2);
	};
}
}

#endif

