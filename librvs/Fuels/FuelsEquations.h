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
#include <string>

namespace RVS
{
namespace Fuels
{

	class FuelsEquations
	{
	public:
		// Equation 6107 Biomass crown live, timelag fuel 1 hr + herbs
		static double calc1Hr(float height, float cover, float herbs);
		// // Equation 6074 SYAL Fuel Size 1hr + herbs
		static double calc1Hr(double bio, float herbs);

		// Equation 6095 Biomass crown live, timelag fuel 10 hr
		static double calc10Hr(float height, float cover);
		// Equation 6076 SYAL Fuel Size 10hr
		static double calc10Hr(double bio);

		// Equation 6089 Biomass crown live, timelag fuel 100 hr
		static double calc100Hr(float heigt, float cover);
		// Equation 5098 High Shrub Fuel size 100 hr
		static double calc100Hr(double bio);
	};
}
}

#endif

