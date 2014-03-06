/// ********************************************************** ///
/// Name: FuelsDriver.h                                      ///
/// Desc: 
/// ********************************************************** ///

#pragma once

#ifndef FUELSDRIVER_H
#define FUELSDRIVER_H

#include <exception>
#include <iostream>
#include <stdio.h>

#include "../DataManagement/RVS_TypeDefs.h"
#include "../Biomass/BiomassEVT.h"
#include "FuelsDIO.h"
#include "FuelsEquations.h"

namespace RVS
{
namespace Fuels
{
    /// 
	class FuelsDriver
	{
	public:
		FuelsDriver(bool suppress_messages = false, bool write_intermediate = false);
		virtual ~FuelsDriver(void);

		bool suppress_messages;
		bool write_intermediate;
		
		int* FuelsMain(RVS::Biomass::BiomassEVT* bioEVT, int* plot_num, double* shrubBiomass, double* herbBiomass);

	private:
		
	};
}
}

#endif