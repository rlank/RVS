#pragma once

#include <map>
#include <memory>
#include <string>

using namespace std;

namespace rvs
{
class Biomass
	{


	public:
		Biomass();
		virtual ~Biomass();

		const double calcShrubBiomass(const map<string, shared_ptr<void>> args);
		const double calcHerbBiomass(const map<string, shared_ptr<void>> args);
	private:

		void calcConfidence(int year, double biomass, double* lower, double* upper, double* level);
		// Given cover, calculate the holdover amount
		double calcHerbHoldover(double totalShrubCover);
		double calcAttenuation(double herbBiomass);
		void growHerbs(double* herbCover, double* herbHeight, double oldBiomass);

		double calc_s2b(double* lnNDVI, double* lnPPT);
		double calc_s2b(string* grp_id, double* lnNDVI, double* lnPPT);
		double** matrix_mult(double** A, int aRow, int aCol, double** B, int bRow, int bCol);
		double** matrix_trans(double** A, int aRow, int aCol);
		double** generate_dummy_variables(int index, double* lnPPT, double* lnNDVI);

	};

}