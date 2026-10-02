"""
Tier 1 - Feature 4: Fuels Driver Routines F08–F12.
Verifies pure fuels driver routines: calc_fuel_component_wb, calc_fuel_component_fl,
calc_total_1hr_fuel, calc_fuel_percentages, and calc_herb_loading.
"""

import unittest

from tests.rvs_math_oracle import (
    calc_fuel_component_fl,
    calc_fuel_component_wb,
    calc_fuel_percentages,
    calc_herb_loading,
    calc_total_1hr_fuel,
)


class TestFuelsDrivers(unittest.TestCase):
    def test_wood_bark_and_foliage_components(self):
        """F08 & F09: Shrub biomass proportion splits into 1-hr wood/bark and foliage."""
        shrub_biomass = 240.0
        wb_prop = 0.35
        fl_prop = 0.25

        wb = calc_fuel_component_wb(shrub_biomass, wb_prop)
        fl = calc_fuel_component_fl(shrub_biomass, fl_prop)

        # 240.0 * 0.35 * 0.5 = 42.0
        self.assertEqual(wb, 42.0)
        # 240.0 * 0.25 * 0.5 = 30.0
        self.assertEqual(fl, 30.0)

    def test_total_1hr_fuel_aggregation(self):
        """F10: Aggregates wood/bark, foliage, and herbaceous 1-hr fuels."""
        wb = 42.0
        fl = 30.0
        herb = 55.0
        total_1hr = calc_total_1hr_fuel(wb, fl, herb)
        self.assertEqual(total_1hr, 127.0)

    def test_fuel_percentages_calculation(self):
        """F11: Computes percentage contributions across fuelbed size classes."""
        f1, f10, f100, f1000 = 50.0, 100.0, 150.0, 200.0
        # total = 500.0
        # p1 = 10%, p10 = 20%, p100 = 30%, p1000 = 40%
        p1, p10, p100, p1000, tot = calc_fuel_percentages(f1, f10, f100, f1000)
        self.assertEqual(tot, 500.0)
        self.assertAlmostEqual(p1, 10.0, places=4)
        self.assertAlmostEqual(p10, 20.0, places=4)
        self.assertAlmostEqual(p100, 30.0, places=4)
        self.assertAlmostEqual(p1000, 40.0, places=4)
        self.assertAlmostEqual(p1 + p10 + p100 + p1000, 100.0, places=4)

    def test_fuel_percentages_zero_total(self):
        """F11: Division-by-zero guard returns all zeros when total fuel loading is zero."""
        p1, p10, p100, p1000, tot = calc_fuel_percentages(0.0, 0.0, 0.0, 0.0)
        self.assertEqual((p1, p10, p100, p1000, tot), (0.0, 0.0, 0.0, 0.0, 0.0))

    def test_herb_loading_curing(self):
        """F12: Herbaceous loading scales with curing rate."""
        herb_biomass = 350.0
        cure_rate = 0.80  # 80% cured
        loading = calc_herb_loading(herb_biomass, cure_rate)
        self.assertEqual(loading, 280.0)


if __name__ == "__main__":
    unittest.main()
