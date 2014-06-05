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
	int colCount = sqlite3_column_count(stmt);

	for (int i = 0; i < colCount; i++)
	{
		// Get the column name
		const char* colName = sqlite3_column_name(stmt, i);
		// Get the data out of the column using boost::any for type safety
		boost::any aval;
		dio->getVal(stmt, i, &aval);

		if (strcmp(colName, DOM_SPP_FIELD) == 0)
		{
			dom_spp = boost::any_cast<std::string>(aval);
		}
		else if (strcmp(colName, SPP_CODE_FIELD) == 0)
		{
			spp_code = boost::any_cast<std::string>(aval);
		}
		else if (strcmp(colName, BIOMASS_HEIGHT_FIELD) == 0)
		{
			this->height = boost::any_cast<double>(aval);
		}
		else if (strcmp(colName, BIOMASS_COVER_FIELD) == 0)
		{
			this->cover = boost::any_cast<double>(aval);
		}
	}
	return;
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
		return width; // we assume circles
	}
	else if (parameterName.compare("BIO") == 0)
	{
		ret = shrubBiomass;
	}

	return ret;
}


