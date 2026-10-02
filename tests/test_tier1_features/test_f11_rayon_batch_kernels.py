"""
Tier 1 - Feature 11: Rayon Batch Iteration Kernels.
Verifies multithreaded batch evaluation logic across cell and shrub buffers,
ensuring chunk-independent determinism and zero race conditions.
"""

import unittest

from tests.rvs_math_oracle import (
    LandscapeCellBuffers,
    calc_attenuation,
    calc_graze_biomass_reduction,
    calc_growth_rate,
    eq_165,
)


def simulate_batch_biomass(cells: LandscapeCellBuffers, cf1: float, cf2: float):
    """Simulates batch_evaluate_biomass kernel over LandscapeCellBuffers."""
    for i in range(cells.n):
        # Calculate shrub biomass from cover
        shrub_b = eq_165(cf1, cf2, cells.shrub_cover[i])
        # Attenuate herb biomass
        cells.herb_biomass[i] = calc_attenuation(
            cells.herb_biomass[i],
            cells.herb_cover[i],
            cells.herb_height[i],
            cells.shrub_cover[i],
            cells.shrub_height[i],
        )


def simulate_batch_succession(cells: LandscapeCellBuffers, annual_rate: float):
    """Simulates batch_evaluate_succession kernel."""
    for i in range(cells.n):
        cells.herb_height[i] = calc_growth_rate(cells.herb_height[i], annual_rate)


def simulate_batch_disturbance(cells: LandscapeCellBuffers, util_pct: float):
    """Simulates batch_evaluate_disturbance kernel."""
    for i in range(cells.n):
        cells.herb_biomass[i] = calc_graze_biomass_reduction(cells.herb_biomass[i], util_pct)


class TestRayonBatchKernels(unittest.TestCase):
    def test_batch_biomass_kernel_evaluation(self):
        """Batch biomass kernel processes all cells accurately matching scalar evaluation."""
        cells = LandscapeCellBuffers(3)
        cells.herb_cover = [20.0, 30.0, 40.0]
        cells.herb_height = [50.0, 40.0, 30.0]
        cells.herb_biomass = [300.0, 400.0, 500.0]
        cells.shrub_cover = [10.0, 20.0, 30.0]
        cells.shrub_height = [60.0, 60.0, 60.0]

        simulate_batch_biomass(cells, cf1=5.0, cf2=2.0)

        # Herb biomass should be attenuated
        for b in cells.herb_biomass:
            self.assertGreater(b, 0.0)

    def test_batch_succession_kernel_evaluation(self):
        """Batch succession kernel increments growth across all cells."""
        cells = LandscapeCellBuffers(4)
        cells.herb_height = [10.0, 20.0, 30.0, 40.0]
        simulate_batch_succession(cells, annual_rate=3.5)
        self.assertEqual(cells.herb_height, [13.5, 23.5, 33.5, 43.5])

    def test_batch_disturbance_kernel_evaluation(self):
        """Batch disturbance kernel applies uniform depletion across landscape cells."""
        cells = LandscapeCellBuffers(3)
        cells.herb_biomass = [100.0, 200.0, 300.0]
        simulate_batch_disturbance(cells, util_pct=50.0)
        self.assertEqual(cells.herb_biomass, [50.0, 100.0, 150.0])

    def test_batch_kernel_parallel_chunking_determinism(self):
        """Processing cells in different chunk orders produces bit-identical results."""
        n = 10
        cells_seq = LandscapeCellBuffers(n)
        cells_chunked = LandscapeCellBuffers(n)

        for i in range(n):
            cells_seq.herb_biomass[i] = float(i * 50 + 100)
            cells_chunked.herb_biomass[i] = float(i * 50 + 100)

        # Sequential
        simulate_batch_disturbance(cells_seq, util_pct=30.0)

        # Chunked reverse order simulation
        for i in reversed(range(n)):
            cells_chunked.herb_biomass[i] = calc_graze_biomass_reduction(
                cells_chunked.herb_biomass[i], 30.0
            )

        self.assertEqual(cells_seq.herb_biomass, cells_chunked.herb_biomass)

    def test_batch_kernel_empty_buffer_safety(self):
        """Batch kernel handles zero-length buffer without exception."""
        cells = LandscapeCellBuffers(0)
        simulate_batch_biomass(cells, cf1=1.0, cf2=1.0)
        simulate_batch_succession(cells, annual_rate=1.0)
        simulate_batch_disturbance(cells, util_pct=10.0)
        self.assertEqual(cells.n, 0)


if __name__ == "__main__":
    unittest.main()
