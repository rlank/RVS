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

EVT::EVT(const EVT& old_evt)
{
    //copyData(old_evt);
}

EVT::~EVT(void)
{
}

//## Functions ##//
void EVT::base_buildEVT(RVS::DataManagement::DataTable &dt, int &row_num)
{
	//this.parseItem(dt.Rows[row_num]["evt_num"], ref evt_num);
	//this.parseItem(dt.Rows[row_num]["bps"], ref bps_num);
	//this.parseItem(dt.Rows[row_num]["lifeform"], ref lifeform);
	//this.parseItem(dt.Rows[row_num]["spp_code"], ref spp_code);
	//this.parseItem(dt.Rows[row_num]["dom_spp"], ref dom_spp);
	//this.parseItem(dt.Rows[row_num]["plots"], ref samplePlots);
	//this.parseItem(dt.Rows[row_num]["tplots"], ref totalSamplePlots);
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