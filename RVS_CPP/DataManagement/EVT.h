/// ********************************************************** ///
/// Name: EVT.h                                                ///
/// Desc: Master EVT class. All EVT objects inherit from this  ///
/// class. This class is abstract. Use the 'interface' classes ///
/// Base Class(es): none                                       ///
/// Requires: map, string, RVS_TypeDefs                        ///
/// ********************************************************** ///

//$$ TODO: Finish copy constructor? (is this even needed?)
//$$ TODO: Start coding succession parameters from spreadsheet Matt sent

#pragma once

#ifndef EVT_H
#define EVT_H

#include <map>
#include <string>

#include "RVS_TypeDefs.h"

namespace RVS
{
namespace DataManagement
{
	/// Master (abstract) EVT class. All EVT objects inherit from this class. 
	class EVT
	{
	public:
		EVT(void);
		EVT(const EVT& old_evt);
		virtual ~EVT(void);

		// General EVT parameters //
		virtual inline int EVT_NUM() { return evt_num; };
		virtual inline std::string BPS_NUM() { return bps_num; };
		virtual inline Lifeform LIFEFORM() { return lifeform; };
		virtual inline std::string DOM_SPP() { return dom_spp; };
		virtual inline std::string SPP_CODE() { return spp_code; };
		virtual inline float HEIGHT() { return height; };
		virtual inline float COVER() { return cover; };
		virtual inline int SAMPLEPLOTS() { return samplePlots; };
		virtual inline int TOTALSAMPLEPLOTS() { return totalSamplePlots; };

		// General EVT public functions //
		virtual void base_buildEVT(RVS::DataManagement::DataTable &dt, int &row_num);

		// Biomass EVT parameters //
		virtual float PERCENT_DOM() = 0;
		virtual Biomass::BiomassVarUnits PA1_CODE() = 0;
		virtual float PA1_VAL() = 0;
		virtual Biomass::BiomassVarUnits PA2_CODE() = 0;
		virtual float PA2_VAL() = 0;
		virtual Biomass::BiomassVarUnits PA3_CODE() = 0;
		virtual float PA3_VAL() = 0;
		virtual float COEF1() = 0;
		virtual float COEF2() = 0;
		virtual float COEF3() = 0;
		virtual float COEF4() = 0;
		virtual Biomass::BiomassReturnType RETURN_TYPE() = 0;
		virtual std::map<RVS::Biomass::BiomassVarUnits, std::string> PARAMETERS() = 0;

		// Succession EVT parameters //
		                                                                                                                                                                                                                                                                                                                                                                                                                                                      
	protected:
		// General EVT variables //
		int evt_num;
		std::string bps_num;
		DataManagement::Lifeform lifeform;
		std::string dom_spp;
		std::string spp_code;
		float height;
		float cover;
		int samplePlots;
		int totalSamplePlots;
	    
		// General EVT protected functions //
		virtual void initialize_object();
		

		// BIOMASS EVT variables //
		float percentDom;
	    Biomass::BiomassVarUnits PA1_Code;
	    float PA1_Val;
	    Biomass::BiomassVarUnits PA2_Code;
	    float PA2_Val;
	    Biomass::BiomassVarUnits PA3_Code;
	    float PA3_Val;
	    float coef1;
	    float coef2;
	    float coef3;
	    float coef4;
		Biomass::BiomassReturnType returnType;
		std::map<RVS::Biomass::BiomassVarUnits, std::string> parms;

		// Succession EVT variables //

	};
}
}

#endif