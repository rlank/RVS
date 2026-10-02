"""
Tier 1 - Feature 7: Non-Positive Domain Guards.
Verifies strict guards returning 0.0 or safe clamped values for ln/log10/sqrt
with non-positive inputs and division-by-zero guards.
"""

import unittest

from tests.rvs_math_oracle import (
    calc_attenuation,
    calc_crown_width,
    calc_fuel_percentages,
    calc_stems_per_acre_pure,
    eq_202,
    eq_636_2,
    eq_998,
    eq_999,
    eq_1000,
    eq_1058,
    eq_1153,
    eq_1161,
    eq_pch,
    evaluate_herb_growth,
    fuels_eq_2,
    fuels_eq_17,
    fuels_eq_25,
)


class TestDomainGuards(unittest.TestCase):
    def test_log_singularity_guards(self):
        """Natural log functions must guard against zero and negative arguments."""
        # eq_202: ln(cover)
        self.assertEqual(eq_202(1.0, 1.0, 0.0), 0.0)
        self.assertEqual(eq_202(1.0, 1.0, -15.0), 0.0)

        # eq_636_2: ln(vol / 1000)
        self.assertEqual(eq_636_2(1.0, 1.0, 0.0), 0.0)
        self.assertEqual(eq_636_2(1.0, 1.0, -500.0), 0.0)

        # fuels_eq_2: ln(p1)
        self.assertEqual(fuels_eq_2(1.0, 1.0, 0.0), 0.0)
        self.assertEqual(fuels_eq_2(1.0, 1.0, -10.0), 0.0)

        # fuels_eq_17: ln(p1) and ln(p2)
        self.assertEqual(fuels_eq_17(1.0, 1.0, 1.0, 0.0, 10.0), 0.0)
        self.assertEqual(fuels_eq_17(1.0, 1.0, 1.0, 10.0, 0.0), 0.0)
        self.assertEqual(fuels_eq_17(1.0, 1.0, 1.0, -5.0, -5.0), 0.0)

        # evaluate_herb_growth: ln(ppt) and ln(ndvi)
        self.assertEqual(evaluate_herb_growth(1.0, 1.0, 1.0, 1.0, 0.0, 0.5), 0.0)
        self.assertEqual(evaluate_herb_growth(1.0, 1.0, 1.0, 1.0, 500.0, 0.0), 0.0)
        self.assertEqual(evaluate_herb_growth(1.0, 1.0, 1.0, 1.0, -100.0, 0.5), 0.0)

    def test_log10_singularity_guards(self):
        """Log10 functions must guard against zero and negative arguments."""
        # eq_998
        self.assertEqual(eq_998(1.0, 1.0, 0.0), 0.0)
        self.assertEqual(eq_998(1.0, 1.0, -20.0), 0.0)

        # eq_999
        self.assertEqual(eq_999(1.0, 1.0, 1.0, 0.0, 10.0), 0.0)
        self.assertEqual(eq_999(1.0, 1.0, 1.0, 10.0, 0.0), 0.0)

        # eq_1000
        self.assertEqual(eq_1000(1.0, 1.0, 1.0, 1.0, 0.0, 10.0, 10.0), 0.0)
        self.assertEqual(eq_1000(1.0, 1.0, 1.0, 1.0, 10.0, -5.0, 10.0), 0.0)

        # eq_1153
        self.assertEqual(eq_1153(1.0, 1.0, 1.0, 0.0, 10.0, 10.0), 0.0)
        self.assertEqual(eq_1153(1.0, 1.0, 1.0, 10.0, 10.0, 0.0), 0.0)

        # eq_pch
        self.assertEqual(eq_pch(1.0, 1.0, 0.0), 0.0)
        self.assertEqual(eq_pch(1.0, 1.0, -50.0), 0.0)

        # fuels_eq_25
        self.assertEqual(fuels_eq_25(1.0, 1.0, 1.0, 1.0, 0.0, 10.0, 10.0), 0.0)

    def test_sqrt_negative_guards(self):
        """Square root operations must guard against negative radicands."""
        # eq_1058: cf1 + cf2*p1 + cf3*sqrt(p1) -> sqrt(-p1) returns 0.0
        val = eq_1058(5.0, 2.0, 3.0, -9.0)
        self.assertEqual(val, 5.0 + 2.0 * (-9.0))  # cf3*0.0

        # calc_crown_width
        self.assertEqual(calc_crown_width(0.0), 0.0)
        self.assertEqual(calc_crown_width(-100.0), 0.0)

    def test_division_by_zero_guards(self):
        """Division operations must guard against zero denominators."""
        # eq_1161: cf1 * (p1 / cf2) where cf2 == 0
        self.assertEqual(eq_1161(10.0, 0.0, 50.0), 0.0)

        # calc_fuel_percentages where total == 0
        self.assertEqual(calc_fuel_percentages(0.0, 0.0, 0.0, 0.0), (0.0, 0.0, 0.0, 0.0, 0.0))

        # calc_attenuation where shrub or herb index == 0
        self.assertEqual(calc_attenuation(150.0, 0.0, 0.0, 20.0, 30.0), 150.0)

        # calc_stems_per_acre_pure where single_stem_area == 0 or cover == 0
        self.assertEqual(calc_stems_per_acre_pure(0.0, 25.0, 50.0), 0.0)
        self.assertEqual(calc_stems_per_acre_pure(100.0, 0.0, 50.0), 0.0)

    def test_sub_zero_cover_and_height_clamps(self):
        """Negative inputs to linear allometrics must be clamped to non-negative domains."""
        from tests.rvs_math_oracle import eq_165, eq_743

        # Negative cover should not reduce below intercept cf1
        self.assertEqual(eq_165(10.0, 2.0, -5.0), 10.0)
        self.assertEqual(eq_743(5.0, 1.0, -10.0, 20.0), 5.0)
        self.assertEqual(eq_743(5.0, 1.0, 10.0, -20.0), 5.0)


if __name__ == "__main__":
    unittest.main()
