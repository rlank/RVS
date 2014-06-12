#include "SppRecord.h"

using RVS::DataManagement::SppRecord;

/// ******************************************* ///
/// SppRecrod.cpp                               ///
/// ******************************************* ///

//## Constructors/Destructors ##//
SppRecord::SppRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	initialize_object();
	buildRecord(dio, dt);
	fuels = std::map<std::string, double>();
}

SppRecord::SppRecord(const SppRecord &old_evt)
{
    //copyData(old_evt);
}

SppRecord::~SppRecord(void)
{
}

//## Functions ##//
void SppRecord::initialize_object()
{
	dom_spp = "";
	spp_code = "";
	height = 0;
	cover = 0;
	shrubBiomass = 0;
	width = 0;
	stemsPerAcre = 0;
}

void SppRecord::buildRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	sqlite3_stmt* stmt = dt->getStmt();

	int column = 0;
	column = dt->Columns[DOM_SPP_FIELD];
	dio->getVal(stmt, dt->Columns[DOM_SPP_FIELD], &dom_spp);
	column = dt->Columns[SPP_CODE_FIELD];
	dio->getVal(stmt, dt->Columns[SPP_CODE_FIELD], &spp_code);
	column = dt->Columns[BIOMASS_HEIGHT_FIELD];
	dio->getVal(stmt, dt->Columns[BIOMASS_HEIGHT_FIELD], &height);
	column = dt->Columns[BIOMASS_COVER_FIELD];
	dio->getVal(stmt, dt->Columns[BIOMASS_COVER_FIELD], &cover);
}

double SppRecord::requestValue(std::string parameterName)
{
	double ret = 0.0;
	if (parameterName.compare("WID") == 0)
	{
		ret = width;
	}
	else if (parameterName.compare("HT") == 0)
	{
		ret = height;
	}
	else if (parameterName.compare("LEN") == 0)
	{
		ret = width; // we assume circles
	}
	else if (parameterName.compare("BIO") == 0)
	{
		ret = shrubBiomass;
	}

	return ret;
}


