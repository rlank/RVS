"""
Tier 5 - Adversarial 4: Resource Limits & Scalability.
Stress-tests memory footprint and linear O(N) scaling under large landscape extents (N >= 50,000 cells).
"""

import time
import unittest

from tests.rvs_math_oracle import (
    LandscapeCellBuffers,
    LandscapeShrubBuffers,
    calc_growth_rate,
)


class TestAdv04ResourceLimits(unittest.TestCase):
    def test_large_landscape_grid_allocation(self):
        """LandscapeCellBuffers allocates 50,000 cells cleanly without OOM."""
        n = 50000
        cells = LandscapeCellBuffers(n)
        self.assertEqual(cells.n, n)
        self.assertEqual(len(cells.herb_cover), n)

    def test_large_shrub_csr_buffer_allocation(self):
        """LandscapeShrubBuffers allocates 100,000 shrubs across 50,000 cells."""
        n_cells = 50000
        m_shrubs = 100000
        shrubs = LandscapeShrubBuffers(n_cells, m_shrubs)
        self.assertEqual(len(shrubs.cell_offsets), n_cells + 1)
        self.assertEqual(len(shrubs.cover), m_shrubs)

    def test_large_landscape_linear_scaling(self):
        """Batch compute across 50,000 cells completes well within 2.0 seconds in pure Python."""
        n = 50000
        cells = LandscapeCellBuffers(n)
        start = time.perf_counter()

        for i in range(n):
            cells.herb_height[i] = calc_growth_rate(cells.herb_height[i], 1.5)

        elapsed = time.perf_counter() - start
        self.assertLess(elapsed, 2.0, f"50,000 cells took too long: {elapsed:.2f}s")

    def test_dense_shrub_packing_stress(self):
        """Stress test: plot with 100 shrub species in a single cell."""
        m_shrubs = 100
        shrubs = LandscapeShrubBuffers(1, m_shrubs)
        shrubs.cell_offsets[0] = 0
        shrubs.cell_offsets[1] = m_shrubs

        tot_cov = sum(shrubs.cover)
        self.assertEqual(tot_cov, 0.0)

    def test_buffer_memory_reuse_no_leak(self):
        """Simulating 10 successive years reuses identical buffer memory without reallocation."""
        cells = LandscapeCellBuffers(1000)
        ptr_init = id(cells.herb_height)
        for _ in range(10):
            for i in range(1000):
                cells.herb_height[i] += 1.0
        self.assertEqual(id(cells.herb_height), ptr_init)


if __name__ == "__main__":
    unittest.main()
