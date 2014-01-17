#include "EVT.h"

using RVS::DataManagement::EVT;

/// ******************************************* ///
/// EVT.cpp                                     ///
/// Master EVT class. All EVT objects inherit   ///
/// from this class. This class is abstract.    ///
/// ******************************************* ///

//## Constructors/Destructors ##//
EVT::EVT(void)
{
	this->initialize_object();
}

EVT::EVT(RVS::DataManagement::DataTable* dt)
{
	this->initialize_object();
	this->buildEVT(dt);
}

EVT::EVT(const EVT& old_evt)
{
    //copyData(old_evt);
}

EVT::~EVT(void)
{
}

//## Functions ##//
void EVT::buildEVT(RVS::DataManagement::DataTable* dt)
{
	sqlite3_stmt* stmt = dt->getStmt();
	int colCount = sqlite3_column_count(stmt);

	for (int i = 0; i < colCount; i++)
	{
		this->parseItem(stmt, i);
	}
	return;
}


void EVT::initialize_object()
{
	evt_num = 0;
	bps_num = "";
	lifeform = unk; 
	dom_spp = "";
	height = 0;
	cover = 0;
	samplePlots = 0;
	totalSamplePlots = 0;
}

void RVS::DataManagement::EVT::parseItem(sqlite3_stmt* stmt, int column)
{
	const char* colName = sqlite3_column_name(stmt, column);
	
	boost::any aval;
	this->getVar(stmt, column, &aval);
	
	// Special cases for all EVT types
	if (strcmp(colName, LIFEFORM_FIELD) == 0)
	{
		this->parseLifeform(boost::any_cast<std::string>(aval));
	}

	std::string name = std::string((char*)colName);
	vars_collection[name] = aval;
	putVar(colName, aval);
}

//$$ TODO finish lifeform parsing
void EVT::parseLifeform(std::string sval)
{
	const char* val = sval.c_str();
	if (strcmp(val, "shrub") == 0 || strcmp(val, "Shrub") == 0)
	{
		lifeform = shrub;
	}
	else if (strcmp(val, "herb") == 0)
	{
		lifeform = herb;
	}
	else
	{
		lifeform = unk;
	}
}

void EVT::getVar(sqlite3_stmt* stmt, int column, boost::any* retval)
{
	// Get the column data type from sqlite
	int colType = sqlite3_column_type(stmt, column);
	// Parse the data based on the appropriate data type
	switch (colType)
	{
	case SQLITE_INTEGER:
		*retval = sqlite3_column_int(stmt, column);
		break;
	case SQLITE_FLOAT:
		*retval = sqlite3_column_double(stmt, column);
		break;
	case SQLITE_BLOB:
		break;
	case SQLITE_NULL:
		break;
	case SQLITE3_TEXT:
		*retval = std::string((char*)sqlite3_column_text(stmt, column));
		break;
	}
}

void EVT::putVar(const char* name, boost::any var)
{
	// Checks if the passed variable is "important" and puts
	// it in a class variable for easier access.
	if (strcmp(name, EVT_NUM_FIELD) == 0)
	{
		evt_num = boost::any_cast<int>(var);
	}
	else if (strcmp(name, BPS_NUM_FIELD) == 0)
	{
		bps_num = boost::any_cast<std::string>(var);
	}
	else if (strcmp(name, DOM_SPP_FIELD) == 0)
	{
		dom_spp = boost::any_cast<std::string>(var);
	}
	else if (strcmp(name, SPP_CODE_FIELD) == 0)
	{
		spp_code = boost::any_cast<std::string>(var);
	}
}