#include "SppRecord.h"

#include <utility>

using RVS::DataManagement::SppRecord;

//## Constructors/Destructors ##//
SppRecord::SppRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	initialize_object();
	buildRecord(dio, dt);
}

SppRecord::SppRecord(std::string spp_code, const double height, const double cover, std::string dom_spp)
{
	initialize_object();
	buildRecord(std::move(spp_code), height, cover, std::move(dom_spp));
}

void SppRecord::initialize_object()
{
	dom_spp = "";
	spp_code = "";
	height = 0.0;
	cover = 0.0;
	width = 0.0;
	stemsPerAcre = 0.0;
	shrubBiomass = 0.0;
	exShrubBiomass = 0.0;
	pchEqNum = 0;
	batEqNum = 0;
	fuel1hr = 0.0;
	fuel10hr = 0.0;
	fuel100hr = 0.0;
	fuel1000hr = 0.0;
}

void SppRecord::buildRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	if (!dio || !dt)
	{
		return;
	}

	sqlite3_stmt* stmt = dt->getStmt();
	if (!stmt)
	{
		return;
	}

	if (dt->Columns.count(DOM_SPP_FIELD))
	{
		dio->getVal(stmt, dt->Columns[DOM_SPP_FIELD], &dom_spp);
	}
	if (dt->Columns.count(SPP_CODE_FIELD))
	{
		dio->getVal(stmt, dt->Columns[SPP_CODE_FIELD], &spp_code);
	}
	if (dt->Columns.count(BIOMASS_HEIGHT_FIELD))
	{
		dio->getVal(stmt, dt->Columns[BIOMASS_HEIGHT_FIELD], &height);
	}
	if (dt->Columns.count(BIOMASS_COVER_FIELD))
	{
		dio->getVal(stmt, dt->Columns[BIOMASS_COVER_FIELD], &cover);
	}
}

void SppRecord::buildRecord(std::string spp_code, const double height, const double cover, std::string dom_spp)
{
	this->spp_code = std::move(spp_code);
	this->height = height;
	this->cover = cover;
	this->dom_spp = std::move(dom_spp);
}

double SppRecord::requestValue(std::string_view parameterName) const
{
	if (parameterName == "LEN" || parameterName == "LENGTH")
	{
		return width;
	}
	if (parameterName == "WID" || parameterName == "WIDTH")
	{
		return width;
	}
	if (parameterName == "HT" || parameterName == "HEIGHT")
	{
		return height;
	}
	if (parameterName == "VOL" || parameterName == "VOLUME")
	{
		return (width * width * height) / 1000.0; // dm^3
	}
	if (parameterName == "COV" || parameterName == "COVER")
	{
		return cover;
	}
	return 0.0;
}
