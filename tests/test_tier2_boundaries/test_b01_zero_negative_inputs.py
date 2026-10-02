"""
Tier 2 - Boundary 1: Zero & Negative Inputs.
Tests responses when physical measurements (cover, height, biomass, stem area) are zero or negative.
"""

import unittest

from tests.rvs_math_oracle import (
    calc_fuel_component_wb,
    calc_herb_loading,
    calc_stems_per_acre_pure,
    eq_165,
    eq_743,
    evaluate_herb_growth,
)


class TestZeroNegativeInputs(unittest.TestCase):
    def test_negative_cover_inputs(self):
        """Negative foliar cover is clamped to 0.0, returning intercept cf1."""
        self.assertEqual(eq_165(15.0, 2.5, -10.0), 15.0)
        self.assertEqual(eq_165(15.0, 2.5, 0.0), 15.0)

    def test_negative_height_inputs(self):
        """Negative height in interaction models is clamped to 0.0."""
        self.assertEqual(eq_743(10.0, 0.5, 20.0, -50.0), 10.0)
        self.assertEqual(eq_743(10.0, 0.5, -20.0, 50.0), 10.0)

    def test_negative_biomass_inputs(self):
        """Negative biomass input produces 0.0 fuel component."""
        self.assertEqual(calc_fuel_component_wb(-500.0, 0.4), 0.0)
        self.assertEqual(calc_herb_loading(-250.0, 0.8), 0.0)

    def test_zero_stem_area(self):
        """Zero single stem area returns 0.0 stems per acre without crashing."""
        self.assertEqual(calc_stems_per_acre_pure(0.0, 30.0, 50.0), 0.0)
        self.assertEqual(calc_stems_per_acre_pure(-100.0, 30.0, 50.0), 0.0)

    def test_negative_precipitation_and_ndvi(self):
        """Negative precipitation or NDVI returns 0.0 herb production."""
        self.assertEqual(evaluate_herb_growth(1.0, 0.5, 0.5, 0.1, -100.0, 0.5), 0.0)
        self.assertEqual(evaluate_herb_growth(1.0, 0.5, 0.5, 0.1, 500.0, -0.2), 0.0)
        self.assertEqual(evaluate_herb_growth(1.0, 0.5, 0.5, 0.1, 0.0, 0.5), 0.0)


if __name__ == "__main__":
    unittest.main()
