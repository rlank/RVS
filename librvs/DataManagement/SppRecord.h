/// ********************************************************** ///
/// Name: SppRecord.h                                          ///
/// Desc: Contains information about a specific species record ///
/// associated with an AnalysisPlot. Currently these are all   ///
/// shrub records.                                             ///
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

namespace RVS { namespace Biomass { class BiomassDriver; } }
namespace RVS { namespace Biomass { class BiomassEqDriver; } }
namespace RVS { namespace Fuels   { class FuelsDriver; } }
namespace RVS { namespace Succession { class SuccessionDriver; } }
namespace RVS { namespace Disturbance { class DisturbanceDriver; } }

using namespace RVS::DataManagement;

namespace RVS
{
namespace DataManagement
{
	class SppRecord
	{
		friend class RVS::Biomass::BiomassDriver;
		friend class RVS::Fuels::FuelsDriver;
		friend class RVS::Biomass::BiomassEqDriver;
		friend class RVS::Succession::SuccessionDriver;
		friend class RVS::Disturbance::DisturbanceDriver;

	public:
		SppRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		SppRecord(string spp_code, double height, double cover, string dom_spp);
		virtual ~SppRecord(void);

		/// General Record parameters ///

		// Dominant species name
		inline std::string DOM_SPP() { return dom_spp; }
		// Dominant species PLANTS code
		inline std::string SPP_CODE() { return spp_code; }
		// HT (cm)
		inline double HEIGHT() { return height; } 
		// COV (%)
		inline double COVER() { return cover; } 
		// WID (cm)
		inline double WIDTH() { return width; } 
		// LEN (cm)
		inline double LENGTH() { return width; } 
		// stems per acre (count)
		inline double STEMSPERACRE() { return stemsPerAcre; } 
		inline int PCHEQNUM() { return pchEqNum; }
		inline int BATEQNUM() { return batEqNum; }
		
		// Biomass result for shrub (individual biomass). coverts internal g/ac to lbs/ac
		inline double SHRUB_SINGLE_BIOMASS() { return shrubBiomass * GRAMS_TO_POUNDS; }
		inline double SHRUB_EX_BIOMASS() { return exShrubBiomass * GRAMS_TO_POUNDS; }
		const float GRAMS_TO_POUNDS = 0.00220462f;

		// Fuels results
		
		// Return a parameter (length, width, height) by name
		double requestValue(std::string parameterName);
		                                                                                                                                                                                                                                                                                                                                                                                                                                                      
	protected:
		/// General variables ///

		std::string dom_spp;  // Dominant species name
		std::string spp_code; // Dominant species PLANTS code
		double height; // Crown height (cm)
		double cover;  // ABSOLUTE cover (0-100) %
		double width;  // Average leaf width (cm)
		double stemsPerAcre;  // calculated value (in Biomass)
	    
		// General protected functions //
		virtual void initialize_object();
		virtual void buildRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		virtual void buildRecord(string spp_code, double height, double cover, string dom_spp);

		// Biomass values
		double shrubBiomass;  // grams
		double exShrubBiomass;  // grams
		int pchEqNum;
		int batEqNum;

		// Fuels collection

		double fuel1hr;
		double fuel10hr;
		double fuel100hr;
		double fuel1000hr;
	};
}
}

#endif