#include "BiomassDriver.h"

using RVS::Biomass::BiomassDriver;

double** RVS::Biomass::BiomassDriver::covariance_matrix = 0; 

BiomassDriver::BiomassDriver(RVS::Biomass::BiomassDIO* bdio, bool suppress_messages)
{
	this->bdio = bdio;
	this->suppress_messages = suppress_messages;

	RVS::Biomass::BiomassDriver::covariance_matrix = bdio->query_covariance_matrix();
}


BiomassDriver::~BiomassDriver(void)
{
}

int* BiomassDriver::BioMain(int year, RVS::DataManagement::AnalysisPlot* ap, double* retShrubBiomass, double* retHerbBiomass)
{
	this->ap = ap;
	int plot_num = ap->PLOT_ID();

	if (plot_num == 1)
	{
		int asdasds = 0;
	}

	std::vector<RVS::DataManagement::SppRecord*>* shrubs = ap->SHRUB_RECORDS();
	double totalShrubCover = 0;
	double runShrubHeight = 0;
	RVS::DataManagement::SppRecord* record = NULL;

	for (int r = 0; r < shrubs->size(); r++)
	{
		record = shrubs->at(r);

		double stemsPerAcre = calcStemsPerAcre(record);
		record->stemsPerAcre = stemsPerAcre;
		double singleBiomass = calcShrubBiomass(record);

		record->shrubBiomass = singleBiomass;
		record->exShrubBiomass = singleBiomass * stemsPerAcre;
		*retShrubBiomass += record->exShrubBiomass;

		totalShrubCover += record->cover;
		runShrubHeight += record->height * record->cover;

		// Write this result to the intermediate table
		RC = bdio->write_intermediate_record(&year, ap, record);
	}

	double oldBiomass = ap->herbBiomass;

	// Apply holdover biomass if applicable
	if (ap->herbBiomass != 0)
	{
		double holdover = calcHerbReduction(totalShrubCover);
		ap->herbHoldoverBiomass = holdover * oldBiomass;
	}

	double* yhat = new double;
	double* lower = new double;
	double* upper = new double;

	if (ap->HERBCOVER() > 0)
	{
		*yhat = calcHerbBiomass(year);
		calcConfidence(year, *yhat, lower, upper, 0);
		*retHerbBiomass = exp(*yhat) * SMEAR;
	}
	else
	{
		*yhat = 0;
		*lower = 0;
		*upper = 0;
		*retHerbBiomass = 0;
	}
	
	ap->lower_confidence = *lower;
	ap->upper_confidence = *upper;

	double averageHeight = runShrubHeight / totalShrubCover;

	if (shrubs->empty())
	{
		totalShrubCover = 0;
		runShrubHeight = 0;
		averageHeight = 0; 
	}

	growHerbs(&ap->herbCover, &ap->herbHeight, oldBiomass, *retHerbBiomass);

	ap->primaryProduction = *retHerbBiomass;
	
	ap->shrubCover = totalShrubCover;
	ap->shrubHeight = averageHeight;

	ap->herbBiomass = *retHerbBiomass + ap->herbHoldoverBiomass;
	ap->shrubBiomass = *retShrubBiomass;
	ap->totalBiomass = ap->SHRUBBIOMASS() + ap->HERBBIOMASS();

	// Write output biomass record
	RC = bdio->write_output_record(&year, ap);

	return RC;
}

double BiomassDriver::calcShrubBiomass(RVS::DataManagement::SppRecord* record)
{
	// Get the equation number for BAT (total aboveground biomass)
	int equationNumber = 0;
	
	try
	{
		equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), "BAT2");
	}
	catch (RVS::DataManagement::DataNotFoundException dex)
	{
		equationNumber = bdio->query_crosswalk_table("ARTR2", "BAT2");
	}

	if (equationNumber == 0)
	{
		string s = "Something is wrong.";
	}

	record->batEqNum = equationNumber;

	// Populate the coefficients with values from the biomass equation table
	double* coefs = new double[4];
	string* paramNames = new string[3];
	bdio->query_equation_parameters(equationNumber, paramNames, coefs);

	std::map<string, double>* params = new std::map<string, double>();

	double val = 0.0;
	for (int i = 0; i < paramNames->size(); i++)
	{
		val = record->requestValue(paramNames[i]);
		params->insert(pair<string, double>(paramNames[i], val));
	}

	double biomass = BiomassEquations::eq_BAT(equationNumber, coefs, params);
	return biomass;
}

double BiomassDriver::calcStemsPerAcre(RVS::DataManagement::SppRecord* record)
{
	// Lookup the equation number from the crosswalk table
	int equationNumber = 0;
	try
	{
		equationNumber = bdio->query_crosswalk_table(record->SPP_CODE(), "PCH");
	}
	catch (RVS::DataManagement::DataNotFoundException dex)
	{
		equationNumber = bdio->query_crosswalk_table("ARTR2", "PCH");
	}
	
	record->pchEqNum = equationNumber;
	
	double* coefs = new double[4];

	// Populate the coefficients with values from the biomass equation table
	bdio->query_equation_coefficients(equationNumber, coefs);

	double singleStem = BiomassEquations::eq_PCH(coefs[0], coefs[1], record->HEIGHT());

	// While we're here, calculate width (singleStem is area)
	double radius = std::sqrt(singleStem / 3.1415); // Use number for PI rather than constant cause it's the only thing needed out of CMATH
	record->width = radius * 2;

	// Expand the single stem result
	// First convert the cm^2 to m^2, then to ACRES
	double expanded = EXPANSION_FACTOR / (singleStem * .0001);
	// Return the expanded amount as a function of percent cover
	double stemsPerAcre = expanded * (record->COVER() / 100);
	return stemsPerAcre;
}

double BiomassDriver::calcHerbBiomass(int year)
{
	// BASIC FORM:
	// Intercept + ln_ndvi + ln_precip_mm + GRP_ID_CONST + (ln_ndvi*grp_id) + (ln_prcp_mm*grp_id)

	double* baseline_const = new double;
	double* baseline_ndvi = new double;
	double* baseline_ppt = new double;

	string* grp_id = new string;

	try
	{
		bdio->query_biogroup_coefs(0, baseline_const, baseline_ndvi, baseline_ppt, grp_id, false);
	}
	catch (RVS::DataManagement::DataNotFoundException &ex)
	{
		
	}

	// Populate interact fields from database
	double* grp_id_const = new double;
	double* ndvi_grp_interact = new double;
	double* ppt_grp_interact = new double;
	
	try
	{
		bdio->query_biogroup_coefs(ap->BPS_NUM(), grp_id_const, ndvi_grp_interact, ppt_grp_interact, grp_id, false);
	}
	catch (RVS::DataManagement::DataNotFoundException &ex)
	{
		bdio->query_biogroup_coefs(ap->BPS_NUM(true), grp_id_const, ndvi_grp_interact, ppt_grp_interact, grp_id, false);
	}

	*grp_id_const = *grp_id_const + *baseline_const;
	*ndvi_grp_interact = *ndvi_grp_interact + *baseline_ndvi;
	*ppt_grp_interact = *ppt_grp_interact + *baseline_ppt;

	ap->grp_id = *grp_id;

	double ndvi = ap->getNDVI("Normal");
	double ppt = ap->getPPT("Normal");

	double ln_ndvi = log(ndvi);
	double ln_ppt = log(ppt);

	double biomass = *grp_id_const + (ln_ndvi * *ndvi_grp_interact) + (ln_ppt * *ppt_grp_interact);
	return biomass;
}

void BiomassDriver::calcConfidence(int year, double biomass, double* lower, double* upper, double* level)
{
	double* baseline_const = new double;
	double* baseline_ndvi = new double;
	double* baseline_ppt = new double;

	string* grp_id = new string;
	// Populate interact fields from database
	double* grp_id_const = new double;
	double* ndvi_grp_interact = new double;
	double* ppt_grp_interact = new double;

	double ndvi = ap->getNDVI("Normal");
	double ppt = ap->getPPT("Normal");

	double ln_ndvi = log(ndvi);
	double ln_ppt = log(ppt);

	try
	{
		bdio->query_biogroup_coefs(0, baseline_const, baseline_ndvi, baseline_ppt, grp_id, true);
	}
	catch (RVS::DataManagement::DataNotFoundException &ex)
	{

	}
	try
	{
		bdio->query_biogroup_coefs(ap->BPS_NUM(), grp_id_const, ndvi_grp_interact, ppt_grp_interact, grp_id, true);
	}
	catch (RVS::DataManagement::DataNotFoundException &ex)
	{
		bdio->query_biogroup_coefs(ap->BPS_NUM(true), grp_id_const, ndvi_grp_interact, ppt_grp_interact, grp_id, true);
	}

	// Begin confidence interval code

	double yhat = exp(biomass) * SMEAR;
	double ln_yhat = biomass;

	double n = 1192;  // make this dynamic
	double p = 102;   // make this dynamic

	double tval = 1.9621475777951;  // make this dynamic

	double s2y = calc_s2b(grp_id, &ln_ndvi, &ln_ppt);
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

/*
double BiomassDriver::calcHerbBiomass(int year)
{

	double ndvi = ap->getNDVI(year);
	double biomass = 139.07 * exp(0.0004 * ndvi);
	//double biomass = 152.03 * exp(0.0004 * ndvi);
	return biomass;
}
*/

double BiomassDriver::calcHerbReduction(double totalShrubCover)
{
	//$$ TODO OMG THE MAGIC NUMBERS
	//if (totalShrubCover > 85) totalShrubCover = 85;
	//double val = -0.0004 * pow(totalShrubCover, 3) + 0.0458 * pow(totalShrubCover, 2) + 4.5394;
	double val = -.214 * log(100 - totalShrubCover) + 1.15;
	return val;
}

double BiomassDriver::calcAttenuation(double herbBiomass)
{
	double herbIndex = ap->herbCover * ap->herbHeight;
	double shrubIndex = ap->shrubCover * ap->shrubHeight;

	double ratio = herbIndex / shrubIndex;
	if (ratio > 1)
	{
		ratio = 1 / ratio;
	}
	return herbBiomass * ratio;
}

void BiomassDriver::growHerbs(double* herbCover, double* herbHeight, double oldBiomass, double newBiomass)
{
	double ratio = *herbCover / *herbHeight;
	double newProduction = newBiomass;

	double* coverRate = new double;
	double* herbRate = new double;

	bdio->query_herb_growth_coefs(ap->BPS_MODEL_NUM(), coverRate, herbRate);

	double cover = (*coverRate * newProduction);
	double height = *herbRate * newProduction * 10;

	if (cover > 100)
	{
		*herbCover = 95;
	}
	else
	{
		*herbCover = cover;
	}
	*herbHeight = height;
}

double BiomassDriver::calc_s2b(string* grp_id, double* lnNDVI, double* lnPPT)
{
	int index = bdio->find_group_index(grp_id);

	double** dummy = generate_dummy_variables(index, lnPPT, lnNDVI);
	double** dummyTrans = matrix_trans(dummy, 1, 102);

	double** temp = matrix_mult(covariance_matrix, 102, 102, dummyTrans, 102, 1);

	double** s2b = matrix_mult(dummy, 1, 102, temp, 102, 1);

	ap->s2y = s2b[0][0];

	return s2b[0][0];
}

double** BiomassDriver::matrix_mult(double** A, int aRow, int aCol, double** B, int bRow, int bCol)
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

double** BiomassDriver::matrix_trans(double** A, int aRow, int aCol)
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

double** BiomassDriver::generate_dummy_variables(int index, double* lnPPT, double* lnNDVI)
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