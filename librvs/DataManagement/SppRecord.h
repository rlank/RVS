/// ********************************************************** ///
/// Name: SppRecord.h                                          ///
/// Desc: Contains information about a specific species record ///
/// associated with an AnalysisPlot. Currently these are all   ///
/// shrub records.                                             ///
/// Base Class(es): none                                       ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#pragma once

#ifndef SPP_RECORD_H
#define SPP_RECORD_H

#include <string>
#include <string_view>
#include <vector>

#include "../RVSDBNAMES.h"
#include "DIO.h"

namespace RVS { namespace Biomass { class BiomassDriver; } }
namespace RVS { namespace Biomass { class BiomassEqDriver; } }
namespace RVS { namespace Fuels   { class FuelsDriver; } }
namespace RVS { namespace Succession { class SuccessionDriver; } }
namespace RVS { namespace Disturbance { class DisturbanceDriver; } }

namespace RVS
{
namespace DataManagement
{
	class SppRecord
	{
		friend class RVS::Biomass::BiomassDriver;
		friend class RVS::Fuels::FuelsDriver;
		friend class RVS::Biomass::BiomassEqDriver;
		friend class RVS::Succession::SuccessionDriver;
		friend class RVS::Disturbance::DisturbanceDriver;

	public:
		SppRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		SppRecord(std::string spp_code, double height, double cover, std::string dom_spp);
		virtual ~SppRecord() = default;

		SppRecord(const SppRecord&) = default;
		SppRecord& operator=(const SppRecord&) = default;
		SppRecord(SppRecord&&) noexcept = default;
		SppRecord& operator=(SppRecord&&) noexcept = default;

		// Metadata
		[[nodiscard]] const std::string& DOM_SPP() const noexcept { return dom_spp; }
		[[nodiscard]] const std::string& SPP_CODE() const noexcept { return spp_code; }

		// Morphological values
		[[nodiscard]] double HEIGHT() const noexcept { return height; }
		[[nodiscard]] double COVER() const noexcept { return cover; }
		[[nodiscard]] double WIDTH() const noexcept { return width; }
		[[nodiscard]] double STEMS_PER_ACRE() const noexcept { return stemsPerAcre; }
		[[nodiscard]] double STEMSPERACRE() const noexcept { return stemsPerAcre; }

		// Equation lookup numbers
		[[nodiscard]] int PCHEQNUM() const noexcept { return pchEqNum; }
		[[nodiscard]] int BATEQNUM() const noexcept { return batEqNum; }

		// Biomass result for shrub (individual biomass). Converts internal g/ac to lbs/ac
		[[nodiscard]] double SHRUB_SINGLE_BIOMASS() const noexcept { return shrubBiomass * GRAMS_TO_POUNDS; }
		[[nodiscard]] double SHRUB_EX_BIOMASS() const noexcept { return exShrubBiomass * GRAMS_TO_POUNDS; }
		static constexpr double GRAMS_TO_POUNDS = 0.00220462;

		// Return a parameter (length, width, height, vol, cov) by name
		[[nodiscard]] double requestValue(std::string_view parameterName) const;

	protected:
		/// General variables ///
		std::string dom_spp;       // Dominant species name
		std::string spp_code;      // Dominant species PLANTS code
		double height = 0.0;       // Crown height (cm)
		double cover = 0.0;        // ABSOLUTE cover (0-100) %
		double width = 0.0;        // Average leaf width (cm)
		double stemsPerAcre = 0.0; // Calculated value (in Biomass)

		virtual void initialize_object();
		virtual void buildRecord(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt);
		virtual void buildRecord(std::string spp_code, double height, double cover, std::string dom_spp);

		// Biomass values
		double shrubBiomass = 0.0;   // grams
		double exShrubBiomass = 0.0; // grams
		int pchEqNum = 0;
		int batEqNum = 0;

		// Fuels collection
		double fuel1hr = 0.0;
		double fuel10hr = 0.0;
		double fuel100hr = 0.0;
		double fuel1000hr = 0.0;
	};
}
}

#endif // SPP_RECORD_H