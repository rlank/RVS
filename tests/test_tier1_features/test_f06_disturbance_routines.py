"""
Tier 1 - Feature 6: Disturbance Routines D01–D06.
Verifies pure disturbance routines: calc_graze_biomass_reduction, calc_graze_height_reduction,
calc_mow_biomass_residual, calc_fire_biomass_consumption, calc_fire_shrub_mortality,
and calc_fire_herb_post_regrowth.
"""

import math
import unittest

from tests.rvs_math_oracle import (
    calc_fire_biomass_consumption,
    calc_fire_herb_post_regrowth,
    calc_fire_shrub_mortality,
    calc_graze_biomass_reduction,
    calc_graze_height_reduction,
    calc_mow_biomass_residual,
)


class TestDisturbanceRoutines(unittest.TestCase):
    def test_grazing_biomass_reduction(self):
        """D01: Biomass reduction from grazing utilization percentage."""
        pre_biomass = 600.0
        util_pct = 40.0  # 40% grazed
        post = calc_graze_biomass_reduction(pre_biomass, util_pct)
        # 600 * (1.0 - 0.40) = 360.0
        self.assertEqual(post, 360.0)

        # 100% utilization leaves 0 biomass
        post_full = calc_graze_biomass_reduction(pre_biomass, 100.0)
        self.assertEqual(post_full, 0.0)

    def test_grazing_height_reduction(self):
        """D02: Non-linear stubble defoliation: pre_height * (1 - u)^0.7."""
        pre_ht = 50.0
        util_pct = 50.0  # u = 0.5
        expected = 50.0 * (0.5**0.7)
        actual = calc_graze_height_reduction(pre_ht, util_pct)
        self.assertAlmostEqual(actual, expected, places=5)

    def test_mowing_residual_biomass(self):
        """D03: Mechanical mower residual biomass proportional to cut height."""
        pre_biomass = 800.0
        pre_ht = 40.0
        mow_ht = 10.0  # cuts to 25% height
        residual = calc_mow_biomass_residual(pre_biomass, pre_ht, mow_ht)
        self.assertEqual(residual, 200.0)

        # If mower height exceeds pre_height, no biomass is removed
        no_cut = calc_mow_biomass_residual(pre_biomass, pre_ht, 50.0)
        self.assertEqual(no_cut, 800.0)

    def test_fire_biomass_consumption(self):
        """D04: Heat-driven wildfire biomass consumption."""
        biomass = 1000.0
        severity_k = 0.05
        intensity = 20.0
        # consumed_fraction = 1.0 - exp(-0.05 * 20) = 1.0 - exp(-1.0) ~ 0.63212
        expected_consumed = 1000.0 * (1.0 - math.exp(-1.0))
        actual = calc_fire_biomass_consumption(biomass, severity_k, intensity)
        self.assertAlmostEqual(actual, expected_consumed, places=4)

    def test_fire_shrub_mortality_and_resprout(self):
        """D05 & D06: Scorch-dependent logistic shrub mortality and post-fire resprout."""
        # D05: Scorch 80%, alpha=-1.5, beta=0.04 -> z = -1.5 + 3.2 = 1.7 -> mortality = 1/(1+exp(-1.7)) ~ 0.8455
        mortality = calc_fire_shrub_mortality(80.0, -1.5, 0.04)
        expected_mort = 1.0 / (1.0 + math.exp(-1.7))
        self.assertAlmostEqual(mortality, expected_mort, places=4)

        # D06: Post-fire resprout with 60% mortality
        resprout_pot = 400.0
        regrowth = calc_fire_herb_post_regrowth(resprout_pot, 60.0)
        # 400 * (1.0 - 0.60) = 160.0
        self.assertEqual(regrowth, 160.0)


if __name__ == "__main__":
    unittest.main()
