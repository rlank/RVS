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
}

SppRecord::SppRecord(string spp_code, double height, double cover, string dom_spp)
{
	initialize_object();
	buildRecord(spp_code, height, cover, dom_spp);
}

SppRecord::~SppRecord(void)
{
}

void SppRecord::initialize_object()
{
	dom_spp = "";
	spp_code = "";
	height = 0;
	cover = 0;
	width = 0;
	shrubBiomass = 0;
	stemsPerAcre = 0;
	shrubBiomass = 0;
	exShrubBiomass = 0;
	pchEqNum = 0;
	batEqNum = 0;
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

void SppRecord::buildRecord(string spp_code, double height, double cover, string dom_spp)
{
	this->spp_code = spp_code;
	this->height = height;
	this->cover = cover;
	this->dom_spp = dom_spp;
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
	else if (parameterName.compare("COV") == 0)
	{
		ret = cover;
	}
	else if (parameterName.compare("BIO") == 0)
	{
		ret = shrubBiomass;
	}
	else if (parameterName.compare("VOL") == 0)
	{
		ret = width * width * height;
	}

	return ret;
}


