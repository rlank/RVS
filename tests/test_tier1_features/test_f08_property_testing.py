"""
Tier 1 - Feature 8: Property-Based Testing & Stochastic Invariants.
Validates numerical stability, non-panicking, monotonicity, and mathematical
invariants across stochastic domains.
"""

import math
import random
import unittest

from tests.rvs_math_oracle import (
    calc_fire_biomass_consumption,
    calc_graze_biomass_reduction,
    calc_s2b_quadratic_form,
    eq_165,
    eq_202,
    eq_998,
    fuels_eq_33,
    scale_shrub_cover,
)


class TestPropertyTesting(unittest.TestCase):
    def setUp(self):
        # Deterministic seed for reproducible property runs
        random.seed(42)

    def test_property_biomass_non_panicking_and_finite(self):
        """Stochastic draws across wide intervals must never crash or produce NaN."""
        for _ in range(500):
            cf1 = random.uniform(-10.0, 10.0)
            cf2 = random.uniform(-5.0, 5.0)
            dim = random.uniform(-50.0, 200.0)

            # Test eq_165
            b165 = eq_165(cf1, cf2, dim)
            self.assertFalse(math.isnan(b165))

            # Test eq_202
            b202 = eq_202(cf1, cf2, dim)
            self.assertFalse(math.isnan(b202))

            # Test eq_998
            b998 = eq_998(cf1, cf2, dim)
            self.assertFalse(math.isnan(b998))

    def test_property_fuels_monotonicity_under_scaling(self):
        """For positive coefficients, fuel loading should increase monotonically with positive input."""
        cf1, cf2, cf3 = 1.0, 0.5, 0.02
        inputs = sorted([random.uniform(1.0, 100.0) for _ in range(50)])
        outputs = [fuels_eq_33(cf1, cf2, cf3, p) for p in inputs]
        for i in range(len(outputs) - 1):
            self.assertLessEqual(outputs[i], outputs[i + 1])

    def test_property_succession_cover_bounded_invariant(self):
        """Canopy cover scaling must strictly stay within [0.0, 100.0] under any random delta."""
        for _ in range(500):
            current = random.uniform(-50.0, 150.0)
            delta = random.uniform(-200.0, 200.0)
            scaled = scale_shrub_cover(current, delta)
            self.assertGreaterEqual(scaled, 0.0)
            self.assertLessEqual(scaled, 100.0)

    def test_property_disturbance_depletion_invariant(self):
        """Post-disturbance biomass must never exceed pre-disturbance biomass."""
        for _ in range(500):
            pre_b = random.uniform(0.0, 5000.0)
            util = random.uniform(-20.0, 150.0)
            post_graze = calc_graze_biomass_reduction(pre_b, util)
            self.assertLessEqual(post_graze, pre_b + 1e-9)
            self.assertGreaterEqual(post_graze, 0.0)

            intensity = random.uniform(0.0, 100.0)
            k = random.uniform(0.001, 0.2)
            consumed = calc_fire_biomass_consumption(pre_b, k, intensity)
            self.assertLessEqual(consumed, pre_b + 1e-9)
            self.assertGreaterEqual(consumed, 0.0)

    def test_property_quadratic_form_positive_semi_definite(self):
        """x^T * Sigma * x must be >= 0 for positive semi-definite covariance matrices."""
        dim = 3
        # Identity matrix is positive definite
        sigma = [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]
        for _ in range(100):
            x = [random.uniform(-50.0, 50.0) for _ in range(dim)]
            val = calc_s2b_quadratic_form(x, sigma, dim)
            self.assertGreaterEqual(val, 0.0)


if __name__ == "__main__":
    unittest.main()
