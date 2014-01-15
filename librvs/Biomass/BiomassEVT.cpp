#include "BiomassEVT.h"


//## Constructors/Destructors ##//
RVS::Biomass::BiomassEVT::BiomassEVT(void) : RVS::DataManagement::EVT()
{
	// The constructor defintion calls the base class' initialize_object first
	this->initialize_object();
}

RVS::Biomass::BiomassEVT::BiomassEVT(RVS::DataManagement::DataTable* dt) : RVS::DataManagement::EVT(dt)
{
	this->initialize_object();
	this->buildEVT(dt);
}

RVS::Biomass::BiomassEVT::~BiomassEVT(void)
{

}


void RVS::Biomass::BiomassEVT::buildEVT(RVS::DataManagement::DataTable* dt)
{
	sqlite3_stmt* stmt = dt->getStmt();
	int colCount = sqlite3_column_count(stmt);
	
	for (int i = 0; i < colCount; i++)
	{
		this->parseItem(stmt, i);
	}
}

void RVS::Biomass::BiomassEVT::initialize_object()
{
	percentDom = 0;
	PA1_Code = RVS::Biomass::VNUL;
	PA1_Val = 0;
	PA2_Code = RVS::Biomass::VNUL;
	PA2_Val = 0;
	PA3_Code = RVS::Biomass::VNUL;
	PA3_Val = 0;
	returnType = RVS::Biomass::RNUL;
}

void RVS::Biomass::BiomassEVT::parseItem(sqlite3_stmt* stmt, int column)
{
	const char* colName = sqlite3_column_name(stmt, column);

	boost::any aval;
	this->getVar(stmt, column, &aval);

	// Special cases that need populated
	if (strcmp(colName, RET_CODE_FIELD) == 0)
	{
		this->parseReturnType(boost::any_cast<std::string>(aval));
	}
	else if (strcmp(colName, BIOMASS_PARAM_1_CODE_FIELD) == 0 || strcmp(colName, BIOMASS_PARAM_2_CODE_FIELD) == 0)
	{
		this->parseParameter(stmt, (char*)colName, column);
	}

	std::string name = std::string((char*)colName);
	vars_collection[name] = aval;
	this->putVar(colName, aval);
}

 //$$ TODO add other return types
void RVS::Biomass::BiomassEVT::parseReturnType(std::string rtype)
{
	const char* ctype = rtype.c_str();
	if (strcmp(ctype, "PCH") == 0)
	{
		returnType = PCH;
	}
}

void RVS::Biomass::BiomassEVT::parseParameter(sqlite3_stmt* stmt, char* columnName, int column)
{
	boost::any val;
	if (strcmp(columnName, BIOMASS_PARAM_1_CODE_FIELD) == 0)
	{
		this->getVar(stmt, column, &val);
		PA1_Code = RVS::Biomass::BiomassVarUnitsMap[boost::any_cast<std::string>(val)];
		this->getVar(stmt, column + 1, &val);
		PA1_Val = boost::any_cast<int>(val);

		if (PA1_Code == HT)
		{
			height = PA1_Val;
		}
		else
		{
			cover = PA1_Val;
		}
	}
	else if (strcmp(columnName, BIOMASS_PARAM_2_CODE_FIELD) == 0)
	{
		this->getVar(stmt, column, &val);
		PA2_Code = RVS::Biomass::BiomassVarUnitsMap[boost::any_cast<std::string>(val)];
		this->getVar(stmt, column + 1, &val);
		PA2_Val = boost::any_cast<int>(val);

		if (PA2_Code == HT)
		{
			height = PA2_Val;
		}
		else
		{
			cover = PA2_Val;
		}
	}
}

std::string RVS::Biomass::BiomassEVT::toString()
{
	std::stringstream str;
	int i = this->b_EVT_NUM();
	std::string b = this->b_BPS_NUM();

	str << "EVT: " << i << ", BPS: " << b;
	return str.str();
}

