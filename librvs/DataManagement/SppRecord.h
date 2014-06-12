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

#include "../RVSDBNAMES.h"
#include "DIO.h"
#include "RVS_TypeDefs.h"

namespace RVS { namespace Biomass { class BiomassDriver; } }
namespace RVS { namespace Fuels   { class FuelsDriver; } }

using namespace RVS::DataManagement;

namespace RVS
{
namespace DataManagement
{
	class SppRecord
	{
		friend class RVS::Biomass::BiomassDriver;
		friend class RVS::Fuels::FuelsDriver;
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
		inline double LENGTH() { return width; }
		inline double STEMSPERACRE() { return stemsPerAcre; }
		
		// Biomass getters/setters params (results)
		inline double SHRUBBIOMASS() { return shrubBiomass; }
		
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
		inline void SET_SHRUBBIOMASS(double biomass) { shrubBiomass = biomass; }
		inline void SET_WIDTH(double width) { this->width = width; }
		inline void SET_STEMSPERACRE(double stemsPerAcre) { this->stemsPerAcre = stemsPerAcre; }

		// Fuels collection
		std::map<std::string, double> fuels;
	};
}
}

#endif