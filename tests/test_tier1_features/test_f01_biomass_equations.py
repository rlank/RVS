"""
Tier 1 - Feature 1: Biomass Equations B01–B18.
Verifies all 18 pure allometric shrub and tree biomass equations.
"""

import math
import unittest

from tests.rvs_math_oracle import (
    eq_165,
    eq_201,
    eq_202,
    eq_636,
    eq_636_2,
    eq_743,
    eq_998,
    eq_999,
    eq_1000,
    eq_1002,
    eq_1008,
    eq_1012,
    eq_1058,
    eq_1153,
    eq_1160,
    eq_1161,
    eq_basic_bat,
    eq_pch,
)


class TestBiomassEquations(unittest.TestCase):
    def test_linear_and_exponential_models(self):
        """B01, B02, B03: Cover-based linear, exponential, and power-law models."""
        # B01: eq_165: cf1 + cf2 * cover
        res165 = eq_165(10.0, 2.5, 30.0)
        self.assertAlmostEqual(res165, 10.0 + 2.5 * 30.0, places=6)
        self.assertEqual(res165, 85.0)

        # B02: eq_201: exp(cf1 + cf2 * cover)
        res201 = eq_201(1.2, 0.05, 20.0)
        expected201 = math.exp(1.2 + 0.05 * 20.0)
        self.assertAlmostEqual(res201, expected201, places=6)

        # B03: eq_202: exp(cf1 + cf2 * ln(cover))
        res202 = eq_202(0.8, 1.5, 10.0)
        expected202 = math.exp(0.8 + 1.5 * math.log(10.0))
        self.assertAlmostEqual(res202, expected202, places=6)

    def test_dimensional_volume_models(self):
        """B04, B05, B06, B10: Crown volume and interaction equations."""
        # B04 & B05: eq_636 and eq_636_2: exp(cf1 + cf2 * ln(vol / 1000))
        # length=50, width=40, height=30 -> vol = 60000 -> vol/1000 = 60.0
        res636 = eq_636(0.5, 1.2, 50.0, 40.0, 30.0)
        res636_2 = eq_636_2(0.5, 1.2, 60000.0)
        self.assertAlmostEqual(res636, res636_2, places=7)
        expected636 = math.exp(0.5 + 1.2 * math.log(60.0))
        self.assertAlmostEqual(res636, expected636, places=6)

        # B06: eq_743: cf1 + cf2 * cover * height
        res743 = eq_743(5.0, 0.02, 45.0, 120.0)
        self.assertAlmostEqual(res743, 5.0 + 0.02 * 45.0 * 120.0, places=6)

        # B10: eq_1002: cf1 + cf2 * vol
        res1002 = eq_1002(2.0, 0.005, 500.0)
        self.assertAlmostEqual(res1002, 2.0 + 0.005 * 500.0, places=6)

    def test_log10_power_law_models(self):
        """B07, B08, B09, B15, B18: Log10 multi-parameter allometric models."""
        # B07: eq_998: 10^(cf1 + cf2 * log10(width))
        res998 = eq_998(1.0, 1.5, 100.0)
        self.assertAlmostEqual(res998, 10.0 ** (1.0 + 1.5 * 2.0), places=6)

        # B08: eq_999: 10^(cf1 + cf2*log10(L) + cf3*log10(W))
        res999 = eq_999(0.5, 1.0, 1.2, 50.0, 30.0)
        expected999 = 10.0 ** (0.5 + 1.0 * math.log10(50.0) + 1.2 * math.log10(30.0))
        self.assertAlmostEqual(res999, expected999, places=6)

        # B09: eq_1000: 10^(cf1 + cf2*log10(L) + cf3*log10(W) + cf4*log10(H))
        res1000 = eq_1000(0.2, 0.8, 0.9, 1.1, 40.0, 35.0, 80.0)
        expected1000 = 10.0 ** (
            0.2 + 0.8 * math.log10(40.0) + 0.9 * math.log10(35.0) + 1.1 * math.log10(80.0)
        )
        self.assertAlmostEqual(res1000, expected1000, places=6)

        # B15: eq_1153: 10^(cf1 + cf2*log10(L*W) + cf3*log10(H))
        res1153 = eq_1153(0.1, 0.7, 0.8, 25.0, 20.0, 60.0)
        expected1153 = 10.0 ** (0.1 + 0.7 * math.log10(500.0) + 0.8 * math.log10(60.0))
        self.assertAlmostEqual(res1153, expected1153, places=6)

        # B18: eq_pch: 10^(cf1 + cf2 * log10(H))
        respch = eq_pch(1.2, 0.9, 150.0)
        expectedpch = 10.0 ** (1.2 + 0.9 * math.log10(150.0))
        self.assertAlmostEqual(respch, expectedpch, places=6)

    def test_multivariate_natural_log_models(self):
        """B11, B12, B14: Natural log exponential allometries."""
        # B11: eq_1008: exp(cf1 + cf2*ln(L) + cf3*ln(W) + cf4*ln(H))
        res1008 = eq_1008(0.4, 0.5, 0.6, 0.7, 45.0, 35.0, 90.0)
        expected1008 = math.exp(
            0.4 + 0.5 * math.log(45.0) + 0.6 * math.log(35.0) + 0.7 * math.log(90.0)
        )
        self.assertAlmostEqual(res1008, expected1008, places=6)

        # B12: eq_1012: exp(cf1 + cf2*ln(p1) + cf3*ln(p2))
        res1012 = eq_1012(0.3, 1.1, 0.9, 25.0, 50.0)
        expected1012 = math.exp(0.3 + 1.1 * math.log(25.0) + 0.9 * math.log(50.0))
        self.assertAlmostEqual(res1012, expected1012, places=6)

        # B14: eq_basic_bat: exp(cf1 + cf2*ln(L + W))
        res_bat = eq_basic_bat(0.5, 1.3, 40.0, 60.0)
        expected_bat = math.exp(0.5 + 1.3 * math.log(100.0))
        self.assertAlmostEqual(res_bat, expected_bat, places=6)

    def test_polynomial_and_ratio_models(self):
        """B13, B16, B17: Polynomial, square root, and ratio models."""
        # B13: eq_1058: cf1 + cf2*p1 + cf3*sqrt(p1)
        res1058 = eq_1058(2.0, 1.5, 3.0, 16.0)
        self.assertAlmostEqual(res1058, 2.0 + 1.5 * 16.0 + 3.0 * 4.0, places=6)
        self.assertEqual(res1058, 38.0)

        # B16: eq_1160: cf1 + cf2 * p1
        res1160 = eq_1160(3.5, 0.75, 40.0)
        self.assertAlmostEqual(res1160, 3.5 + 0.75 * 40.0, places=6)
        self.assertEqual(res1160, 33.5)

        # B17: eq_1161: cf1 * (p1 / cf2)
        res1161 = eq_1161(12.0, 4.0, 20.0)
        self.assertAlmostEqual(res1161, 12.0 * (20.0 / 4.0), places=6)
        self.assertEqual(res1161, 60.0)

    def test_biomass_monotonicity_under_cover(self):
        """Validates that biomass increases monotonically with increasing cover."""
        prev_b = -1.0
        for cov in [5.0, 15.0, 30.0, 50.0, 75.0, 95.0]:
            b = eq_165(5.0, 2.0, cov)
            self.assertGreater(b, prev_b)
            prev_b = b


if __name__ == "__main__":
    unittest.main()
