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
	// Get the sqlite3 statement out of the datatable
	sqlite3_stmt* stmt = dt->getStmt();
	// Iterate through the input data, parsing each item
	int colCount = sqlite3_column_count(stmt);
	for (int i = 0; i < colCount; i++)
	{
		this->parseItem(stmt, i);
	}
}

void RVS::Biomass::BiomassEVT::initialize_object()
{
	// Initialize all class variables
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
	// Get the column name
	const char* colName = sqlite3_column_name(stmt, column);

	// Get the data out of the column using boost::any for type safety
	boost::any aval;
	this->getVar(stmt, column, &aval);

	// Special cases that need populated into class variables
	if (strcmp(colName, RET_CODE_FIELD) == 0)
	{
		this->parseReturnType(boost::any_cast<std::string>(aval));
	}
	else if (strcmp(colName, BIOMASS_PARAM_1_CODE_FIELD) == 0 || strcmp(colName, BIOMASS_PARAM_2_CODE_FIELD) == 0)
	{
		this->parseParameter(stmt, (char*)colName, column);
	}

	// Add the new data to the collection
	std::string name = std::string((char*)colName);
	vars_collection[name] = aval;
	// Try putting it into a class variable if there is a rule for it
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
	//$$ TODO parseParameter really only works for PCH (expecting 1 of 2 input parameters, 
	// height or cover. Will expand later.
	boost::any val;
	if (strcmp(columnName, BIOMASS_PARAM_1_CODE_FIELD) == 0)
	{
		// Get the parameter type (height or cover)
		this->getVar(stmt, column, &val);
		PA1_Code = RVS::Biomass::BiomassVarUnitsMap[boost::any_cast<std::string>(val)];
		// Get the parameter value (next column)
		this->getVar(stmt, column + 1, &val);
		PA1_Val = boost::any_cast<int>(val);

		// If it was height, populate height variable and vis verca
		if (PA1_Code == HT)
		{

			height = PA1_Val + (PA1_Val * *GROWTH_MULT);  //$$ HACK linear growth increase
		}
		else
		{
			cover = PA1_Val + (PA1_Val * *GROWTH_MULT);  //$$ HACK linear growth increase
		}
	}
	else if (strcmp(columnName, BIOMASS_PARAM_2_CODE_FIELD) == 0)
	{
		// Get the parameter type (height or cover)
		this->getVar(stmt, column, &val);
		PA2_Code = RVS::Biomass::BiomassVarUnitsMap[boost::any_cast<std::string>(val)];
		// Get the parameter value (next column)
		this->getVar(stmt, column + 1, &val);
		PA2_Val = boost::any_cast<int>(val);

		// If it was height, populate height variable and vis verca
		if (PA2_Code == HT)
		{
			height = PA2_Val + (PA2_Val * *GROWTH_MULT);  //$$ HACK linear growth increase
		}
		else
		{
			cover = PA2_Val + (PA2_Val * *GROWTH_MULT);  //$$ HACK linear growth increase
		}
	}
}

std::string RVS::Biomass::BiomassEVT::toString()
{
	std::stringstream str;
	return str.str();
}

