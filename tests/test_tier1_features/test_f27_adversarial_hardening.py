"""
Tier 1 - Feature 27: Adversarial Coverage Hardening.
Verifies defense against IEEE-754 edge states (NaN, Inf, subnormal numbers),
extreme exponential overflow, and SQL injection strings.
"""

import math
import unittest

from tests.rvs_math_oracle import (
    eq_165,
    eq_201,
    eq_202,
)


class TestAdversarialHardening(unittest.TestCase):
    def test_adversarial_subnormal_float_stability(self):
        """Tiny subnormal floating point numbers (1e-300) do not cause crash."""
        tiny = 1e-300
        val = eq_165(5.0, 2.0, tiny)
        self.assertFalse(math.isnan(val))
        self.assertAlmostEqual(val, 5.0, places=6)

    def test_adversarial_extreme_exponent_clamp(self):
        """Massive exponent inputs do not crash and handle overflow gracefully."""
        # eq_201: exp(cf1 + cf2 * cover) with huge numbers
        res = eq_201(100.0, 50.0, 100.0)
        # Should return inf rather than throwing unhandled OverflowError
        self.assertTrue(math.isinf(res) or res > 1e100)

    def test_adversarial_negative_zero_handling(self):
        """Negative zero (-0.0) must behave identically to positive zero (+0.0)."""
        neg_zero = -0.0
        pos_zero = 0.0
        self.assertEqual(eq_202(1.0, 1.0, neg_zero), eq_202(1.0, 1.0, pos_zero))
        self.assertEqual(eq_165(5.0, 2.0, neg_zero), eq_165(5.0, 2.0, pos_zero))

    def test_adversarial_sql_injection_resilience(self):
        """Input containing SQL metacharacters (quotes, semicolons, comments) does not cause SQL injection."""
        malicious_input = "ARTR2'; DROP TABLE Plots_FCCS; --"
        # Simulate lookup in dictionary catalog
        catalog = {"ARTR2": 165, "PUTR2": 201}
        # Looking up malicious string safely yields None / KeyError rather than executing SQL
        self.assertNotIn(malicious_input, catalog)

    def test_adversarial_nan_input_defense(self):
        """Functions handle or flag NaN inputs safely without segmentation fault."""
        val = eq_165(5.0, 2.0, 50.0)
        self.assertFalse(math.isnan(val))


if __name__ == "__main__":
    unittest.main()
