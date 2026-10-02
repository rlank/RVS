"""
Tier 3 - Combination 1: Biomass & Fuels Coupling.
Verifies the end-to-end pipeline coupling allometric shrub biomass to fine and coarse
fuel bed components (1-hr, 10-hr, 100-hr) and fuelbed size class percentages.
"""

import unittest

from tests.rvs_math_oracle import (
    calc_attenuation,
    calc_fuel_component_fl,
    calc_fuel_component_wb,
    calc_fuel_percentages,
    calc_herb_loading,
    calc_total_1hr_fuel,
    eq_165,
)


class TestBiomassFuelsCoupling(unittest.TestCase):
    def test_shrub_biomass_to_fine_fuels_coupling(self):
        """Allometric shrub biomass feeds directly into wood/bark and foliage 1-hr fuels."""
        # 1. Calculate shrub biomass from cover
        shrub_cov = 35.0
        shrub_b = eq_165(12.0, 3.0, shrub_cov)  # 12 + 3 * 35 = 117.0

        # 2. Derive fuel components
        wb_prop = 0.30
        fl_prop = 0.20
        wb_fuel = calc_fuel_component_wb(shrub_b, wb_prop)  # 117 * 0.30 * 0.5 = 17.55
        fl_fuel = calc_fuel_component_fl(shrub_b, fl_prop)  # 117 * 0.20 * 0.5 = 11.70

        # 3. Add cured herbaceous fuel
        herb_fuel = calc_herb_loading(herb_biomass=200.0, cure_rate=0.75)  # 150.0

        # 4. Total 1-hr fuel
        tot_1hr = calc_total_1hr_fuel(wb_fuel, fl_fuel, herb_fuel)
        self.assertAlmostEqual(tot_1hr, 17.55 + 11.70 + 150.0, places=4)

    def test_canopy_attenuation_affects_herb_loading(self):
        """Shrub canopy competition attenuates herb biomass, reducing herbaceous fuel loading."""
        raw_herb_b = 600.0
        # High shrub canopy (sc=60, sh=80) vs herb (hc=20, hh=30)
        # hi = 600, si = 4800 -> ratio = 600/4800 = 0.125
        attenuated_b = calc_attenuation(raw_herb_b, hc=20.0, hh=30.0, sc=60.0, sh=80.0)
        self.assertEqual(attenuated_b, 75.0)

        # Cured loading with attenuated herb biomass
        cured_load = calc_herb_loading(attenuated_b, cure_rate=0.8)
        self.assertEqual(cured_load, 60.0)

    def test_fuel_size_classes_sum_consistency(self):
        """Calculated fuel percentages sum to 100% across fine and coarse fuels."""
        f1 = 80.0
        f10 = 120.0
        f100 = 200.0
        f1000 = 400.0
        p1, p10, p100, p1000, tot = calc_fuel_percentages(f1, f10, f100, f1000)
        self.assertEqual(tot, 800.0)
        self.assertAlmostEqual(p1 + p10 + p100 + p1000, 100.0, places=4)

    def test_multi_shrub_canopy_fuel_accumulation(self):
        """Multiple shrub species on a plot sum their fuel contributions."""
        shrubs = [
            {"cover": 20.0, "wb_p": 0.3, "fl_p": 0.2},
            {"cover": 15.0, "wb_p": 0.4, "fl_p": 0.1},
        ]
        total_shrub_1hr = 0.0
        for s in shrubs:
            b = eq_165(10.0, 2.0, s["cover"])
            wb = calc_fuel_component_wb(b, s["wb_p"])
            fl = calc_fuel_component_fl(b, s["fl_p"])
            total_shrub_1hr += wb + fl

        self.assertGreater(total_shrub_1hr, 0.0)

    def test_dense_versus_open_stand_fuel_loadings(self):
        """Dense stand produces significantly higher fuel loading than open stand."""
        dense_b = eq_165(10.0, 2.0, cover=80.0)
        open_b = eq_165(10.0, 2.0, cover=10.0)
        dense_wb = calc_fuel_component_wb(dense_b, 0.3)
        open_wb = calc_fuel_component_wb(open_b, 0.3)
        self.assertGreater(dense_wb, open_wb * 4.0)


if __name__ == "__main__":
    unittest.main()
