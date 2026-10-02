"""
Tier 2 - Boundary 4: Division by Zero Guards.
Tests explicit protection against zero denominators across all mathematical routines.
"""

import unittest

from tests.rvs_math_oracle import (
    calc_attenuation,
    calc_fuel_percentages,
    calc_mow_biomass_residual,
    calc_stems_per_acre_pure,
    eq_1161,
)


class TestDivisionByZero(unittest.TestCase):
    def test_fuel_percentages_zero_total_loading(self):
        """Zero total fuel across all size classes returns zeros without division by zero."""
        p1, p10, p100, p1000, tot = calc_fuel_percentages(0.0, 0.0, 0.0, 0.0)
        self.assertEqual(tot, 0.0)
        self.assertEqual(p1, 0.0)
        self.assertEqual(p10, 0.0)
        self.assertEqual(p100, 0.0)
        self.assertEqual(p1000, 0.0)

    def test_eq_1161_zero_cf2_denominator(self):
        """eq_1161: cf1 * (p1 / cf2) where cf2 == 0.0 must return 0.0."""
        self.assertEqual(eq_1161(25.0, 0.0, 100.0), 0.0)

    def test_calc_attenuation_zero_canopy_indexes(self):
        """Attenuated biomass returns raw herb biomass when shrub or herb index is 0.0."""
        # shrub cover/height = 0
        self.assertEqual(calc_attenuation(450.0, 30.0, 40.0, 0.0, 0.0), 450.0)
        # herb cover/height = 0
        self.assertEqual(calc_attenuation(450.0, 0.0, 0.0, 30.0, 40.0), 450.0)

    def test_calc_mow_biomass_zero_pre_height(self):
        """calc_mow_biomass_residual returns 0.0 when pre_height == 0.0."""
        self.assertEqual(calc_mow_biomass_residual(500.0, 0.0, 15.0), 0.0)

    def test_calc_stems_per_acre_zero_stem_area(self):
        """calc_stems_per_acre_pure returns 0.0 when single_stem_area == 0.0."""
        self.assertEqual(calc_stems_per_acre_pure(0.0, 20.0, 40.0), 0.0)


if __name__ == "__main__":
    unittest.main()
