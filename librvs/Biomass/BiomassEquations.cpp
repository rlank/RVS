#include "BiomassEquations.h"

double RVS::Biomass::BiomassEquations::eq_BAT(int equationNumber, double* coefs, map<string, double>* params)
{
	double biomass = 0.0;
	//$$ TODO add bounds checking to arrays
	switch (equationNumber)
	{
	case 201:
		biomass = eq_201(coefs[0], coefs[1], params->at("COV"));
		break;
	case 620:
	case 622:
	case 623:
		biomass = eq_basicBAT(coefs[0], coefs[1], params->at("LEN"), params->at("WID"));
		break;
	case 636:
		biomass = eq_636(coefs[0], coefs[1], params->at("LEN"), params->at("WID"), params->at("HT"));
		break;
	case 999:
		biomass = eq_999(coefs[0], coefs[1], coefs[2], params->at("LEN"), params->at("WID"));
		break;
	case 1153:
		biomass = eq_1153(coefs[0], coefs[1], coefs[2], params->at("LEN"), params->at("WID"), params->at("HT"));
		break;
	}

	return biomass;
}


double RVS::Biomass::BiomassEquations::eq_PCH(double cf1, double cf2, double height)
{
	double result = 0;
	result = cf1 + cf2 * log10(height);
	result = pow(10, result);
	return result;
}

double RVS::Biomass::BiomassEquations::eq_201(double cf1, double cf2, double cover)
{
	double biomass = cf1 + cf2 * cover;
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_636(double cf1, double cf2, double length, double width, double height)
{
	double biomass = cf1 + cf2 * log(length * width * height); // Volume
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_999(double cf1, double cf2, double cf3, double length, double width)
{
	double biomass = cf1 + cf2 * log(length) + cf3 * log(width);
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_1153(double cf1, double cf2, double cf3, double length, double width, double height)
{
	double biomass = cf1 + cf2 * log(length + width) + cf3 * log(height);
	biomass = exp(biomass);
	return biomass;
}


double RVS::Biomass::BiomassEquations::eq_basicBAT(double cf1, double cf2, double length, double width)
{
	double biomass = cf1 + cf2 * log(length + width);
	biomass = exp(biomass);
	return biomass;
}



double RVS::Biomass::BiomassEquations::shunt(string equation, map<string, double> vars)
{
	
	map<std::string, int> prec;
	prec.insert(pair<string, int>("ln", 4)); 
	prec.insert(pair<string, int>("^", 4));
	prec.insert(pair<string, int>("*", 3));
	prec.insert(pair<string, int>("/", 3));
	prec.insert(pair<string, int>("+", 2));
	prec.insert(pair<string, int>("-", 2));

	vector<string> tokens;
	//boost::split(tokens, equation, " ");  // This fails.
	stack<string> operators;
	stack<double> operands;

	/*
	for (int i = 0; i < sizeof(tokens) / sizeof(char); i++)
	{
		string t = tokens.at(i);
		if (t.compare(")") == 0)
		{
			// POP TIME
		}
		else if (t.compare("(") || boost::is_any_of(prec.begin()))
		{
			operands.push(t);
		}
		else
		{
			double v = atof(t.c_str());
			operators.push(v);
		}
	}
	*/


	return 0;
}


