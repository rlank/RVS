"""
Tier 1 - Feature 5: Succession Routines S01–S06.
Verifies pure succession routines: calc_growth_rate, evaluate_herb_growth,
calc_s2b_quadratic_form, calc_herb_biomass_production, scale_shrub_height,
and scale_shrub_cover.
"""

import math
import unittest

from tests.rvs_math_oracle import (
    calc_growth_rate,
    calc_herb_biomass_production,
    calc_s2b_quadratic_form,
    evaluate_herb_growth,
    scale_shrub_cover,
    scale_shrub_height,
)


class TestSuccessionRoutines(unittest.TestCase):
    def test_annual_growth_rate_increment(self):
        """S01: Annual linear growth increment, non-negative constraint."""
        val1 = calc_growth_rate(15.0, 2.5)
        self.assertEqual(val1, 17.5)

        val2 = calc_growth_rate(5.0, -10.0)
        self.assertEqual(val2, 0.0)

    def test_climate_driven_herb_growth(self):
        """S02: Precipitation and NDVI log-linear interaction."""
        b0, b1, b2, b3 = 1.0, 0.4, 0.6, 0.1
        ppt = 450.0
        ndvi = 0.55
        expected = math.exp(
            b0 + b1 * math.log(ppt) + b2 * math.log(ndvi) + b3 * math.log(ppt) * math.log(ndvi)
        )
        actual = evaluate_herb_growth(b0, b1, b2, b3, ppt, ndvi)
        self.assertAlmostEqual(actual, expected, places=5)
        self.assertGreater(actual, 0.0)

    def test_variance_quadratic_form(self):
        """S03: Matrix-vector quadratic form x^T * Sigma * x."""
        x = [1.0, 2.0]
        # 2x2 matrix: [[2.0, 0.5], [0.5, 3.0]]
        # x^T Sigma x = 1*(2*1 + 0.5*2) + 2*(0.5*1 + 3*2) = 1*(3.0) + 2*(6.5) = 3 + 13 = 16.0
        sigma = [2.0, 0.5, 0.5, 3.0]
        actual = calc_s2b_quadratic_form(x, sigma, 2)
        self.assertAlmostEqual(actual, 16.0, places=6)

    def test_stochastic_herb_biomass_draw(self):
        """S04: Stochastic herb biomass draw bounded at zero."""
        mean_prod = 500.0
        s2b = 2500.0  # std_dev = 50.0

        # Positive draw: z = 1.96
        draw_high = calc_herb_biomass_production(mean_prod, s2b, 1.96)
        self.assertAlmostEqual(draw_high, 500.0 + 1.96 * 50.0, places=4)

        # Extreme negative draw bounded at 0.0
        draw_neg = calc_herb_biomass_production(100.0, 10000.0, -2.5)
        self.assertEqual(draw_neg, 0.0)

    def test_shrub_height_and_cover_transitions(self):
        """S05 & S06: Shrub height growth and canopy cover upper bound at 100%."""
        ht = scale_shrub_height(45.0, 3.2)
        self.assertEqual(ht, 48.2)

        # Cover scaling: 85% + 20% clamped to 100%
        cov = scale_shrub_cover(85.0, 20.0)
        self.assertEqual(cov, 100.0)

        # Negative cover change clamped at 0%
        cov_neg = scale_shrub_cover(10.0, -15.0)
        self.assertEqual(cov_neg, 0.0)


if __name__ == "__main__":
    unittest.main()
