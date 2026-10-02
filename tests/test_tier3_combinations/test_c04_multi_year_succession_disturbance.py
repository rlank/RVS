"""
Tier 3 - Combination 4: Multi-Year Succession & Disturbance Interaction.
Simulates a multi-year ecological timeline tracking growth trajectories punctuated
by scheduled disturbances (grazing cycles and year-7 wildfire events).
"""

import unittest

from tests.rvs_math_oracle import (
    calc_fire_biomass_consumption,
    calc_fire_herb_post_regrowth,
    calc_graze_biomass_reduction,
    calc_graze_height_reduction,
    calc_growth_rate,
    scale_shrub_cover,
)


class TestMultiYearSuccessionDisturbance(unittest.TestCase):
    def test_succession_growth_interrupted_by_wildfire(self):
        """Simulates 10 years: growth in years 0–6, wildfire at year 7, recovery in 8–9."""
        herb_b = 300.0
        herb_ht = 20.0
        history = []

        for year in range(10):
            # Annual growth
            herb_ht = calc_growth_rate(herb_ht, 2.0)
            herb_b += 50.0

            # Year 7 fire disturbance
            if year == 7:
                consumed = calc_fire_biomass_consumption(
                    herb_b, severity_k=0.05, fire_intensity=30.0
                )
                herb_b = calc_fire_herb_post_regrowth(resprout_pot=250.0, mortality_pct=40.0)
                herb_ht = 5.0  # Top-kill reset

            history.append((year, herb_b, herb_ht))

        # Check year 7 drop
        b_yr6 = history[6][1]
        b_yr7 = history[7][1]
        self.assertLess(b_yr7, b_yr6)

        # Check year 9 recovery
        b_yr9 = history[9][1]
        self.assertGreater(b_yr9, b_yr7)

    def test_cyclical_grazing_regime(self):
        """Cyclical grazing (every 3 years) stabilizes herb height below ungrazed ceiling."""
        ht = 15.0
        for year in range(12):
            ht = calc_growth_rate(ht, 3.0)
            if year % 3 == 0 and year > 0:
                ht = calc_graze_height_reduction(ht, utilization_pct=50.0)

        # Grazed height should stay moderate
        self.assertLess(ht, 40.0)

    def test_multi_decadal_trajectory_stability(self):
        """20-year simulation trajectory remains strictly non-negative and finite."""
        b = 200.0
        ht = 15.0
        for year in range(20):
            ht = calc_growth_rate(ht, 1.5)
            b += 30.0
            if year == 7:
                b = calc_graze_biomass_reduction(b, 60.0)
            self.assertGreaterEqual(b, 0.0)
            self.assertGreaterEqual(ht, 0.0)
            self.assertLess(b, 10000.0)

    def test_shrub_cover_recovery_dynamics(self):
        """Shrub cover grows, is burned, and then resumes slow incremental recovery."""
        cov = 20.0
        # Years 0 to 4 growth
        for _ in range(5):
            cov = scale_shrub_cover(cov, 2.0)
        self.assertEqual(cov, 30.0)

        # Burn reduces cover
        cov = scale_shrub_cover(cov, -25.0)
        self.assertEqual(cov, 5.0)

        # Post-burn recovery
        cov = scale_shrub_cover(cov, 2.0)
        self.assertEqual(cov, 7.0)

    def test_biomass_floor_continuity(self):
        """Even with consecutive extreme disturbances, biomass cannot drop below 0.0."""
        b = 100.0
        b = calc_graze_biomass_reduction(b, 100.0)
        self.assertEqual(b, 0.0)
        b = calc_graze_biomass_reduction(b, 50.0)
        self.assertEqual(b, 0.0)


if __name__ == "__main__":
    unittest.main()
