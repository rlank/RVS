#include "SuccessionDriver.h"

using RVS::Succession::SuccessionDriver;
double** RVS::Succession::SuccessionDriver::covariance_matrix = 0;

SuccessionDriver::SuccessionDriver(RVS::Succession::SuccessionDIO* sdio, bool suppress_messages)
{
	this->sdio = sdio;
	this->suppress_messages = suppress_messages;

	SuccessionDriver::covariance_matrix = sdio->query_covariance_matrix();
}


SuccessionDriver::~SuccessionDriver(void)
{
}

int* SuccessionDriver::SuccessionMain(int year, string* climate, RVS::DataManagement::AnalysisPlot* ap)
{
	this->ap = ap;
	this->shrubs = ap->SHRUB_RECORDS();
	this->climate = climate;

	//stringstream* s = new stringstream();
	//*s << "PLOT_ID: " << ap->PLOT_ID() << ". STARTING.";
	//const char* c = sdio->streamToCharPtr(s);
	//sdio->write_debug_msg(c);
	
	// TRUE: Do not model // FALSE: Model
	bool* doNotModel = new bool(false);

	// Load the (up to) 3 succession stages' values
	loadSuccessionVals(doNotModel);
	
	// Calculate herbaceous production
	double* yhat = new double;
	double* lower = new double;
	double* upper = new double;
	double production = 0;

	
	*yhat = calcProduction(year);
	calcConfidence(year, *yhat, lower, upper, 0);
	production = exp(*yhat) * SMEAR;
	
	ap->primaryProduction = production;
	ap->lower_confidence = *lower;
	ap->upper_confidence = *upper;
	
	// Get the current succession stage. Values are 0-3, with 0 indicating not yet classified
	// and -1 indicating uncharacteristic (unclassifiable) plot
	int sclass = ap->CURRENT_SUCCESSION_STAGE();
	// Landfire uses codes other than 1-3, so as long as it's not an error code (>100) just 
	// assume late stage (3)
	if (sclass > 3 && sclass < 100) { sclass = 3; }

	// Attempt to retreive plot age
	int ageOfPlot = ap->plotAge;
	
	// When sclass is 0, it needs to be detected and defined. 
	if (sclass == 0 || ap->burned)
	{
		sclass = determineCurrentClass();
		// If sclass == -1, no classification could be made and it's uncharacteristic
		// so don't attempt to calculate plot age
		if (sclass != -1)
		{
			ageOfPlot = plotAge(sclass);
			ap->plotAge = ageOfPlot;
		}
		ap->currentStage = sclass;
	}

	if (sclass == -1 || sclass == -99)
	{
		if (*doNotModel)
		{
			stringstream* s = new stringstream();
			*s << "PLOT_ID: " << ap->PLOT_ID() << ". RVS does not model this BPS.";
			const char* c = sdio->streamToCharPtr(s);
			sdio->write_debug_msg(c);
			ap->currentStage = -99;
		}

		ap->plotAge += 1;
		ap->currentStageType = "U";

		sdio->write_output_record(&year, ap);
		return RC;
	}

	// When using sclass to get parameters, remember stages are (1-3) but array is (0-2)
	ap->currentStageType = successionStrParameters[sclass - 1]["cohort_type"];

	// Check if this type of succession stage is even supported
	if (ap->currentStageType.compare("S") != 0 && ap->currentStageType.compare("H") != 0)
	{
		stringstream* s = new stringstream();
		*s << "PLOT_ID: " << ap->PLOT_ID() << ". Not a S or H plot. RVS does not model (grow shrubs).";
		const char* c = sdio->streamToCharPtr(s);
		sdio->write_debug_msg(c);
	}

	// get rounded midpoint year for stage
	// add rounded midpoint to actual years(startage + rounded midpoint length) = adjusted midpoint
	int midpoint = 0;
	int adjust = 0;
	string stage;

	midpoint = (int)successionNumParameters[sclass - 1]["midpoint"];
	adjust = (int)successionNumParameters[sclass - 1]["startAge"];
	midpoint += adjust;
	stage = successionStrParameters[sclass - 1]["cover_type"];

	bool isLate = stage.compare("L") == 0 ? true : false;

	shrubs = ap->SHRUB_RECORDS();
	// If the plot is not at the midpoint of the current stage, or if it's late stage,
	// grow the current succession stage. Growth for next stage begins halfway through
	// the current succession stage

	int useAge = stage.compare("H") == 0 ? ap->timeInHerbStage : ageOfPlot;
	if (stage.compare("H") == 0) { ap->timeInHerbStage += 1; }

	if (useAge < midpoint || isLate)
	{
		growStage(successionStrParameters[sclass - 1], successionNumParameters[sclass - 1]);
	}
	else
	{
		if (sclass >= successionNumParameters.size()) 
		{
			growStage(successionStrParameters[sclass - 1], successionNumParameters[sclass - 1]);
		}
		else
		{
			growStage(successionStrParameters[sclass], successionNumParameters[sclass]);
		}
		
	}
	
	// Grow herbs
	growHerbs(&(ap->herbCover), &(ap->herbHeight), &(ap->primaryProduction));
	
	// attenuate shrubs and herbs(if sum(cover) > 100, find ratio and reduce)
	if (ap->herbCover + ap->shrubCover > 100)
	{
		double total = ap->herbCover + ap->shrubCover;
		ap->herbCover = (ap->herbCover / total) * 100;
		ap->shrubCover = (ap->shrubCover / total) * 100;
	}

	
	ap->currentStage = sclass;

	sdio->write_output_record(&year, ap);

	if (successionStrParameters[sclass - 1]["cover_type"].compare("H")) { }
	ap->plotAge += 1;

	if (ageOfPlot == successionNumParameters[sclass - 1]["endAge"])
	{
		ap->currentStage += 1;
	}
	
	return RC;
}

void SuccessionDriver::loadSuccessionVals(bool* doNotModel)
{
	successionStrParameters = vector<map<string, string>>();
	successionNumParameters = vector<map<string, double>>();

	// Declare 3 cohort value maps. Not all may be used.
	map<string, string> strVals_primary = map<string, string>();
	map<string, double> numVals_primary = map<string, double>();

	map<string, string> strVals_secondary = map<string, string>();
	map<string, double> numVals_secondary = map<string, double>();

	map<string, string> strVals_tertiary = map<string, string>();
	map<string, double> numVals_tertiary = map<string, double>();

	bool lastCohort = false;

	// Populate the first succession stage
	lastCohort = sdio->get_succession_data(ap->BPS_MODEL_NUM(), &strVals_primary, &numVals_primary, doNotModel);
	// If there's another stage, populate secondary cohort
	if (!lastCohort)
	{
		lastCohort = sdio->get_succession_data(ap->BPS_MODEL_NUM(), &strVals_secondary, &numVals_secondary, doNotModel);
	}
	// If there's a final stage, populate the tertiary cohort
	if (!lastCohort)
	{
		lastCohort = sdio->get_succession_data(ap->BPS_MODEL_NUM(), &strVals_tertiary, &numVals_tertiary, doNotModel);
	}

	successionNumParameters.push_back(numVals_primary);
	successionNumParameters.push_back(numVals_secondary);
	successionNumParameters.push_back(numVals_tertiary);
	
	successionStrParameters.push_back(strVals_primary);
	successionStrParameters.push_back(strVals_secondary);
	successionStrParameters.push_back(strVals_tertiary);
}

int SuccessionDriver::determineCurrentClass()
{
	int sclass = 0;

	double cover = 0;
	double height = 0;

	// This loop attemps to diminish succession stages until the logic fails (cover and height do not
	// exceed maximum conditions). It also checks that the stage exists, as some classes only have
	// 1 or 2 succession stages
	for (int i = successionNumParameters.size() - 1; i >= 0; --i)
	{
		if (successionNumParameters[i].size() > 0)
		{
			
			if (successionStrParameters[i]["cohort_type"].compare("H") == 0)
			{
				cover = ap->HERBCOVER();
				height = ap->HERBHEIGHT();
			}
			else
			{
				if (ap->SHRUBCOVER() <= 1)
				{
					cover = 0;
					for (auto &s : *(ap->SHRUB_RECORDS()))
					{
						cover += s->COVER();
					}
					ap->shrubCover = cover;
				}
				

				height = ap->SHRUBHEIGHT(); 
			}
			

			// $REMINDER turned off height classification because too many uncharacteristic
			//if (cover < successionNumParameters[i]["max_cov"] && height < successionNumParameters[i]["max_ht"])
			//{
			//	if (cover > successionNumParameters[i]["min_cov"] && height > successionNumParameters[i]["min_ht"])
			//	{
			//		sclass = i + 1;
			//	}
			//	
			//}

			//cover = ap->SHRUBCOVER();
			//height = ap->SHRUBHEIGHT();

			if (successionStrParameters[i]["cover_type"].compare("Late") == 0 && cover > successionNumParameters[i]["max_cov"])
			{
				sclass = i + 1;
			}

			if (cover < successionNumParameters[i]["max_cov"])
			{
					sclass = i + 1;
			}
		}
	}

	if (sclass == 0)
	{
		stringstream* s = new stringstream();
		*s << "PLOT_ID: " << ap->PLOT_ID() << ". Plot uncharacteristic for succession.";
		const char* c = sdio->streamToCharPtr(s);
		sdio->write_debug_msg(c);

		sclass = -1;
	}

	return sclass;
}

int SuccessionDriver::plotAge(int sclass)
{
	double cover = 0;
	if (successionStrParameters[sclass - 1]["cohort_type"].compare("H") == 0)
	{
		cover = ap->HERBCOVER();
	}
	else
	{
		cover = ap->SHRUBCOVER();
	}
	int adjust = plotAgeCalculation(cover, successionStrParameters[sclass - 1], successionNumParameters[sclass - 1]);
	return adjust;
}

int SuccessionDriver::plotAgeCalculation(double cover, map<string, string> strVals, map<string, double> numVals)
{
	// At this point the stage has been classified, so just need to determine how many years it's been in this stage
	double stageStartingCover = numVals["min_cov"];
	double coverGrowthRate = numVals["gr_cov"];

	int ageOfPlot = int((cover - stageStartingCover) / coverGrowthRate) + numVals["startAge"];

	return ageOfPlot;
}

void SuccessionDriver::growStage(map<string, string> strVals, map<string, double> numVals)
{
	RVS::DataManagement::SppRecord* record = NULL;

	//determine H or S growth stage(Cohort Type)
	string growthStage = strVals["cohort_type"];
	//height growth amount = GR_HT_m_yr
	double ht_growth = numVals["gr_ht"];
	//cover growth amount = GR_CC_yr
	double cov_growth = numVals["gr_cov"];

	double max_height = numVals["max_ht"];
	double max_cover = numVals["max_cov"];

	if (growthStage.compare("H") != 0)
	{
		if (shrubs->empty())  
		{
			list<string> newspecies = makeSpeciesList(strVals);
			for (string s : newspecies)
			{
				bool modelSpp = sdio->check_shrub_data_exists(s);
				bool isShrub = sdio->check_code_is_shrub(s);
				if (modelSpp && isShrub)
				{ 
					string spp_name = sdio->get_scientific_name(s);
					SppRecord* shrub = new SppRecord(s, 0, 0, spp_name);
					shrubs->push_back(shrub);
				}
			}
		}

		double divCover = 0;
		double divHeight = 0;

		// We cannot allow cover to exceed 100, so check that growth won't go over max
		if (ap->shrubCover + cov_growth >= 100)
		{ 
			divCover = (99 - ap->SHRUBCOVER()) / shrubs->size();
		}
		else if (ap->shrubCover + cov_growth < max_cover)
		{
			divCover = cov_growth / shrubs->size(); 
		}
		else 
		{
			divCover = (max_cover - ap->SHRUBCOVER()) / shrubs->size();
		}

		// We allow height to grow beyond max height, as this is possible
		if (ap->shrubHeight < max_height)
		{
			divHeight = ht_growth / shrubs->size();
		}

		for (int r = 0; r < shrubs->size(); r++)
		{
			record = shrubs->at(r);
			record->height += divHeight;
			record->cover += divCover;
		}
	}
}

void SuccessionDriver::growHerbs(double* herbCover, double* herbHeight, double* production)
{
	double newProduction = *production;

	double* coverRate = new double;
	double* herbRate = new double;

	sdio->query_herb_growth_coefs(ap->BPS_MODEL_NUM(), coverRate, herbRate);

	double cover = *coverRate * newProduction;
	double height = *herbRate * newProduction * 100;  

	if (cover + ap->SHRUBCOVER() > 98)
	{
		*herbCover = 100 - ap->SHRUBCOVER() - 1;
	}
	else
	{
		*herbCover = cover;
	}


	*herbHeight = height;
}

list<string> SuccessionDriver::makeSpeciesList(map<string, string> strVals)
{
	list<string> species = list<string>();

	if (strVals["species1"].compare("") != 0) { species.push_back(strVals["species1"]); }
	if (strVals["species2"].compare("") != 0) { species.push_back(strVals["species2"]); }
	if (strVals["species3"].compare("") != 0) { species.push_back(strVals["species3"]); }
	if (strVals["species4"].compare("") != 0) { species.push_back(strVals["species4"]); }
	
	return species;
}

double SuccessionDriver::calcProduction(int year)
{
	double ndvi = ap->getNDVI(*climate, false);
	double ppt = ap->getPPT(*climate, false);

	double ln_ndvi = log(ndvi);
	double ln_ppt = log(ppt);

	double rawProduction = -5.2058235 + (ln_ppt * 0.1088213) + (ln_ndvi * 1.386304);
	ap->rawProduction = exp(rawProduction) * SMEAR;

	// Modify NDVI and PPT as a function of NOT SHRUB cover
	double adjust = 1 - (ap->SHRUBCOVER() / 100);

	ndvi = ndvi * adjust;
	ppt = ppt * adjust;

	ln_ndvi = log(ndvi);
	ln_ppt = log(ppt);

	double biomass = -5.2058235 + (ln_ppt * 0.1088213) + (ln_ndvi * 1.386304);
	return biomass;
}

void SuccessionDriver::calcConfidence(int year, double biomass, double* lower, double* upper, double* level)
{
	double ndvi = ap->getNDVI(*climate, false);
	double ppt = ap->getPPT(*climate, false);

	// Modify NDVI and PPT as a function of NOT SHRUB cover
	double adjust = 1 - (ap->SHRUBCOVER() / 100);

	ndvi = ndvi * adjust;
	ppt = ppt * adjust;

	double ln_ndvi = log(ndvi);
	double ln_ppt = log(ppt);

	// Begin confidence interval code

	double yhat = exp(biomass) * SMEAR;
	double ln_yhat = biomass;

	double tval = 1.961961;  // make this dynamic

	double s2y = calc_s2b(&ln_ndvi, &ln_ppt);
	double sy = sqrt(s2y);

	double ln_yhat_lower = ln_yhat - tval * sy;
	double yhat_lower = exp(ln_yhat_lower);
	yhat_lower = yhat_lower * SMEAR;

	double ln_yhat_upper = ln_yhat + tval * sy;
	double yhat_upper = exp(ln_yhat_upper);
	yhat_upper = yhat_upper * SMEAR;

	*lower = yhat_lower;
	*upper = yhat_upper;
}

double SuccessionDriver::calc_s2b(double* lnNDVI, double* lnPPT)
{
	double** dummy = new double*[1];
	dummy[0] = new double[3];

	dummy[0][0] = 1.0;
	dummy[0][1] = *lnPPT;
	dummy[0][2] = *lnNDVI;

	double** dummyTrans = matrix_trans(dummy, 1, 3);

	double** temp = matrix_mult(covariance_matrix, 3, 3, dummyTrans, 3, 1);

	double** s2b = matrix_mult(dummy, 1, 3, temp, 3, 1);

	ap->s2y = s2b[0][0];

	return s2b[0][0];
}

/*
double SuccessionDriver::calc_s2b(string* grp_id, double* lnNDVI, double* lnPPT)
{
	int index = sdio->find_group_index(grp_id);

	double** dummy = generate_dummy_variables(index, lnPPT, lnNDVI);
	double** dummyTrans = matrix_trans(dummy, 1, 102);

	double** temp = matrix_mult(covariance_matrix, 102, 102, dummyTrans, 102, 1);

	double** s2b = matrix_mult(dummy, 1, 102, temp, 102, 1);

	ap->s2y = s2b[0][0];

	return s2b[0][0];
}
*/

double** SuccessionDriver::matrix_mult(double** A, int aRow, int aCol, double** B, int bRow, int bCol)
{
	const int ROW = aRow;
	const int INNER = aCol;
	const int COL = bCol;

	double** C = new double*[ROW];

	for (int row = 0; row != ROW; ++row)
	{
		C[row] = new double[COL];
		for (int col = 0; col != COL; ++col)
		{
			double sum = 0;
			for (int inner = 0; inner != INNER; ++inner)
			{
				sum += A[row][inner] * B[inner][col];
			}
			C[row][col] = sum;
		}
	}

	return C;
}

double** SuccessionDriver::matrix_trans(double** A, int aRow, int aCol)
{
	const int ROW = aRow;
	const int COL = aCol;

	double** B = new double*[aCol];

	for (int col = 0; col != COL; ++col)
	{
		B[col] = new double[ROW];

		for (int row = 0; row != ROW; ++row)
		{
			B[col][row] = A[row][col];
		}
	}
	return B;
}

double** SuccessionDriver::generate_dummy_variables(int index, double* lnPPT, double* lnNDVI)
{
	double** dummy = new double*[1];
	dummy[0] = new double[102];

	dummy[0][0] = 1.0;
	dummy[0][1] = *lnPPT;
	dummy[0][2] = *lnNDVI;

	int counter = 1;
	for (int i = 3; i < 102; i++)
	{
		if (counter == index && i < 33)
		{
			dummy[0][i] = 1.0;
		}
		else if (counter == index && i < 66)
		{
			dummy[0][i] = *lnPPT;
		}
		else if (counter == index && i < 99)
		{
			dummy[0][i] = *lnNDVI;
		}
		else
		{
			dummy[0][i] = 0.0;
		}

		if (counter == 33)
		{
			counter = 0;
		}

		counter += 1;
	}
	return dummy;
}