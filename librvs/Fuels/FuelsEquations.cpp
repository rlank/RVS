#include "FuelsEquations.h"

namespace RVS
{
namespace Fuels
{
	double FuelsEquations::calcFuels(
		const int equationType,
		std::span<const double> coefs,
		std::span<const double> params) noexcept
	{
		switch (equationType)
		{
		case 1:
			if (coefs.size() >= 2 && params.size() >= 1)
			{
				return fuels_eq_1(coefs[0], coefs[1], params[0]);
			}
			break;

		case 2:
			if (coefs.size() >= 2 && params.size() >= 1)
			{
				return fuels_eq_2(coefs[0], coefs[1], params[0]);
			}
			break;

		case 13:
			if (coefs.size() >= 2 && params.size() >= 1)
			{
				return fuels_eq_13(coefs[0], coefs[1], params[0]);
			}
			break;

		case 15:
			if (coefs.size() >= 2 && params.size() >= 2)
			{
				return fuels_eq_15(coefs[0], coefs[1], params[0], params[1]);
			}
			break;

		case 17:
			if (coefs.size() >= 3 && params.size() >= 2)
			{
				return fuels_eq_17(coefs[0], coefs[1], coefs[2], params[0], params[1]);
			}
			break;

		case 42:
			if (coefs.size() >= 4 && params.size() >= 3)
			{
				return fuels_eq_42(coefs[0], coefs[1], coefs[2], coefs[3], params[0], params[1], params[2]);
			}
			break;

		case 46:
			if (coefs.size() >= 3 && params.size() >= 2)
			{
				return fuels_eq_46(coefs[0], coefs[1], coefs[2], params[0], params[1]);
			}
			break;

		default:
			break;
		}

		return 0.0;
	}
}
}
