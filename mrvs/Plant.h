#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>

using namespace std;

namespace rvs
{

	class Plant
	{
	public:
		Plant();
		~Plant();
		/// General Record parameters ///

		// Dominant species name
		inline const string DOM_SPP() { return dom_spp; }
		// Dominant species PLANTS code
		inline const string SPP_CODE() { return spp_code; }
		// HT (cm)
		inline const double HEIGHT() { return height; }
		// COV (%)
		inline const double COVER() { return cover; }

		inline const double BIOMASS() { return biomass; }

		// Caculate total biomass of this plant for an acre
		const double(*calcBiomass)(const map<string, shared_ptr<void>> args) = 0;

		// Calculate 1, 10, 100, 1000 hour fuel loads for this plant
		const map<string, double>(*FUEL)() = 0;

		// Do a disturbance (reduction) to the plant
		const void(*DISTURB)() = 0;

		// Grow this plant for succession
		const double(*GROW)() = 0;

	protected:
		/// General variables ///

		string dom_spp;  // Dominant species name
		string spp_code; // Dominant species PLANTS code
		double height; // Crown height (cm)
		double cover;  // ABSOLUTE cover (0-100) %
		double width;  // Average leaf width (cm)
		double biomass;
	};
}
