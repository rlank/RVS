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

	RVS::DataManagement::SppRecord* record = NULL;

	std::map<string, string> strVals_primary = std::map<string, string>();
	std::map<string, double> numVals_primary = std::map<string, double>();

	std::map<string, string> strVals_secondary = std::map<string, string>();;
	std::map<string, double> numVals_secondary = std::map<string, double>();

	std::map<string, string> strVals_tertiary = std::map<string, string>();;
	std::map<string, double> numVals_tertiary = std::map<string, double>();

	bool lastCohort = false;

	lastCohort = sdio->get_succession_data(ap->BPS_NUM(), &strVals_primary, &numVals_primary);

	if (!lastCohort)
	{
		lastCohort = sdio->get_succession_data(ap->BPS_NUM(), &strVals_secondary, &numVals_secondary);
	}

	if (!lastCohort)
	{
		lastCohort = sdio->get_succession_data(ap->BPS_NUM(), &strVals_tertiary, &numVals_tertiary);
	}

	std::vector<double> endyears = std::vector<double>();
	endyears.push_back(numVals_primary["endAge"]);
	if (numVals_secondary.size() > 0) { endyears.push_back(numVals_secondary["endAge"]); }
	if (numVals_tertiary.size() > 0) { endyears.push_back(numVals_tertiary["endAge"]); }

	int sclass = determineCurrentClass(year, endyears);

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
		midpoint = (int)numVals_secondary["midpoint"] + numVals_primary["midpoint"];
		stage = strVals_secondary["cover_type"];
		break;
	case 3:
		midpoint = (int)numVals_tertiary["midpoint"] + numVals_secondary["midpoint"];
		stage = strVals_tertiary["cover_type"];
		break;
	}

	//	if LATE and adjusted midpoint > current year : DO NOTHING, BREAK METHOD NOW
	//	else, do below
	if (sclass == 3 && midpoint > year)
	{
		return RC;
	}
	if (sclass == 2 && midpoint > year && strVals_tertiary.size() == 0)
	{
		return RC;
	}
	
	std::map<string, string> strVals_current;
	std::map<string, double> numVals_current;
	std::map<string, string> strVals_next;
	std::map<string, double> numVals_next;

	bool isLate = strcmp(stage.c_str(), "Late") == 0 ? true : false;

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

	
	if (year <= midpoint)
	{
		//determine H or S growth stage(Cohort Type)
		string growthStage = strVals_current["cohort_type"];
		//height growth amount = GR_HT_m_yr
		double ht_growth = numVals_current["gr_ht"];
		//cover growth amount = GR_CC_yr
		double cov_growth = numVals_current["gr_cov"];

		double max_height = numVals_current["max_ht"];
		double max_cover = numVals_current["max_cov"];
		
		/*
		//(remember if type S split cover among all shrubs)
		//check species list
		shrubs = ap->SHRUB_RECORDS();
		
		std::list<string> classSpecies = makeSpeciesList(strVals_current);
		std::list<string> speciesToAdd = std::list<string>(classSpecies);
		for (int r = 0; r < shrubs->size(); r++)
		{
			record = shrubs->at(r);
			for (string s : classSpecies)
			{
				if (record->spp_code.compare(s) == 0)
				{
					speciesToAdd.remove(s);
				}
			}
		}

		for (string s : speciesToAdd)
		{
			//if species doesn't exist:
			//if spp_code NOT IN Bio_Crosswalk : PASS
			bool useSpecies = sdio->check_shrub_data_exists(s);
			if (useSpecies)
			{
				string spp_name = sdio->get_scientific_name(s);
				RVS::DataManagement::SppRecord* record = new RVS::DataManagement::SppRecord(s, 0.0, 0.0, spp_name);
				ap->push_shrub(record);
			}
		}

		shrubs = ap->SHRUB_RECORDS();
		*/

		if (growthStage.compare("H") == 0)
		{	
			if (ap->herbHeight < max_height)
			{
				ap->herbHeight += cov_growth;
			}
			if (ap->herbCover < max_cover)
			{
				ap->herbCover += ht_growth;
			}
		}
		else
		{
			double divHeight = ht_growth / shrubs->size();
			double divCover = cov_growth / shrubs->size();
			for (int r = 0; r < shrubs->size(); r++)
			{
				record = shrubs->at(r);

				//if amounts > current(cover or height) : do nothing
				if (record->height < max_height)
				{
					record->height += divHeight;
				}
				if (record->cover < max_cover)
				{
					record->cover += divCover;
				}
			}
		}
	}
	else
	{
		//determine H or S growth stage(Cohort Type)
		string growthStage = strVals_current["cohort_type"];
		//determine next stage H or S
		string nextGrowthStage = strVals_current["cover_type"];
		//height growth amount = GR_HT_m_yr
		double ht_growth = numVals_next["gr_ht"];
		//cover growth amount = GR_CC_yr
		double cov_growth = numVals_next["gr_cov"];

		double max_height = numVals_next["max_ht"];
		double max_cover = numVals_next["max_cov"];
		//check species list of 'next' class
		/*
		std::list<string> classSpecies = makeSpeciesList(strVals_next);
		std::list<string> speciesToAdd = std::list<string>(classSpecies);
		for (int r = 0; r < shrubs->size(); r++)
		{
			record = shrubs->at(r);
			for (string s : classSpecies)
			{
				if (record->spp_code.compare(s) == 0)
				{
					speciesToAdd.remove(s);
				}
			}
		}

		for (string s : speciesToAdd)
		{
			//if species doesn't exist:
			//if spp_code NOT IN Bio_Crosswalk : PASS
			bool useSpecies = sdio->check_shrub_data_exists(s);
			if (useSpecies)
			{
				string spp_name = sdio->get_scientific_name(s);
				RVS::DataManagement::SppRecord* record = new RVS::DataManagement::SppRecord(s, 0.0, 0.0, spp_name);
				ap->push_shrub(record);
			}
		}

		shrubs = ap->SHRUB_RECORDS();
		*/
		std::list<string> oldClassSpecies = makeSpeciesList(strVals_current);
		

		if (growthStage.compare("H") == 0)
		{
			ap->herbHeight -= ht_growth;
			ap->herbCover -= cov_growth;
			if (ap->herbHeight < 0) { ap->herbHeight = 0; }
			if (ap->herbCover < 0) { ap->herbCover = 0; }
		}
		else
		{
			for (int r = 0; r < shrubs->size(); r++)
			{
				record = shrubs->at(r);
				for (string os : oldClassSpecies)
				{
					if (strcmp(os.c_str(), record->spp_code.c_str()) == 0)
					{
						record->height -= ht_growth;
						record->cover -= cov_growth;
						if (record->height < 0) { record->height = 0; }
						if (record->cover < 0) { record->cover = 0; }
					}
				}
			}
		}

		if (nextGrowthStage.compare("H") == 0)
		{
			if (record->height < max_height)
			{
				ap->herbCover += ht_growth;
			}
			if (record->cover < max_cover)
			{
				ap->herbHeight += cov_growth;
			}
		}
		else
		{
			double divHeight = ht_growth / shrubs->size();
			double divCover = cov_growth / shrubs->size();
			for (int r = 0; r < shrubs->size(); r++)
			{
				record = shrubs->at(r);

				//if amounts > current(cover or height) : do nothing
				if (record->height < max_height)
				{
					record->height += divHeight;
				}
				if (record->cover < max_cover)
				{
					record->cover += divCover;
				}
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
	return RC;
}

int RVS::Succession::SuccessionDriver::determineCurrentClass(int year, std::vector<double> endpoints)
{
	int sclass = 0;
	if (year <= endpoints[0])
	{
		sclass = 1;
	}
	else if (year <= endpoints[1])
	{
		sclass = 2;
	}
	else
	{
		sclass = 3;
	}

	return sclass;
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