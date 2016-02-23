#pragma once
#include "Plant.h"

namespace rvs
{
class Shrub :
	public Plant
	{

	friend class ShrubFactory;
	
	public:
	
		virtual ~Shrub();

		// Return a single plant's biomass calculation
		virtual const double SINGLEBIOMASS();
		// Return total biomass of this plant for an acre
		virtual const double BIOMASS();

		// WID (cm)
		inline double WIDTH() { return width; }
		// LEN (cm)
		inline double LENGTH() { return width; }
		// stems per acre (count)
		inline double STEMSPERACRE() { return stemsPerAcre; }

		inline int PCHEQNUM() { return pchEqNum; }
		inline int BATEQNUM() { return batEqNum; }

		// Return a parameter (length, width, height) by name
		double requestValue(std::string parameterName);

	private:
		Shrub();

		double singleBiomass;
		double width;  // Average leaf width (cm)
		double stemsPerAcre;  // calculated value (in Biomass)
		int pchEqNum;
		int batEqNum;
	};
}
