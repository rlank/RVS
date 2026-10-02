#include "BiomassEquations.h"

#include <cmath>
#include <stdexcept>

namespace RVS
{
namespace Biomass
{

double BiomassEquations::eq_BAT(
	const int equationNumber,
	std::span<const double> coefs,
	const std::map<std::string, double>& params)
{
	if (equationNumber <= 0)
	{
		return 0.0;
	}

	if (coefs.size() < 2)
	{
		return 0.0;
	}

	auto get_val = [&params](const std::string& key) -> double {
		const auto it = params.find(key);
		if (it != params.end())
		{
			return it->second;
		}
		return 0.0;
	};

	double biomass = 0.0;

	if (equationNumber <= 168 || (equationNumber >= 791 && equationNumber <= 807) || equationNumber == 1068)
	{
		biomass = eq_165(coefs[0], coefs[1], get_val("COV"));
	}
	else if (equationNumber == 201)
	{
		biomass = eq_201(coefs[0], coefs[1], get_val("COV"));
	}
	else if (equationNumber == 202 || equationNumber == 1087 || equationNumber == 1096 || equationNumber == 1105 || equationNumber == 1109)
	{
		biomass = eq_202(coefs[0], coefs[1], get_val("COV"));
	}
	else if (equationNumber <= 629)
	{
		biomass = eq_basicBAT(coefs[0], coefs[1], get_val("LEN"), get_val("WID"));
	}
	else if (equationNumber <= 639)
	{
		try
		{
			biomass = eq_636(coefs[0], coefs[1], get_val("LEN"), get_val("WID"), get_val("HT"));
		}
		catch (const std::exception&)
		{
			biomass = eq_636_2(coefs[0], coefs[1], get_val("VOL"));
		}
	}
	else if (equationNumber == 743 || equationNumber == 831 || equationNumber == 832)
	{
		biomass = eq_743(coefs[0], coefs[1], get_val("COV"), get_val("HT"));
	}
	else if (equationNumber == 998)
	{
		biomass = eq_998(coefs[0], coefs[1], get_val("WID"));
	}
	else if (equationNumber == 999)
	{
		if (coefs.size() < 3) throw std::invalid_argument("Equation 999 requires 3 coefficients");
		biomass = eq_999(coefs[0], coefs[1], coefs[2], get_val("LEN"), get_val("WID"));
	}
	else if (equationNumber == 1000)
	{
		if (coefs.size() < 4) throw std::invalid_argument("Equation 1000 requires 4 coefficients");
		biomass = eq_1000(coefs[0], coefs[1], coefs[2], coefs[3], get_val("LEN"), get_val("WID"), get_val("HT"));
	}
	else if (equationNumber == 1001)
	{
		if (coefs.size() < 3) throw std::invalid_argument("Equation 1001 requires 3 coefficients");
		biomass = eq_999(coefs[0], coefs[1], coefs[2], get_val("LEN"), get_val("HT"));
	}
	else if (equationNumber == 1002)
	{
		biomass = eq_1002(coefs[0], coefs[1], get_val("VOL"));
	}
	else if (equationNumber == 1008 || equationNumber == 1016)
	{
		if (coefs.size() < 4) throw std::invalid_argument("Equation 1008/1016 requires 4 coefficients");
		biomass = eq_1008(coefs[0], coefs[1], coefs[2], coefs[3], get_val("LEN"), get_val("WID"), get_val("HT"));
	}
	else if (equationNumber == 1012)
	{
		if (coefs.size() < 3) throw std::invalid_argument("Equation 1012 requires 3 coefficients");
		biomass = eq_1012(coefs[0], coefs[1], coefs[2], get_val("LEN"), get_val("HT"));
	}
	else if (equationNumber >= 1025 && equationNumber <= 1031)
	{
		if (coefs.size() < 3) throw std::invalid_argument("Equation 1025-1031 requires 3 coefficients");
		biomass = eq_1012(coefs[0], coefs[1], coefs[2], get_val("LEN"), get_val("WID"));
	}
	else if (equationNumber == 1058 || equationNumber == 1137)
	{
		if (coefs.size() < 3) throw std::invalid_argument("Equation 1058/1137 requires 3 coefficients");
		biomass = eq_1058(coefs[0], coefs[1], coefs[2], get_val("COV"));
	}
	else if (equationNumber == 1067 || equationNumber == 1097 || equationNumber == 1103 || equationNumber == 1106)
	{
		biomass = eq_202(coefs[0], coefs[1], get_val("LEN"));
	}
	else if (equationNumber == 1136)
	{
		if (coefs.size() < 3) throw std::invalid_argument("Equation 1136 requires 3 coefficients");
		biomass = eq_999(coefs[0], coefs[1], coefs[2], get_val("COV"), get_val("LEN"));
	}
	else if (equationNumber == 1152 || equationNumber == 1153)
	{
		if (coefs.size() < 3) throw std::invalid_argument("Equation 1152/1153 requires 3 coefficients");
		biomass = eq_1153(coefs[0], coefs[1], coefs[2], get_val("LEN"), get_val("WID"), get_val("HT"));
	}
	else if (equationNumber == 1160)
	{
		biomass = eq_1160(coefs[0], coefs[1], get_val("VOL"));
	}
	else if (equationNumber == 1161)
	{
		biomass = eq_1161(coefs[0], coefs[1], get_val("VOL"));
	}
	else
	{
		throw std::invalid_argument("Biomass equation number not recognized: " + std::to_string(equationNumber));
	}

	return biomass;
}

double BiomassEquations::eq_PCH(const double cf1, const double cf2, const double height) noexcept
{
	const double result = cf1 + cf2 * std::log10(height);
	return std::pow(10.0, result);
}

double BiomassEquations::eq_201(const double cf1, const double cf2, const double cover) noexcept
{
	const double exponent = cf1 + cf2 * cover;
	return std::exp(exponent);
}

double BiomassEquations::eq_202(const double cf1, const double cf2, const double cover) noexcept
{
	const double exponent = cf1 + cf2 * std::log(cover);
	return std::exp(exponent);
}

double BiomassEquations::eq_636(
	const double cf1,
	const double cf2,
	const double length,
	const double width,
	const double height) noexcept
{
	const double exponent = cf1 + cf2 * std::log((length * width * height) / 1000.0);
	return std::exp(exponent);
}

double BiomassEquations::eq_636_2(const double cf1, const double cf2, const double vol) noexcept
{
	const double exponent = cf1 + cf2 * std::log(vol / 1000.0);
	return std::exp(exponent);
}

double BiomassEquations::eq_998(const double cf1, const double cf2, const double width) noexcept
{
	const double exponent = cf1 + (cf2 * std::log10(width));
	return std::pow(10.0, exponent);
}

double BiomassEquations::eq_999(
	const double cf1,
	const double cf2,
	const double cf3,
	const double length,
	const double width) noexcept
{
	const double exponent = cf1 + cf2 * std::log10(length) + cf3 * std::log10(width);
	return std::pow(10.0, exponent);
}

double BiomassEquations::eq_1000(
	const double cf1,
	const double cf2,
	const double cf3,
	const double cf4,
	const double length,
	const double width,
	const double height) noexcept
{
	const double exponent = cf1 + cf2 * std::log10(length) + cf3 * std::log10(width) + cf4 * std::log10(height);
	return std::pow(10.0, exponent);
}

double BiomassEquations::eq_1008(
	const double cf1,
	const double cf2,
	const double cf3,
	const double cf4,
	const double length,
	const double width,
	const double height) noexcept
{
	const double exponent = cf1 + cf2 * std::log(length) + cf3 * std::log(width) + cf4 * std::log(height);
	return std::exp(exponent);
}

double BiomassEquations::eq_1012(
	const double cf1,
	const double cf2,
	const double cf3,
	const double p1,
	const double p2) noexcept
{
	const double exponent = cf1 + cf2 * std::log(p1) + cf3 * std::log(p2);
	return std::exp(exponent);
}

double BiomassEquations::eq_1058(
	const double cf1,
	const double cf2,
	const double cf3,
	const double p1) noexcept
{
	return cf1 + cf2 * p1 + cf3 * std::sqrt(p1);
}

double BiomassEquations::eq_basicBAT(
	const double cf1,
	const double cf2,
	const double length,
	const double width) noexcept
{
	const double exponent = cf1 + cf2 * std::log(length + width);
	return std::exp(exponent);
}

double BiomassEquations::eq_1153(
	const double cf1,
	const double cf2,
	const double cf3,
	const double length,
	const double width,
	const double height) noexcept
{
	const double exponent = cf1 + cf2 * std::log10(length * width) + cf3 * std::log10(height);
	return std::pow(10.0, exponent);
}

double BiomassEquations::shunt(
	[[maybe_unused]] std::string_view equation,
	[[maybe_unused]] const std::map<std::string, double>& variables)
{
	// Equation parsing stub for runtime expressions
	return 0.0;
}

} // namespace Biomass
} // namespace RVS
