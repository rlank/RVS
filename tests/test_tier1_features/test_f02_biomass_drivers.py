"""
Tier 1 - Feature 2: Biomass Driver Routines B19–B22.
Verifies pure biomass driver routines: calc_crown_width, calc_stems_per_acre_pure,
calc_herb_holdover, and calc_attenuation.
"""

import math
import unittest

from tests.rvs_math_oracle import (
    EXPANSION_FACTOR,
    PI,
    calc_attenuation,
    calc_crown_width,
    calc_herb_holdover,
    calc_stems_per_acre_pure,
)


class TestBiomassDrivers(unittest.TestCase):
    def test_crown_width_calculation(self):
        """B19: Crown diameter from circular crown area in cm^2."""
        # Area = 10000 cm^2 -> radius = sqrt(10000 / pi) ~ 56.4189 -> width = 112.8379
        area = 10000.0
        expected = 2.0 * math.sqrt(area / PI)
        actual = calc_crown_width(area)
        self.assertAlmostEqual(actual, expected, places=6)

    def test_stems_per_acre_expansion(self):
        """B20: Stems per acre without height dampening (tall shrubs)."""
        single_stem_area = 5000.0  # cm^2
        cover = 25.0  # %
        height = (
            80.0  # cm -> reduction = exp(0.0809 * 80) * 0.0053 = exp(6.472)*0.0053 ~ 3.42 > 0.4536
        )
        expanded = EXPANSION_FACTOR / (single_stem_area * 0.0001)
        expected = expanded * (cover / 100.0)
        actual = calc_stems_per_acre_pure(single_stem_area, cover, height)
        self.assertAlmostEqual(actual, expected, places=4)
        self.assertGreater(actual, 0.0)

    def test_stems_per_acre_reduction_cutoff(self):
        """B20: Stems per acre with low-height dampening factor."""
        single_stem_area = 2000.0
        cover = 10.0
        height = 10.0  # cm -> reduction = exp(0.809) * 0.0053 ~ 0.0119 <= 0.453595949
        expanded = EXPANSION_FACTOR / (single_stem_area * 0.0001)
        stems_raw = expanded * (cover / 100.0)
        reduction = math.exp(0.0809 * 10.0) * 0.0053
        expected = stems_raw * reduction
        actual = calc_stems_per_acre_pure(single_stem_area, cover, height)
        self.assertAlmostEqual(actual, expected, places=4)
        self.assertLess(actual, stems_raw)

    def test_herb_holdover_three_year_sum(self):
        """B21: Standing dead herb holdover from 3 prior years."""
        p0, p1, p2 = 1000.0, 800.0, 600.0
        # 1000*0.3 + 800*0.1 + 600*0.05 = 300 + 80 + 30 = 410.0
        actual = calc_herb_holdover(p0, p1, p2)
        self.assertEqual(actual, 410.0)

    def test_attenuation_canopy_competition(self):
        """B22: Attenuation between herb understory and shrub canopy."""
        herb_b = 500.0
        # Scenario 1: herb index > shrub index (hi=40*50=2000, si=20*50=1000) -> ratio = 1000/2000 = 0.5
        att1 = calc_attenuation(herb_b, hc=40.0, hh=50.0, sc=20.0, sh=50.0)
        self.assertAlmostEqual(att1, 250.0, places=4)

        # Scenario 2: shrub index > herb index (hi=1000, si=2000) -> ratio = 1000/2000 = 0.5
        att2 = calc_attenuation(herb_b, hc=20.0, hh=50.0, sc=40.0, sh=50.0)
        self.assertAlmostEqual(att2, 250.0, places=4)

        # Scenario 3: equal index -> ratio = 1.0 -> no reduction
        att3 = calc_attenuation(herb_b, hc=30.0, hh=50.0, sc=30.0, sh=50.0)
        self.assertAlmostEqual(att3, 500.0, places=4)

    def test_attenuation_boundary_properties(self):
        """B22: Attenuation when shrub or herb index is non-positive returns raw biomass."""
        herb_b = 350.0
        res_zero_shrub = calc_attenuation(herb_b, hc=30.0, hh=40.0, sc=0.0, sh=0.0)
        self.assertEqual(res_zero_shrub, 350.0)
        res_zero_herb = calc_attenuation(herb_b, hc=0.0, hh=0.0, sc=30.0, sh=50.0)
        self.assertEqual(res_zero_herb, 350.0)


if __name__ == "__main__":
    unittest.main()
