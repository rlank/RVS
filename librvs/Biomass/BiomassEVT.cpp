#include "BiomassEVT.h"


//## Constructors/Destructors ##//
RVS::Biomass::BiomassEVT::BiomassEVT(void) : RVS::DataManagement::EVT()
{
	// The constructor defintion calls the base class' initialize_object first
	this->initialize_object();
}

RVS::Biomass::BiomassEVT::BiomassEVT(RVS::DataManagement::DataTable* dt, int* row_num) : RVS::DataManagement::EVT()
{
	this->initialize_object();
	this->buildEVT(dt, row_num);
}

RVS::Biomass::BiomassEVT::~BiomassEVT(void)
{
}


//$$ TODO: Code this.
void RVS::Biomass::BiomassEVT::buildEVT(RVS::DataManagement::DataTable* dt, int* row_num)
{
//   this.parseItem(dt.Rows[row_num]["pa1_val"], ref pa1_val);
//   this.parseItem(dt.Rows[row_num]["pa2_val"], ref pa2_val);
//   this.parseItem(dt.Rows[row_num]["pa3_val"], ref pa3_val);
//   this.parseItem(dt.Rows[row_num]["pa1_code"], ref pa1_code);
//   this.parseItem(dt.Rows[row_num]["pa2_code"], ref pa2_code);
//   this.parseItem(dt.Rows[row_num]["pa3_code"], ref pa3_code);

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

