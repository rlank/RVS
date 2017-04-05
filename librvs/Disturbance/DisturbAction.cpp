#include "DisturbAction.h"


RVS::Disturbance::DisturbAction::DisturbAction(int actionYear, string actionType, string actionSubType, map<string, double> params)
{
	this->actionYear = actionYear;
	this->actionType = actionType;
	this->actionSubType = actionSubType;
	this->params = params;
}


RVS::Disturbance::DisturbAction::~DisturbAction()
{
}
