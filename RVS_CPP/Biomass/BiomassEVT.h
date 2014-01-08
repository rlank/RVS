/// ********************************************************** ///
/// Name: BiomassEVT.h                                         ///
/// Desc: Biomass EVT interface class.                         ///
/// Base Class(es): RVS::DataManagement::EVT                   ///
/// Requires: map, string, EVT.h, RVS_TypeDefs.h               ///
/// ********************************************************** ///

#pragma once

#ifndef BIOMASS_EVT_H
#define BIOMASS_EVT_H

#include <map>
#include <string>

#include <DataManagement/EVT.h>
#include <DataManagement/RVS_TypeDefs.h>


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
		BiomassEVT(RVS::DataManagement::DataTable* dt, int* row_num);
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
		inline float COEF1() { return coef1; };
		inline float COEF2() { return coef2; };
		inline float COEF3() { return coef3; };
		inline float COEF4() { return coef4; };
		inline Biomass::BiomassReturnType RETURN_TYPE() { return returnType; };
		inline std::map<Biomass::BiomassVarUnits, std::string> PARAMETERS() { return parms; }

	protected:
		/// <summary>
        /// A better buildEVT function that doesn't depend on column positions
        /// </summary>
        /// <param name="dt"></param>
        /// <param name="row_num"></param>
		void buildEVT(RVS::DataManagement::DataTable* dt, int* row_num);
		/// <summary>
        /// Variable initializer. Called by all Constructors.
        /// </summary>
		void initialize_object();

	private:
	    void copyData(const BiomassEVT& biomassevt_to_copy);

	};
}
}

#endif
