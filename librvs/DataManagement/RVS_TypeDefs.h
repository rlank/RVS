/// ********************************************************** ///
/// Name: RVS_TypeDefs.h                                       ///
/// Desc: Containers for data type and enums needed throughout ///
/// the RVS modules.                                           ///
/// Base Class(es): none                                       ///
/// Requires:                                                  ///
/// ********************************************************** ///

#pragma once

#ifndef RVS_TYPEDEFS_H
#define RVS_TYPEDEFS_H

#include <map>
#include <string>

#include "DataTable.h"

namespace RVS
{
namespace DataManagement
{
	enum Lifeform 
	{
		forb, 
		graminoid,
		herb, 
		liana,
		lichenous, 
		nonvascular, 
		shrub,
		subshrub, 
		tree, 
		unk, 
		vine };
	
}
namespace Biomass
{
    /// Enumeration of return units for shrub allometries
    enum BiomassReturnType
    {
        AFN = 1, // New all-sided leaf area (cm^2)
        AFT = 2, // Total all-sided leaf area (cm^2)
        BAE = 3, // Total aboveground biomass (g) (more specific definition than BAT)
        BAL = 4, // Aboveground live biomass (g)
        BAP = 5, // Aboveground live biomass, wood + bark (g)
        BAT = 6, // Total aboveground biomass (g)
        BBD = 7, // Dead branch biomass (g)
        BBL = 8, // Live branch biomass (g)
        BFN = 9, // New foliage biomass (g)
        BFT = 10, // Total foliage biomass (g)
        BRL = 11, // Total root biomass (g)
        BST = 12, // Total stem biomass (g)
        BUN = 13, // Nodule biomass, underground (g)
        PCH = 14, // Projected area, crown, horizontal surface (cm^2)
        PFN = 15, // New projected leaf area (cm^2)
        PFT = 16, // Total projected leaf area (cm^2)
        VST = 17, // Bole volume, bark and wood (cm^3)
        RNUL = 0  // None/void type
    };

    /// Enumeration of parameter input types for shrub allometries
    enum BiomassVarUnits
    {
        BIO = 21, // Total aboveground biomass
        COV = 22, // Cover (% in decimal i.e. .4 for 40%)
        DBA = 23, // Stem basal diameter (cm)
        HT = 24,  // Height (cm)
        LEN = 25, // Length or Canopy Length (cm)
        NUM = 26, // Number of basal stems (count)
        SPA = 27, // Sapwood cross-section area at ground level (cm)
        VOL = 28, // Volume (dm^3) (if not given directly, VOL = LEN * WID * HT
        WID = 29, // Width (cm)
        VNUL = 20  // None/Void type
    };

    /// Enumeration of herb lookup level
    enum BiomassLookupLevel { high, medium, low };

	inline char* ENUMPARSE(int i)
		{
			switch (i)
			{
				case 0: return "NUL";
				case 1: return "AFN";
				case 2: return "AFT";
				case 3:	return "BAE";
				case 4:	return "BAL";
				case 5:	return "BAP";
				case 6: return "BAT";
				case 7: return "BBD";
				case 8: return "BBL";
				case 9: return "BFN";
				case 10: return "BFT";
				case 11: return "BRL";
				case 12: return "BST";
				case 13: return "BUN";
				case 14: return "PCH";
				case 15: return "PFN";
				case 16: return "PFT";
				case 17: return "VST";
				case 20: return "NUL";
				case 21: return "BIO";
				case 22: return "COV";
				case 23: return "DBA";
				case 24: return "HT";
				case 25: return "LEN";
				case 26: return "NUM";
				case 27: return "SPA";
				case 28: return "VOL";
				case 29: return "WID";
				default: return "Not recognized";
			}
		};

	static std::map<std::string, BiomassVarUnits> BiomassVarUnitsMap = 
	{
		{ "BIO", BIO },
		{ "COV", COV },
		{ "DBA", DBA },
		{ "HT", HT },
		{ "LEN", LEN },
		{ "NUM", NUM },
		{ "SPA", SPA },
		{ "VOL", VOL },
		{ "WID", WID },
		{ "", VNUL }
	};
}
}

#endif