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

//   this.parseItem(dt.Rows[row_num]["ret_code"], ref returnType);

//   if (pa1_code != BiomassVarUnits.NUL)
//       parms.Add(pa1_code, pa1_val);
//   if (pa2_code != BiomassVarUnits.NUL)
//       parms.Add(pa2_code, pa2_val);
//   if (pa3_code != BiomassVarUnits.NUL)
//       parms.Add(pa3_code, pa3_val);
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
	coef1 = 0;
	coef2 = 0;
	coef3 = 0;
	coef4 = 0;
	returnType = RVS::Biomass::RNUL;
	//parms = new std::map<RVS::Biomass::BiomassVarUnits, std::string);
}

std::string RVS::Biomass::BiomassEVT::toString()
{
	std::stringstream str; 
	
	str << EVT_NUM2() << " " << vars_collection["dom_spp"];
	return str.str();
}

