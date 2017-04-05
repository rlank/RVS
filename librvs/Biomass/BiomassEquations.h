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
#include <map>
#include <stack>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

using namespace std;

namespace RVS
{
namespace Biomass
{
	class BiomassEquations
	{
	public:
		static double eq_BAT(int equationNumber, double* coefs, map<string, double>* params);
		static double eq_PCH(double cf1, double cf2, double height);
		
	private:
		static double eq_165(double cf1, double cf2, double cover);
		static double eq_201(double cf1, double cf2, double cover);
		static double eq_202(double cf1, double cf2, double cover);
		static double eq_636(double cf1, double cf2, double length, double width, double height);
		static double eq_636_2(double cf1, double cf2, double vol);
		static double eq_743(double cf1, double cf2, double cover, double height);
		static double eq_998(double cf1, double cf2, double width);
		static double eq_999(double cf1, double cf2, double cf3, double length, double width);
		static double eq_1000(double cf1, double cf2, double cf3, double cf4, double length, double width, double height);
		static double eq_1002(double cf1, double cf2, double vol);
		static double eq_1008(double cf1, double cf2, double cf3, double cf4, double length, double width, double height);
		static double eq_1012(double cf1, double cf2, double cf3, double p1, double p2);
		static double eq_1058(double cf1, double cf2, double cf3, double p1);
		static double eq_basicBAT(double cf1, double cf2, double length, double width);
		static double eq_1153(double cf1, double cf2, double cf3, double length, double width, double height);
		static double eq_1160(double cf1, double cf2, double p1);
		static double eq_1161(double cf1, double cf2, double p1);

		static double shunt(std::string equation, std::map<std::string, double> variables);
	};
}
}

#endif

