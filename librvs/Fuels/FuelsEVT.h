/// ********************************************************** ///
/// Name: FuelsEVT.h                                         ///
/// Desc: 
/// ********************************************************** ///

#pragma once

#ifndef FUELS_EVT_H
#define FUELS_EVT_H

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
namespace Fuels
{
	/// BiomassEVT contains methods and parameters for accessing biomass information
	/// about a plot. The (EVT) object holds one evt record as found in the RVSDB
	class FuelsEVT :
		public RVS::DataManagement::EVT
	{
	public:
		// Constructors | Destructors //
		FuelsEVT(void);
		FuelsEVT(RVS::DataManagement::DataTable* dt);
		FuelsEVT(const FuelsEVT& old_biomassevt);
		virtual ~FuelsEVT(void);

	protected:
        

	private:

	};
}
}

#endif
