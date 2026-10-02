"""
Tier 1 - Feature 3: Fuels Equations F01–F07.
Verifies all 7 pure fuel loading allometric equations.
"""

import math
import unittest

from tests.rvs_math_oracle import (
    fuels_eq_1,
    fuels_eq_2,
    fuels_eq_13,
    fuels_eq_15,
    fuels_eq_17,
    fuels_eq_25,
    fuels_eq_33,
)


class TestFuelsEquations(unittest.TestCase):
    def test_linear_and_power_fuels(self):
        """F01 & F02: Linear and power-law fuel equations."""
        # F01: fuels_eq_1: cf1 + cf2 * p1
        f1 = fuels_eq_1(1.5, 0.45, 120.0)
        self.assertAlmostEqual(f1, 1.5 + 0.45 * 120.0, places=6)
        self.assertEqual(f1, 55.5)

        # F02: fuels_eq_2: exp(cf1 + cf2 * ln(p1))
        f2 = fuels_eq_2(0.5, 1.1, 40.0)
        expected2 = math.exp(0.5 + 1.1 * math.log(40.0))
        self.assertAlmostEqual(f2, expected2, places=6)

    def test_exponential_and_bilinear_fuels(self):
        """F03 & F04: Exponential and bilinear interaction fuel equations."""
        # F03: fuels_eq_13: exp(cf1 + cf2 * p1)
        f13 = fuels_eq_13(0.2, 0.03, 50.0)
        expected13 = math.exp(0.2 + 0.03 * 50.0)
        self.assertAlmostEqual(f13, expected13, places=6)

        # F04: fuels_eq_15: cf1 + cf2 * p1 * p2
        f15 = fuels_eq_15(2.0, 0.05, 30.0, 40.0)
        expected15 = 2.0 + 0.05 * 30.0 * 40.0
        self.assertAlmostEqual(f15, expected15, places=6)
        self.assertEqual(f15, 62.0)

    def test_bivariate_and_trivariate_fuels(self):
        """F05 & F06: Log-linear and trivariate log10 fuel equations."""
        # F05: fuels_eq_17: exp(cf1 + cf2*ln(p1) + cf3*ln(p2))
        f17 = fuels_eq_17(0.1, 0.8, 0.9, 25.0, 35.0)
        expected17 = math.exp(0.1 + 0.8 * math.log(25.0) + 0.9 * math.log(35.0))
        self.assertAlmostEqual(f17, expected17, places=6)

        # F06: fuels_eq_25: 10^(cf1 + cf2*log10(p1) + cf3*log10(p2) + cf4*log10(p3))
        f25 = fuels_eq_25(0.2, 0.5, 0.6, 0.7, 10.0, 20.0, 30.0)
        expected25 = 10.0 ** (
            0.2 + 0.5 * math.log10(10.0) + 0.6 * math.log10(20.0) + 0.7 * math.log10(30.0)
        )
        self.assertAlmostEqual(f25, expected25, places=6)

    def test_quadratic_fuel_curve(self):
        """F07: Quadratic fuel curve: cf1 + p1 * (cf2 + cf3 * p1)."""
        f33 = fuels_eq_33(1.0, 2.0, 0.5, 10.0)
        # 1.0 + 10.0 * (2.0 + 0.5 * 10.0) = 1.0 + 10.0 * 7.0 = 71.0
        self.assertAlmostEqual(f33, 71.0, places=6)

    def test_fuels_equations_monotonicity(self):
        """Verifies monotonicity: higher inputs produce higher fuel loading."""
        prev = -1.0
        for p in [10.0, 25.0, 50.0, 100.0, 150.0]:
            val = fuels_eq_33(1.0, 1.5, 0.05, p)
            self.assertGreater(val, prev)
            prev = val


if __name__ == "__main__":
    unittest.main()
