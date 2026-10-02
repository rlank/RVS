/// ********************************************************** ///
/// Name: IDataLoader.h                                        ///
/// Desc: Abstract DataLoader interface decoupling simulation   ///
///       from specific storage schemas and database handles.  ///
/// Standard: C++20                                            ///
/// Standards: rvs-optimization-standards, cpp-coding-standards ///
/// ********************************************************** ///

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace RVS::DataManagement {

/// Contiguous Structure-of-Arrays buffers for N landscape cells.
/// Enforces Data-Oriented Design (DoD) per rvs-optimization-standards.
struct LandscapeCellBuffers {
    size_t num_cells{0};
    std::vector<int64_t> plot_id;
    std::vector<int32_t> bps_code;
    std::vector<double> shrub_cover;
    std::vector<double> shrub_height;
    std::vector<double> herb_cover;
    std::vector<double> herb_height;
    std::vector<double> herb_biomass;
    std::vector<double> ndvi;
    std::vector<double> precip;
    std::vector<double> elevation;
    std::vector<double> fuel_1hr;
    std::vector<double> fuel_10hr;
    std::vector<double> fuel_100hr;
    std::vector<double> fuel_1000hr;

    LandscapeCellBuffers() = default;
    explicit LandscapeCellBuffers(size_t n)
        : num_cells(n),
          plot_id(n, 0),
          bps_code(n, 0),
          shrub_cover(n, 0.0),
          shrub_height(n, 0.0),
          herb_cover(n, 0.0),
          herb_height(n, 0.0),
          herb_biomass(n, 0.0),
          ndvi(n, 0.0),
          precip(n, 0.0),
          elevation(n, 0.0),
          fuel_1hr(n, 0.0),
          fuel_10hr(n, 0.0),
          fuel_100hr(n, 0.0),
          fuel_1000hr(n, 0.0) {}

    [[nodiscard]] size_t size() const noexcept { return num_cells; }
    [[nodiscard]] bool empty() const noexcept { return num_cells == 0; }
};

/// Segmented CSR (Compressed Sparse Row) buffers for M shrubs across N cells.
/// For cell i in [0, N), its associated shrubs reside in range [cell_offsets[i], cell_offsets[i+1]).
struct LandscapeShrubBuffers {
    size_t num_cells{0};
    size_t num_shrubs{0};
    std::vector<size_t> cell_offsets;   // Length N + 1
    std::vector<uint32_t> species_id;    // Length M
    std::vector<double> cover;           // Length M
    std::vector<double> height;          // Length M
    std::vector<double> biomass;         // Length M
    std::vector<double> stems_per_acre;  // Length M

    LandscapeShrubBuffers() = default;
    LandscapeShrubBuffers(size_t n, size_t m)
        : num_cells(n),
          num_shrubs(m),
          cell_offsets(n + 1, 0),
          species_id(m, 0),
          cover(m, 0.0),
          height(m, 0.0),
          biomass(m, 0.0),
          stems_per_acre(m, 0.0) {}

    /// Validates the 5 canonical CSR invariants:
    /// 1. cell_offsets.size() == num_cells + 1
    /// 2. cell_offsets[0] == 0
    /// 3. cell_offsets is monotonic non-decreasing
    /// 4. cell_offsets[num_cells] == num_shrubs
    /// 5. All attribute buffer lengths equal num_shrubs
    [[nodiscard]] bool validate() const noexcept {
        if (cell_offsets.size() != num_cells + 1) return false;
        if (cell_offsets.empty() || cell_offsets[0] != 0) return false;
        for (size_t i = 0; i < num_cells; ++i) {
            if (cell_offsets[i] > cell_offsets[i + 1]) return false;
        }
        if (cell_offsets[num_cells] != num_shrubs) return false;
        if (species_id.size() != num_shrubs) return false;
        if (cover.size() != num_shrubs) return false;
        if (height.size() != num_shrubs) return false;
        if (biomass.size() != num_shrubs) return false;
        if (stems_per_acre.size() != num_shrubs) return false;
        return true;
    }
};

/// Pre-compiled biomass equation record.
struct BioEquationRecord {
    uint16_t eqn_num{0};
    uint8_t eqn_type{0};
    double cf1{0.0};
    double cf2{0.0};
    double cf3{0.0};
    double cf4{0.0};
};

/// Succession cohort transition record.
struct BpsCohortRecord {
    uint8_t cohort{0};
    char cohort_type{' '};
    double start_age{0.0};
    double end_age{0.0};
    double min_cc{0.0};
    double max_cc{0.0};
    double min_ht{0.0};
    double max_ht{0.0};
    double gr_ht{0.0};
    double gr_cov{0.0};
};

/// In-memory pre-compiled reference catalog (Feature 15).
struct ReferenceData {
    std::vector<BioEquationRecord> bio_equations;
    std::unordered_map<std::string, uint16_t> bio_crosswalk;
    std::unordered_map<std::string, uint16_t> fuel_crosswalk;
    std::unordered_map<uint32_t, std::vector<BpsCohortRecord>> growth_rates;
    std::vector<double> shrub_eq_cf1;
    std::vector<double> shrub_eq_cf2;

    [[nodiscard]] const BioEquationRecord* get_bio_equation(uint16_t eqn_num) const noexcept {
        if (eqn_num < bio_equations.size() && bio_equations[eqn_num].eqn_num != 0) {
            return &bio_equations[eqn_num];
        }
        return nullptr;
    }
};

/// Abstract DataLoader interface specification (Feature 16).
///
/// Decouples simulation subroutines and batch drivers from storage schemas,
/// relational tables, and filesystem formats.
class IDataLoader {
public:
    virtual ~IDataLoader() = default;

    /// Ingests landscape cells and sub-canopy shrubs into canonical SoA buffers.
    [[nodiscard]] virtual std::pair<LandscapeCellBuffers, LandscapeShrubBuffers> load_landscape() = 0;

    /// Loads static in-memory reference catalog (equations, crosswalks, growth rates).
    [[nodiscard]] virtual ReferenceData load_reference_data() = 0;

    /// Persists annual simulation state back to the storage target.
    virtual void write_landscape(int year, const LandscapeCellBuffers& cells, const LandscapeShrubBuffers& shrubs) = 0;
};

} // namespace RVS::DataManagement
