/// ********************************************************** ///
/// Name: BiomassEVT.h                                         ///
/// Desc: Biomass EVT interface class.                         ///
/// Base Class(es): RVS::DataManagement::EVT                   ///
/// ********************************************************** ///

#pragma once

#ifndef BIOMASS_EVT_H
#define BIOMASS_EVT_H

#include <map>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <sqlite3.h>

#include "../DataManagement/EVT.h"
#include "../DataManagement/RVS_TypeDefs.h"


namespace RVS
{
namespace Biomass
{
	/// BiomassEVT contains methods and parameters for accessing biomass information
	/// about a plot. The (EVT) object holds one evt record as found in the RVSDB
	class BiomassEVT :
		public RVS::DataManagement::EVT
	{
	public:
		// Constructors | Destructors //
		BiomassEVT(void);
		BiomassEVT(RVS::DataManagement::DataTable* dt);
		BiomassEVT(const BiomassEVT& old_biomassevt);
		virtual ~BiomassEVT(void);

		// EVT Overrides //
		inline float PERCENT_DOM() { return percentDom; };
		inline Biomass::BiomassVarUnits PA1_CODE() { return PA1_Code; };
		inline float PA1_VAL() { return PA1_Val; };
		inline Biomass::BiomassVarUnits PA2_CODE() { return PA2_Code; };
		inline float PA2_VAL() { return PA2_Val; };
		inline Biomass::BiomassVarUnits PA3_CODE() { return PA3_Code; };
		inline float PA3_VAL() { return PA3_Val; };
		inline Biomass::BiomassReturnType RETURN_TYPE() { return returnType; };
		inline std::map<Biomass::BiomassVarUnits, std::string> PARAMETERS() { return parms; }

		virtual std::string toString();

	protected:
        /// A better buildEVT function that doesn't depend on column positions
        /// <param name="dt"></param>
		virtual void buildEVT(RVS::DataManagement::DataTable* dt);
        /// Variable initializer. Called by all constructors everywhere.
		virtual void initialize_object();
		// parseItem override
		virtual void parseItem(sqlite3_stmt*, int);
		// Parses the return type to the enum
		virtual void parseReturnType(std::string rtype);
		// Parses a parameter code and value from the database
		virtual void parseParameter(sqlite3_stmt* stmt, char* columnName, int column);


		// BIOMASS EVT variables //
		float percentDom;
		Biomass::BiomassVarUnits PA1_Code;
		float PA1_Val;
		Biomass::BiomassVarUnits PA2_Code;
		float PA2_Val;
		Biomass::BiomassVarUnits PA3_Code;
		float PA3_Val;
		Biomass::BiomassReturnType returnType;
		std::map<RVS::Biomass::BiomassVarUnits, std::string> parms;

	private:
	    void copyData(const BiomassEVT& biomassevt_to_copy);

	};
}
}

#endif
