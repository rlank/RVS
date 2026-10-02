"""
Tier 2 - Boundary 6: Clamping Boundaries.
Tests hard physical clamps at 0% and 100% across canopy cover, disturbance utilization,
fuel proportions, and mortality rates.
"""

import unittest

from tests.rvs_math_oracle import (
    calc_fire_herb_post_regrowth,
    calc_fuel_component_wb,
    calc_graze_biomass_reduction,
    calc_herb_loading,
    scale_shrub_cover,
)


class TestClampingBoundaries(unittest.TestCase):
    def test_canopy_cover_clamped_zero_to_hundred(self):
        """Foliar canopy cover is strictly clamped within [0.0, 100.0]."""
        # Upper clamp
        self.assertEqual(scale_shrub_cover(90.0, 25.0), 100.0)
        self.assertEqual(scale_shrub_cover(110.0, 0.0), 100.0)
        # Lower clamp
        self.assertEqual(scale_shrub_cover(10.0, -30.0), 0.0)
        self.assertEqual(scale_shrub_cover(-20.0, 0.0), 0.0)

    def test_grazing_utilization_clamped(self):
        """Grazing utilization exceeding 100% clamps to 100%, leaving 0 residual biomass."""
        post = calc_graze_biomass_reduction(pre_biomass=500.0, utilization_pct=150.0)
        self.assertEqual(post, 0.0)

        # Negative utilization clamps to 0%, removing no biomass
        post_neg = calc_graze_biomass_reduction(pre_biomass=500.0, utilization_pct=-20.0)
        self.assertEqual(post_neg, 500.0)

    def test_fuel_component_proportion_clamped(self):
        """Fuel component proportions are clamped within [0.0, 1.0]."""
        wb_high = calc_fuel_component_wb(shrub_biomass=200.0, proportion=1.5)
        # 200 * 1.0 * 0.5 = 100.0
        self.assertEqual(wb_high, 100.0)

        wb_neg = calc_fuel_component_wb(shrub_biomass=200.0, proportion=-0.5)
        self.assertEqual(wb_neg, 0.0)

    def test_herb_curing_rate_clamped(self):
        """Herbaceous curing rate is clamped within [0.0, 1.0]."""
        # Exceeding 1.0 clamps to 1.0
        load_high = calc_herb_loading(300.0, 1.3)
        self.assertEqual(load_high, 300.0)

        # Negative rate clamps to 0.0
        load_neg = calc_herb_loading(300.0, -0.4)
        self.assertEqual(load_neg, 0.0)

    def test_fire_mortality_percentage_clamped(self):
        """Post-fire regrowth clamps mortality at 100%."""
        regrowth = calc_fire_herb_post_regrowth(resprout_pot=250.0, mortality_pct=120.0)
        self.assertEqual(regrowth, 0.0)


if __name__ == "__main__":
    unittest.main()
