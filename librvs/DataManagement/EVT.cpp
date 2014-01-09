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

void EVT::parseItem(sqlite3_stmt* stmt, int column)
{
	const char* colName = sqlite3_column_name(stmt, column);
	int colType = sqlite3_column_type(stmt, column);

	std::string name = std::string((char*)colName);
	boost::any aval;
	switch (colType)
	{
		case SQLITE_INTEGER:
			aval = sqlite3_column_int(stmt, column);
			break;
		case SQLITE_FLOAT:
			aval = sqlite3_column_double(stmt, column);
			break;
		case SQLITE_BLOB:
			break;
		case SQLITE_NULL:
			break;
		case SQLITE3_TEXT:
			aval = std::string((char*)sqlite3_column_text16(stmt, column));
			break;
	}
	vars_collection[name] = &aval;
}