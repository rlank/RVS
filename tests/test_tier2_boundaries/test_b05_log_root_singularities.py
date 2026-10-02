"""
Tier 2 - Boundary 5: Logarithmic & Root Singularities.
Tests protection against domain errors in natural log, base-10 log, and square root functions.
"""

import unittest

from tests.rvs_math_oracle import (
    eq_202,
    eq_636_2,
    eq_998,
    eq_1000,
    eq_1008,
    eq_1058,
    eq_basic_bat,
    eq_pch,
    fuels_eq_2,
    fuels_eq_25,
)


class TestLogRootSingularities(unittest.TestCase):
    def test_natural_log_zero_and_negative(self):
        """Natural log routines return 0.0 for arguments <= 0."""
        self.assertEqual(eq_202(1.0, 1.0, 0.0), 0.0)
        self.assertEqual(eq_202(1.0, 1.0, -10.0), 0.0)
        self.assertEqual(eq_636_2(1.0, 1.0, 0.0), 0.0)
        self.assertEqual(fuels_eq_2(1.0, 1.0, 0.0), 0.0)

    def test_log10_zero_and_negative(self):
        """Log10 routines return 0.0 for dimensions <= 0."""
        self.assertEqual(eq_998(1.0, 1.0, 0.0), 0.0)
        self.assertEqual(eq_998(1.0, 1.0, -25.0), 0.0)
        self.assertEqual(eq_pch(1.0, 1.0, 0.0), 0.0)
        self.assertEqual(eq_pch(1.0, 1.0, -100.0), 0.0)

    def test_multivariate_partial_nonpositive(self):
        """Multivariate routines return 0.0 if any single dimension is non-positive."""
        # eq_1000: length=10, width=0, height=20
        self.assertEqual(eq_1000(1.0, 1.0, 1.0, 1.0, 10.0, 0.0, 20.0), 0.0)
        # eq_1008: natural log trivariate
        self.assertEqual(eq_1008(1.0, 1.0, 1.0, 1.0, 10.0, 20.0, -5.0), 0.0)
        # fuels_eq_25: log10 trivariate
        self.assertEqual(fuels_eq_25(1.0, 1.0, 1.0, 1.0, -1.0, 10.0, 10.0), 0.0)

    def test_square_root_negative_radicand(self):
        """Square root models clamp negative inputs without raising ValueError."""
        # eq_1058: cf1 + cf2*p1 + cf3*sqrt(p1) with p1 = -16
        res = eq_1058(10.0, 1.0, 2.0, -16.0)
        self.assertEqual(res, 10.0 - 16.0)  # cf3*0.0

    def test_basic_bat_summed_dimensions_zero(self):
        """eq_basic_bat with length + width <= 0 returns 0.0."""
        self.assertEqual(eq_basic_bat(1.0, 1.0, 0.0, 0.0), 0.0)
        self.assertEqual(eq_basic_bat(1.0, 1.0, -10.0, 5.0), 0.0)


if __name__ == "__main__":
    unittest.main()
