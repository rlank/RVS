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

#include "../DataManagement/AnalysisPlot.h"
#include "../DataManagement/RVS_TypeDefs.h"
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
		FuelsDriver(RVS::Fuels::FuelsDIO* fdio, bool suppress_messages = false);
		virtual ~FuelsDriver(void);

		bool suppress_messages;
		
		int* FuelsMain(int year, RVS::DataManagement::AnalysisPlot* ap);

	private:
		RVS::Fuels::FuelsDIO* fdio;
	};
}
}

#endif