#include "BiomassEquations.h"


double RVS::Biomass::BiomassEquations::eq_PCH(double cf1, double cf2, double height)
{
	double result = 0;
	result = cf1 + cf2 * log10(height);
	result = pow(10, result);
	return result;
}

void RVS::Biomass::BiomassEquations::getToken(std::string inStr)
{
	//string retStr = "";
 //           
	//char[] chars = inStr.ToCharArray();
	//bool isNumber = false;

	//if (Char.IsDigit(chars[0]))
	//{
	//	isNumber = true;
	//}

	//for (int i = 0; i < inStr.Length; i++)
	//{
	//	if (Char.IsDigit(chars[i]) == isNumber)
	//	{
	//		retStr += chars[i].ToString();
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}

	//return retStr;
}


