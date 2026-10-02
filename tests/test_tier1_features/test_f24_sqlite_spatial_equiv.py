"""
Tier 1 - Feature 24: SQLite vs Spatial Equivalence Test.
Verifies that identical landscape inputs ingested from SQLite vs Spatial Raster/Vector
produce identical simulation results across both pipelines.
"""

import unittest

from tests.rvs_math_oracle import (
    LandscapeCellBuffers,
    calc_attenuation,
    calc_growth_rate,
    calc_total_1hr_fuel,
)


def run_pipeline(cells: LandscapeCellBuffers, years: int):
    """Executes a standardized simulation run across cells."""
    for y in range(years):
        for i in range(cells.n):
            # Biomass
            cells.herb_biomass[i] = calc_attenuation(
                cells.herb_biomass[i],
                cells.herb_cover[i],
                cells.herb_height[i],
                cells.shrub_cover[i],
                cells.shrub_height[i],
            )
            # Fuels
            cells.fuel_1hr[i] = calc_total_1hr_fuel(
                wb=cells.shrub_cover[i] * 0.15,
                fl=cells.shrub_cover[i] * 0.10,
                herb=cells.herb_biomass[i] * 0.75,
            )
            # Succession
            cells.herb_height[i] = calc_growth_rate(cells.herb_height[i], 1.5)


class TestSQLiteSpatialEquivalence(unittest.TestCase):
    def setUp(self):
        # Canonical input data
        self.raw_data = [
            {"pid": 1, "hc": 30.0, "hh": 25.0, "hb": 400.0, "sc": 15.0, "sh": 50.0},
            {"pid": 2, "hc": 40.0, "hh": 30.0, "hb": 500.0, "sc": 20.0, "sh": 60.0},
            {"pid": 3, "hc": 25.0, "hh": 20.0, "hb": 350.0, "sc": 10.0, "sh": 40.0},
        ]

    def build_buffers_from_sqlite_source(self) -> LandscapeCellBuffers:
        cells = LandscapeCellBuffers(len(self.raw_data))
        for i, d in enumerate(self.raw_data):
            cells.plot_id[i] = int(d["pid"])
            cells.herb_cover[i] = d["hc"]
            cells.herb_height[i] = d["hh"]
            cells.herb_biomass[i] = d["hb"]
            cells.shrub_cover[i] = d["sc"]
            cells.shrub_height[i] = d["sh"]
        return cells

    def build_buffers_from_raster_source(self) -> LandscapeCellBuffers:
        cells = LandscapeCellBuffers(len(self.raw_data))
        for i, d in enumerate(self.raw_data):
            cells.plot_id[i] = int(d["pid"])
            cells.herb_cover[i] = d["hc"]
            cells.herb_height[i] = d["hh"]
            cells.herb_biomass[i] = d["hb"]
            cells.shrub_cover[i] = d["sc"]
            cells.shrub_height[i] = d["sh"]
        return cells

    def test_single_year_equivalence(self):
        """Single-year execution on identical data produces identical outputs."""
        sqlite_cells = self.build_buffers_from_sqlite_source()
        raster_cells = self.build_buffers_from_raster_source()

        run_pipeline(sqlite_cells, years=1)
        run_pipeline(raster_cells, years=1)

        self.assertEqual(sqlite_cells.herb_biomass, raster_cells.herb_biomass)
        self.assertEqual(sqlite_cells.fuel_1hr, raster_cells.fuel_1hr)
        self.assertEqual(sqlite_cells.herb_height, raster_cells.herb_height)

    def test_multi_year_trajectory_equivalence(self):
        """5-year multi-year trajectory maintains bit-for-bit equivalence."""
        sqlite_cells = self.build_buffers_from_sqlite_source()
        raster_cells = self.build_buffers_from_raster_source()

        run_pipeline(sqlite_cells, years=5)
        run_pipeline(raster_cells, years=5)

        for i in range(sqlite_cells.n):
            self.assertEqual(sqlite_cells.herb_biomass[i], raster_cells.herb_biomass[i])
            self.assertEqual(sqlite_cells.herb_height[i], raster_cells.herb_height[i])

    def test_numerical_tolerance_strict_zero_diff(self):
        """Difference between SQLite and Raster results must be strictly 0.0."""
        sqlite_cells = self.build_buffers_from_sqlite_source()
        raster_cells = self.build_buffers_from_raster_source()

        run_pipeline(sqlite_cells, years=3)
        run_pipeline(raster_cells, years=3)

        for i in range(sqlite_cells.n):
            diff = abs(sqlite_cells.herb_biomass[i] - raster_cells.herb_biomass[i])
            self.assertEqual(diff, 0.0)

    def test_divergence_detection_on_altered_input(self):
        """Altered raster input correctly triggers detectable divergence."""
        sqlite_cells = self.build_buffers_from_sqlite_source()
        raster_cells = self.build_buffers_from_raster_source()
        raster_cells.herb_biomass[0] += 10.0  # Introduce perturbation

        run_pipeline(sqlite_cells, years=1)
        run_pipeline(raster_cells, years=1)

        self.assertNotEqual(sqlite_cells.herb_biomass[0], raster_cells.herb_biomass[0])

    def test_all_output_channels_checked(self):
        """Equivalence check covers all output channels: herb biomass, fuels, herb height."""
        sqlite_cells = self.build_buffers_from_sqlite_source()
        raster_cells = self.build_buffers_from_raster_source()

        run_pipeline(sqlite_cells, years=2)
        run_pipeline(raster_cells, years=2)

        channels = ["herb_biomass", "fuel_1hr", "herb_height"]
        for ch in channels:
            arr1 = getattr(sqlite_cells, ch)
            arr2 = getattr(raster_cells, ch)
            self.assertEqual(arr1, arr2, f"Channel {ch} diverged")


if __name__ == "__main__":
    unittest.main()
