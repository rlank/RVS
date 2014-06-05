/// ********************************************************** ///
/// Name: SppRecord.h                                          ///
/// Desc: 
/// Base Class(es): none                                       ///
/// Requires: map, string, RVS_TypeDefs                        ///
/// ********************************************************** ///

#pragma once

#ifndef SPP_RECORD_H
#define SPP_RECORD_H

#include <string>
#include <vector>

#include <boost/any.hpp>

#include "../RVSDBNAMES.h"
#include "DIO.h"
#include "RVS_TypeDefs.h"

using namespace RVS::DataManagement;

namespace RVS
{
namespace DataManagement
{
	class SppRecord
	{
	public:
		SppRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		SppRecord(const SppRecord &old_evt);
		virtual ~SppRecord(void);

		// General Record getters/setters parameters //
		inline std::string DOM_SPP() { return dom_spp; }
		inline std::string SPP_CODE() { return spp_code; }
		inline double HEIGHT() { return height; }
		inline double COVER() { return cover; }
		inline double WIDTH() { return width; }
		inline void SET_WIDTH(double width) { this->width = width; }
		inline double STEMSPERACRE() { return stemsPerAcre; }
		inline void SET_STEMSPERACRE(double stemsPerAcre) { this->stemsPerAcre = stemsPerAcre; }

		// Biomass getters/setters params (results)
		inline double SHRUBBIOMASS() { return shrubBiomass; }
		inline void SET_SHRUBBIOMASS(double biomass) { shrubBiomass = biomass; }
		
		double requestValue(std::string parameterName);
		                                                                                                                                                                                                                                                                                                                                                                                                                                                      
	protected:
		// General variables //
		std::string dom_spp;
		std::string spp_code;
		double height; // Crown height
		double cover;  // ABSOLUTE cover
		double width;  // Average leaf width
		double stemsPerAcre;
	    
		// General protected functions //
		virtual void initialize_object();
		virtual void buildRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);

		// Biomass values
		double shrubBiomass;


		// Fuels collection
		std::map<std::string, double> fuels;
	};
}
}

#endif