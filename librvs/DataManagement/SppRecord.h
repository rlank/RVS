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
		virtual ~SppRecord(void);

		/// General Record parameters ///

		// Dominant species name
		inline std::string DOM_SPP() { return dom_spp; }
		// Dominant species PLANTS code
		inline std::string SPP_CODE() { return spp_code; }
		inline double HEIGHT() { return height; }
		inline double COVER() { return cover; }
		inline double WIDTH() { return width; }
		inline double LENGTH() { return width; }
		inline double STEMSPERACRE() { return stemsPerAcre; }
		inline int PCHEQNUM() { return pchEqNum; }
		inline int BATEQNUM() { return batEqNum; }
		
		// Biomass result for shrub (individual biomass)
		inline double SHRUBBIOMASS() { return shrubBiomass; }
		
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

		// Biomass values
		double shrubBiomass;  // grams
		double exShrubBiomass;  // lbs/ac
		int pchEqNum;
		int batEqNum;


		// Fuels collection
		std::map<std::string, double> fuels;
		double crl1;  // Percent live, 1hr, calculated value (in Fuels) (0-100)
		double crl2;  // Percent live, 10hr, calculated value (in Fuels) (0-100)
		double crl3;  // Percent live, 100hr, calculated value (in Fuels) (0-100)


	};
}
}

#endif