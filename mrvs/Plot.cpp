#include "Plot.h"


rvs::Plot::Plot()
{
}


rvs::Plot::~Plot()
{
}

double rvs::Plot::getNDVI(string level, bool useRand)
{
	double ndvi = 0;
	if (level.compare("Dry") == 0)
	{
		ndvi = ndviValues[0];
	}
	else if (level.compare("Mid-Dry") == 0)
	{
		ndvi = ndviValues[1];
	}
	else if (level.compare("Mid-Wet") == 0)
	{
		ndvi = ndviValues[3];
	}
	else if (level.compare("Wet") == 0)
	{
		ndvi = ndviValues[4];
	}
	else if (level.compare("Normal") == 0)
	{
		ndvi = ndviValues[2];
	}
	else
	{
		ndvi = ndviValues[2];
	}

	if (useRand)
	{
		int r = rand() % 15 + 90;
		ndvi = ndvi * (r / 100);
	}

	return ndvi;
}

double rvs::Plot::getPPT(string level, bool useRand)
{
	double ppt = 0;
	if (level.compare("Dry") == 0)
	{
		ppt = precipValues[0];
	}
	else if (level.compare("Mid-Dry") == 0)
	{
		ppt = precipValues[1];
	}
	else if (level.compare("Mid-Wet") == 0)
	{
		ppt = precipValues[3];
	}
	else if (level.compare("Wet") == 0)
	{
		ppt = precipValues[4];
	}
	else if (level.compare("Normal") == 0)
	{
		ppt = precipValues[2];
	}
	else
	{
		ppt = precipValues[2];
	}


	if (useRand)
	{
		int r = rand() % 15 + 90;
		ppt = ppt * (r / 100);
	}

	return ppt;
}