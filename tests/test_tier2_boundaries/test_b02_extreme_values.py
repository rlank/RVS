"""
Tier 2 - Boundary 2: Extreme Values.
Tests behavior under astronomical or extreme ecological conditions.
"""

import unittest

from tests.rvs_math_oracle import (
    calc_fire_biomass_consumption,
    calc_fire_shrub_mortality,
    calc_fuel_percentages,
    calc_stems_per_acre_pure,
)


class TestExtremeValues(unittest.TestCase):
    def test_extreme_fire_intensity(self):
        """Ultra-high fire intensity consumes 100% of available biomass without exceeding pre-biomass."""
        pre_b = 5000.0
        consumed = calc_fire_biomass_consumption(pre_b, severity_k=0.1, fire_intensity=10000.0)
        self.assertAlmostEqual(consumed, pre_b, places=4)
        self.assertLessEqual(consumed, pre_b)

    def test_extreme_shrub_height(self):
        """Extreme shrub height (e.g. 2000 cm) does not crash or raise OverflowError."""
        stems = calc_stems_per_acre_pure(5000.0, 30.0, 2000.0)
        self.assertGreater(stems, 0.0)

    def test_extreme_biomass_fuel_percentages(self):
        """Massive fuel loads (1e9 g/m^2) calculate percentages without numerical overflow."""
        f1, f10, f100, f1000 = 1e8, 2e8, 3e8, 4e8
        p1, p10, p100, p1000, tot = calc_fuel_percentages(f1, f10, f100, f1000)
        self.assertAlmostEqual(tot, 1e9, places=2)
        self.assertAlmostEqual(p1, 10.0, places=4)
        self.assertAlmostEqual(p1000, 40.0, places=4)

    def test_near_zero_epsilon_values(self):
        """Microscopic non-zero values (1e-12) calculate smoothly without discontinuities."""
        tiny = 1e-12
        consumed = calc_fire_biomass_consumption(100.0, 0.05, tiny)
        self.assertGreater(consumed, 0.0)
        self.assertLess(consumed, 1.0)

    def test_extreme_scorch_percentage(self):
        """Extreme scorch percentage (e.g. 500%) clamps to 100%, producing bounded logistic mortality."""
        mort = calc_fire_shrub_mortality(500.0, alpha=-1.0, beta=0.05)
        self.assertLessEqual(mort, 1.0)
        self.assertGreater(mort, 0.0)


if __name__ == "__main__":
    unittest.main()
