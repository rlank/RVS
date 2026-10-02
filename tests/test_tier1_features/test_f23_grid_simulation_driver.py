"""
Tier 1 - Feature 23: In-Memory Grid Simulation Driver.
Verifies the simulation pipeline executing the annual cycle (Biomass -> Fuels ->
Succession -> Disturbance) purely on abstract in-memory SoA grid buffers.
"""

import unittest

from tests.rvs_math_oracle import (
    LandscapeCellBuffers,
    calc_attenuation,
    calc_fuel_percentages,
    calc_graze_biomass_reduction,
    calc_growth_rate,
    calc_total_1hr_fuel,
    eq_165,
)


class InMemorySimulationDriver:
    """Simulation engine driving annual ecological cycles over in-memory buffers."""

    def __init__(self, cells: LandscapeCellBuffers):
        self.cells = cells
        self.holdover_history = [[0.0] * cells.n for _ in range(3)]

    def run_year(self, year: int, has_fire: bool = False, has_graze: bool = False):
        n = self.cells.n

        # 1. Biomass: Calculate shrub biomass and attenuate herb biomass
        for i in range(n):
            shrub_b = eq_165(5.0, 1.8, self.cells.shrub_cover[i])
            self.cells.herb_biomass[i] = calc_attenuation(
                self.cells.herb_biomass[i],
                self.cells.herb_cover[i],
                self.cells.herb_height[i],
                self.cells.shrub_cover[i],
                self.cells.shrub_height[i],
            )

        # 2. Fuels: Calculate 1-hr fuel loading and fuel size classes
        for i in range(n):
            self.cells.fuel_1hr[i] = calc_total_1hr_fuel(
                wb=self.cells.shrub_cover[i] * 0.2,
                fl=self.cells.shrub_cover[i] * 0.1,
                herb=self.cells.herb_biomass[i] * 0.8,
            )
            # Allocation
            p1, p10, p100, p1000, tot = calc_fuel_percentages(
                self.cells.fuel_1hr[i],
                self.cells.fuel_10hr[i],
                self.cells.fuel_100hr[i],
                self.cells.fuel_1000hr[i],
            )

        # 3. Succession: Annual height increment
        for i in range(n):
            self.cells.herb_height[i] = calc_growth_rate(self.cells.herb_height[i], 2.0)

        # 4. Disturbance
        if has_graze:
            for i in range(n):
                self.cells.herb_biomass[i] = calc_graze_biomass_reduction(
                    self.cells.herb_biomass[i], 30.0
                )


class TestGridSimulationDriver(unittest.TestCase):
    def test_simulation_driver_annual_cycle(self):
        """Single-year execution updates biomass, fuels, and succession height."""
        cells = LandscapeCellBuffers(2)
        cells.herb_cover = [30.0, 40.0]
        cells.herb_height = [25.0, 30.0]
        cells.herb_biomass = [400.0, 500.0]
        cells.shrub_cover = [15.0, 20.0]
        cells.shrub_height = [50.0, 60.0]

        driver = InMemorySimulationDriver(cells)
        driver.run_year(year=0)

        self.assertEqual(cells.herb_height[0], 27.0)  # 25 + 2
        self.assertEqual(cells.herb_height[1], 32.0)  # 30 + 2
        self.assertGreater(cells.fuel_1hr[0], 0.0)

    def test_multi_year_simulation_stepping(self):
        """5-year simulation run tracks cumulative succession height increments."""
        cells = LandscapeCellBuffers(1)
        cells.herb_height[0] = 10.0
        cells.herb_cover[0] = 20.0
        cells.herb_biomass[0] = 300.0
        cells.shrub_cover[0] = 10.0
        cells.shrub_height[0] = 40.0

        driver = InMemorySimulationDriver(cells)
        for y in range(5):
            driver.run_year(y)

        # 10 + 5 * 2 = 20.0
        self.assertEqual(cells.herb_height[0], 20.0)

    def test_disturbance_year_application(self):
        """Disturbance applied in year 3 reduces biomass appropriately."""
        cells = LandscapeCellBuffers(1)
        cells.herb_height[0] = 15.0
        cells.herb_cover[0] = 20.0
        cells.herb_biomass[0] = 400.0
        cells.shrub_cover[0] = 10.0
        cells.shrub_height[0] = 30.0

        driver = InMemorySimulationDriver(cells)
        # Year 0..2 no graze
        for y in range(3):
            driver.run_year(y, has_graze=False)

        pre_graze_b = cells.herb_biomass[0]
        # Year 3 with graze
        driver.run_year(3, has_graze=True)
        self.assertLess(cells.herb_biomass[0], pre_graze_b)

    def test_driver_zero_cells_handling(self):
        """Driver runs gracefully on 0 cells without exceptions."""
        cells = LandscapeCellBuffers(0)
        driver = InMemorySimulationDriver(cells)
        driver.run_year(0)
        self.assertEqual(cells.n, 0)

    def test_driver_reproducibility(self):
        """Two independent drivers with identical inputs produce bit-identical results."""
        cells1 = LandscapeCellBuffers(3)
        cells2 = LandscapeCellBuffers(3)
        for i in range(3):
            cells1.herb_height[i] = cells2.herb_height[i] = 20.0 + i
            cells1.herb_cover[i] = cells2.herb_cover[i] = 30.0 + i
            cells1.herb_biomass[i] = cells2.herb_biomass[i] = 300.0 + i * 50
            cells1.shrub_cover[i] = cells2.shrub_cover[i] = 10.0 + i
            cells1.shrub_height[i] = cells2.shrub_height[i] = 40.0 + i

        d1 = InMemorySimulationDriver(cells1)
        d2 = InMemorySimulationDriver(cells2)

        for y in range(3):
            d1.run_year(y)
            d2.run_year(y)

        self.assertEqual(cells1.herb_height, cells2.herb_height)
        self.assertEqual(cells1.herb_biomass, cells2.herb_biomass)
        self.assertEqual(cells1.fuel_1hr, cells2.fuel_1hr)


if __name__ == "__main__":
    unittest.main()
