#pragma once


#include "Plant.h"



namespace rvs
{

class Herb :
	public Plant
	{
	friend class HerbFactory;

	public:
		
		virtual ~Herb();

		// Return a single plant's biomass calculation
		virtual const double SINGLEBIOMASS();
		// Return total biomass of this plant for an acre
		virtual const double BIOMASS();

		inline const double LOWER_BOUND() { return lower_confidence; }
		inline const double UPPER_BOUND() { return upper_confidence; }
		inline const double S2Y() { return s2y; }

	private:
		Herb();

		double lower_confidence;
		double upper_confidence;
		double s2y;
		double primary_production;
		double holdover;
	};


}