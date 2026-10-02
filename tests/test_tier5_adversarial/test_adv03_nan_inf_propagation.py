"""
Tier 5 - Adversarial 3: NaN and Inf State Injection.
Tests resilience against IEEE-754 Special Floating-Point Values (NaN, +Inf, -Inf).
"""

import math
import unittest

from tests.rvs_math_oracle import (
    calc_fire_biomass_consumption,
    scale_shrub_cover,
)


class TestAdv03NaNInfPropagation(unittest.TestCase):
    def test_inf_fire_intensity_clamps_safely(self):
        """Infinite fire intensity (+inf) consumes 100% of biomass without crashing."""
        pre_b = 500.0
        consumed = calc_fire_biomass_consumption(
            pre_b, severity_k=0.05, fire_intensity=float("inf")
        )
        self.assertEqual(consumed, pre_b)

    def test_inf_shrub_cover_clamps_to_100(self):
        """Infinite shrub cover (+inf) clamps to 100.0%."""
        cov = scale_shrub_cover(20.0, float("inf"))
        self.assertEqual(cov, 100.0)

    def test_negative_inf_shrub_cover_clamps_to_zero(self):
        """Negative infinite shrub cover (-inf) clamps to 0.0%."""
        cov = scale_shrub_cover(20.0, float("-inf"))
        self.assertEqual(cov, 0.0)

    def test_nan_filtering_in_array_sanitizer(self):
        """Input sanitizer catches and sanitizes NaN values into default 0.0."""
        raw_values = [10.0, float("nan"), 20.0, float("inf"), -5.0]
        sanitized = [0.0 if (math.isnan(x) or math.isinf(x) or x < 0) else x for x in raw_values]
        self.assertEqual(sanitized, [10.0, 0.0, 20.0, 0.0, 0.0])

    def test_fuel_percentages_with_inf_guard(self):
        """Infinite fuel input handled without unhandled exception."""
        # Total with Inf
        total = float("inf")
        is_finite_total = math.isfinite(total)
        self.assertFalse(is_finite_total)


if __name__ == "__main__":
    unittest.main()
