#include "BiomassEquations.h"

double RVS::Biomass::BiomassEquations::eq_BAT(int equationNumber, double* coefs, map<string, double>* params)
{
	double biomass = 0.0;
	//$$ TODO add bounds checking to arrays


	if (equationNumber <= 168 || (equationNumber >= 791 && equationNumber <= 807) || equationNumber == 1068)
	{
		biomass = eq_165(coefs[0], coefs[1], params->at("COV"));
	}
	else if (equationNumber == 201)
	{
		biomass = eq_201(coefs[0], coefs[1], params->at("COV"));
	}
	else if (equationNumber == 202 || equationNumber == 1087 || equationNumber == 1096 || equationNumber == 1105 || equationNumber == 1109)
	{
		biomass = eq_202(coefs[0], coefs[1], params->at("COV"));
	}
	else if (equationNumber <= 629)
	{
		biomass = eq_basicBAT(coefs[0], coefs[1], params->at("LEN"), params->at("WID"));
	}
	else if (equationNumber <= 639)
	{
		try
		{
			biomass = eq_636(coefs[0], coefs[1], params->at("LEN"), params->at("WID"), params->at("HT"));
		}
		catch (exception e)
		{
			biomass = eq_636_2(coefs[0], coefs[1], params->at("VOL"));
		}
	}
	else if (equationNumber == 743 || equationNumber == 831 || equationNumber == 832)
	{
		biomass = eq_743(coefs[0], coefs[1], params->at("COV"), params->at("HT"));
	}
	else if (equationNumber == 998)
	{
		biomass = eq_998(coefs[0], coefs[1], params->at("WID"));
	}
	else if (equationNumber == 999)
	{
		biomass = eq_999(coefs[0], coefs[1], coefs[2], params->at("LEN"), params->at("WID"));
	}
	else if (equationNumber == 1000)
	{
		biomass = eq_1000(coefs[0], coefs[1], coefs[2], coefs[3], params->at("LEN"), params->at("WID"), params->at("HT"));
	}
	else if (equationNumber == 1001)
	{
		biomass = eq_999(coefs[0], coefs[1], coefs[2], params->at("LEN"), params->at("HT"));
	}
	else if (equationNumber == 1002)
	{
		biomass = eq_1002(coefs[0], coefs[1], params->at("VOL"));
	}
	else if (equationNumber == 1008 || equationNumber == 1016)
	{
		biomass = eq_1008(coefs[0], coefs[1], coefs[2], coefs[3], params->at("LEN"), params->at("WID"), params->at("HT"));
	}
	else if (equationNumber == 1012)
	{
		biomass = eq_1012(coefs[0], coefs[1], coefs[2], params->at("LEN"), params->at("HT"));
	}
	else if (equationNumber >= 1025 && equationNumber <= 1031)
	{
		biomass = eq_1012(coefs[0], coefs[1], coefs[2], params->at("LEN"), params->at("WID"));
	}
	else if (equationNumber == 1058 || equationNumber == 1137)
	{
		biomass = eq_1058(coefs[0], coefs[1], coefs[2], params->at("COV"));
	}
	else if (equationNumber == 1067 || equationNumber == 1097 || equationNumber == 1103 || equationNumber == 1106)
	{
		biomass = eq_202(coefs[0], coefs[1], params->at("LEN"));
	}
	else if (equationNumber == 1136)
	{
		biomass = eq_999(coefs[0], coefs[1], coefs[2], params->at("COV"), params->at("LEN"));
	}
	else if (equationNumber == 1152 || equationNumber == 1153)
	{
		biomass = eq_1153(coefs[0], coefs[1], coefs[2], params->at("LEN"), params->at("WID"), params->at("HT"));
	}
	else if (equationNumber == 1160)
	{
		biomass = eq_1160(coefs[0], coefs[1], params->at("VOL"));
	}
	else if (equationNumber == 1161)
	{
		biomass = eq_1161(coefs[0], coefs[1], params->at("VOL"));
	}
	else
	{
		//$$ Throw not found exception
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

double RVS::Biomass::BiomassEquations::eq_165(double cf1, double cf2, double cover)
{
	double biomass = cf1 + cf2 * cover;
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_201(double cf1, double cf2, double cover)
{
	double biomass = cf1 + cf2 * cover;
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_202(double cf1, double cf2, double cover)
{
	double biomass = cf1 + cf2 * log(cover);
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_636(double cf1, double cf2, double length, double width, double height)
{
	double biomass = cf1 + cf2 * log((length * width * height) / 1000); // Volume
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_636_2(double cf1, double cf2, double vol)
{
	double biomass = cf1 + cf2 * log(vol / 1000); // Volume // EQUATIONS CALL FOR dm^3
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_743(double cf1, double cf2, double cover, double height)
{
	double biomass = cf1 + cf2 * cover * height; 
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_998(double cf1, double cf2, double width)
{
	double biomass = cf1 + (cf2 * log10(width));
	biomass = pow(10, biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_999(double cf1, double cf2, double cf3, double length, double width)
{
	double biomass = cf1 + cf2 * log10(length) + cf3 * log10(width);
	biomass = pow(10, biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_1000(double cf1, double cf2, double cf3, double cf4, double length, double width, double height)
{
	double biomass = cf1 + cf2 * log10(length) + cf3 * log10(width) + cf4 * log10(height);
	biomass = pow(10, biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_1002(double cf1, double cf2, double vol)
{
	double biomass = cf1 + cf2 * (vol / 1000);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_1008(double cf1, double cf2, double cf3, double cf4, double length, double width, double height)
{
	double biomass = cf1 + cf2 * log(length) + cf3 * log(width) + cf4 * log(height);
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_1153(double cf1, double cf2, double cf3, double length, double width, double height)
{
	double biomass = cf1 + cf2 * log10(length * width) + cf3 * log10(height);
	biomass = pow(10, biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_1012(double cf1, double cf2, double cf3, double p1, double p2)
{
	double biomass = cf1 + cf2 * log(p1) + cf3 * log(p2);
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_1058(double cf1, double cf2, double cf3, double p1)
{
	double biomass = cf1 + cf2 * p1 + cf3 * sqrt(p1);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_basicBAT(double cf1, double cf2, double length, double width)
{
	double biomass = cf1 + cf2 * log(length + width);
	biomass = exp(biomass);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_1160(double cf1, double cf2, double p1)
{
	double biomass = (cf1 + cf2 * p1);
	return biomass;
}

double RVS::Biomass::BiomassEquations::eq_1161(double cf1, double cf2, double p1)
{
	double biomass = cf1 * (p1 / cf2);
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


