#include "SuccessionDriver.h"

using RVS::Succession::SuccessionDriver;


SuccessionDriver::SuccessionDriver(RVS::Succession::SuccessionDIO* sdio, bool suppress_messages)
{
	this->sdio = sdio;
	this->suppress_messages = suppress_messages;
}


SuccessionDriver::~SuccessionDriver(void)
{
}

int* SuccessionDriver::SuccessionMain(int year, RVS::DataManagement::AnalysisPlot* ap)
{
	this->ap = ap;

	std::vector<RVS::DataManagement::SppRecord*>* shrubs = ap->SHRUB_RECORDS();

	if (ap->PLOT_NAME().compare("EOSG 05") == 0 && year == 19)
	{
		int asd = 0;
	}


	RVS::DataManagement::SppRecord* record = NULL;

	strVals_primary = std::map<string, string>();
	numVals_primary = std::map<string, double>();

	strVals_secondary = std::map<string, string>();;
	numVals_secondary = std::map<string, double>();

	strVals_tertiary = std::map<string, string>();;
	numVals_tertiary = std::map<string, double>();

	bool lastCohort = false;

	lastCohort = sdio->get_succession_data(ap->BPS_MODEL_NUM(), &strVals_primary, &numVals_primary);

	if (!lastCohort)
	{
		lastCohort = sdio->get_succession_data(ap->BPS_MODEL_NUM(), &strVals_secondary, &numVals_secondary);
	}

	if (!lastCohort)
	{
		lastCohort = sdio->get_succession_data(ap->BPS_MODEL_NUM(), &strVals_tertiary, &numVals_tertiary);
	}

	std::vector<double> endyears = std::vector<double>();
	endyears.push_back(numVals_primary["endAge"]);
	if (numVals_secondary.size() > 0) { endyears.push_back(numVals_secondary["endAge"]); }
	if (numVals_tertiary.size() > 0) { endyears.push_back(numVals_tertiary["endAge"]); }

	int sclass = ap->CURRENT_SUCCESSION_STAGE();
	int yearOffset = ap->startingYearsOffset;
	int yearsInClass = ap->TIME_IN_SUCCESSION_STAGE();
	isShrubDominated = ap->SHRUBCOVER() > ap->HERBCOVER();
	
	ap->cohortType = isShrubDominated ? "S" : "H";

	if (sclass == 0)
	{
		sclass = determineCurrentClass();
		yearOffset = relativeTimeInStage(sclass);
		ap->startingYearsOffset = yearOffset;
	}
	
	// If sclass == 0, no classification could be made and it's uncharacteristic
	if (sclass == 0)
	{
		ap->cohort = sclass;
		ap->timeInSuccessionStage = yearsInClass;
		ap->cohortType = "U";

		sdio->write_output_record(&year, ap);
		return RC;
	}

	//	get rounded midpoint year for stage
	//	add rounded midpoint to actual years(startage + rounded midpoint length) = adjusted midpoint
	int midpoint = 0;
	string stage;
	switch (sclass)
	{
	case 1:
		midpoint = (int)numVals_primary["midpoint"];
		stage = strVals_primary["cover_type"];
		break;
	case 2:
		midpoint = (int)numVals_secondary["midpoint"] + (int)numVals_secondary["startAge"];
		stage = strVals_secondary["cover_type"];
		break;
	case 3:
		midpoint = (int)numVals_tertiary["midpoint"] + (int)numVals_tertiary["startAge"];
		stage = strVals_tertiary["cover_type"];
		break;
	}
	
	std::map<string, string> strVals_current;
	std::map<string, double> numVals_current;
	std::map<string, string> strVals_next;
	std::map<string, double> numVals_next;

	bool isLate = stage.compare("Late") == 0 ? true : false;

	switch (sclass)
	{
	case 1:
		strVals_current = strVals_primary;
		numVals_current = numVals_primary;

		if (!isLate)
		{
			strVals_next = strVals_secondary;
			numVals_next = numVals_secondary;
		}

		break;
	case 2:
		strVals_current = strVals_secondary;
		numVals_current = numVals_secondary;

		if (!isLate)
		{
			strVals_next = strVals_tertiary;
			numVals_next = numVals_tertiary;
		}
		break;
	case 3:
		strVals_current = strVals_tertiary;
		numVals_current = numVals_tertiary;

		break;
	}

	int relativeYear = year + yearOffset;

	//(remember if type S split cover among all shrubs)
	shrubs = ap->SHRUB_RECORDS();

	if (relativeYear < midpoint)
	{
		//determine H or S growth stage(Cohort Type)
		string growthStage = strVals_current["cohort_type"];
		//height growth amount = GR_HT_m_yr
		double ht_growth = numVals_current["gr_ht"];
		//cover growth amount = GR_CC_yr
		double cov_growth = numVals_current["gr_cov"];

		double max_height = numVals_current["max_ht"];
		double max_cover = numVals_current["max_cov"];
		
		if (growthStage.compare("H") == 0)
		{	
			if (ap->herbHeight < max_height) { ap->herbHeight += cov_growth; }
			if (ap->herbCover < max_cover) { ap->herbCover += ht_growth; }
		}
		else
		{
			double divCover = 0;
			double divHeight = 0;
			if (ap->shrubCover < max_cover) { divCover = cov_growth / shrubs->size(); }
			if (ap->shrubHeight < max_height) { divHeight = ht_growth / shrubs->size(); }
			
			for (int r = 0; r < shrubs->size(); r++)
			{
				record = shrubs->at(r);
				record->height += divHeight;
				record->cover += divCover;
			}
		}
	}
	else
	{
		//determine H or S growth stage(Cohort Type)
		string growthStage = strVals_current["cohort_type"];
		//determine next stage H or S
		string nextGrowthStage = strVals_next["cohort_type"];
		//height growth amount = GR_HT_m_yr
		double ht_growth = numVals_next["gr_ht"];
		//cover growth amount = GR_CC_yr
		double cov_growth = numVals_next["gr_cov"];

		double max_height = numVals_next["max_ht"];
		double max_cover = numVals_next["max_cov"];

		if (nextGrowthStage.compare("H") == 0)
		{
			if (ap->herbHeight < max_height) { ap->herbHeight += cov_growth; }
			if (ap->herbCover < max_cover) { ap->herbCover += ht_growth; }
		}
		else
		{
			double divCover = 0;
			double divHeight = 0;
			if (ap->shrubCover < max_cover) { divCover = cov_growth / shrubs->size(); }
			if (ap->shrubHeight < max_height) { divHeight = ht_growth / shrubs->size(); }

			for (int r = 0; r < shrubs->size(); r++)
			{
				record = shrubs->at(r);
				record->height += divHeight;
				record->cover += divCover;
			}
		}
	}

	//attenuate shrubs and herbs(if sum(cover) > 100, find ratio and reduce)
	if (ap->herbCover + ap->shrubCover > 100)
	{
		double total = ap->herbCover + ap->shrubCover;
		ap->herbCover = (ap->herbCover / total) * 100;
		ap->shrubCover = (ap->shrubCover / total) * 100;
	}


	ap->cohort = sclass;
	ap->timeInSuccessionStage = yearsInClass;

	sdio->write_output_record(&year, ap);

	ap->timeInSuccessionStage += 1;

	if (relativeYear == numVals_current["endAge"])
	{
		ap->cohort += 1;
		ap->timeInSuccessionStage = 0;
	}

	return RC;
}


int RVS::Succession::SuccessionDriver::determineCurrentClass()
{
	int sclass = isShrubDominated ? determineVegClass("S", ap->SHRUBCOVER()) : determineVegClass("H", ap->HERBCOVER());
	return sclass;
}

int RVS::Succession::SuccessionDriver::determineVegClass(string cohortType, double cover)
{
	int sclass = 0;

	if (strVals_primary["cohort_type"].compare(cohortType) == 0)
	{
		if (cover < numVals_primary["max_cov"])
		{
			sclass = 1;
		}
	}
	else if (strVals_secondary["cohort_type"].compare(cohortType) == 0)
	{
		if (cover < numVals_secondary["max_cov"])
		{
			sclass = 2;
		}
	}
	else if (strVals_tertiary.size() > 0)
	{
		if (strVals_tertiary["cohort_type"].compare(cohortType) == 0)
		{
			if (cover < numVals_tertiary["max_cov"])
			sclass = 3;
		}
	}
	return sclass;
}

int RVS::Succession::SuccessionDriver::relativeTimeInStage(int sclass)
{
	int adjust = 0;

	double cover = isShrubDominated ? ap->SHRUBCOVER() : ap->HERBCOVER();

	switch (sclass)
	{
	case 1:
		adjust = relativeTimeInStageAdjuster(cover, strVals_primary, numVals_primary);
		break;
	case 2:
		adjust = relativeTimeInStageAdjuster(cover, strVals_secondary, numVals_secondary);
		break;
	case 3:
		adjust = relativeTimeInStageAdjuster(cover, strVals_tertiary, numVals_tertiary);
		break;
	}
	return adjust;
}

int RVS::Succession::SuccessionDriver::relativeTimeInStageAdjuster(double cover, std::map<string, string> strVals, std::map<string, double> numVals)
{
	int midpoint = numVals["midpoint"];
	int yearsPassed = (int)cover / (int)numVals["max_cov"];
	return yearsPassed;
}

std::list<string> RVS::Succession::SuccessionDriver::makeSpeciesList(std::map<string, string> strVals)
{
	std::list<string> species = std::list<string>();

	if (strVals["species1"].compare("") != 0) { species.push_back(strVals["species1"]); }
	if (strVals["species2"].compare("") != 0) { species.push_back(strVals["species2"]); }
	if (strVals["species3"].compare("") != 0) { species.push_back(strVals["species3"]); }
	if (strVals["species4"].compare("") != 0) { species.push_back(strVals["species4"]); }
	
	return species;
}