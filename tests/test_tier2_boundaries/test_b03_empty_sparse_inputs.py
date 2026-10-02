"""
Tier 2 - Boundary 3: Empty & Sparse Inputs.
Tests responses under sparse landscapes, empty plots, and zero-shrub sub-canopies.
"""

import unittest

from tests.rvs_math_oracle import (
    LandscapeCellBuffers,
    LandscapeShrubBuffers,
    calc_attenuation,
    calc_s2b_quadratic_form,
)


class TestEmptySparseInputs(unittest.TestCase):
    def test_plot_with_zero_shrubs(self):
        """Cell with zero shrubs (CSR offset diff == 0) iterates safely."""
        cells = LandscapeCellBuffers(2)
        shrubs = LandscapeShrubBuffers(2, 1)
        # Cell 0 has 1 shrub, Cell 1 has 0 shrubs
        shrubs.cell_offsets = [0, 1, 1]

        # Extract Cell 1 shrubs
        c1_shrubs = shrubs.species_id[shrubs.cell_offsets[1] : shrubs.cell_offsets[2]]
        self.assertEqual(len(c1_shrubs), 0)

    def test_empty_buffers_allocation(self):
        """Zero cells and zero shrubs in landscape buffers allocate cleanly."""
        cells = LandscapeCellBuffers(0)
        shrubs = LandscapeShrubBuffers(0, 0)
        self.assertEqual(cells.n, 0)
        self.assertEqual(shrubs.m_shrubs, 0)
        self.assertEqual(len(shrubs.cell_offsets), 1)

    def test_all_cells_zero_herb_cover(self):
        """Zero herb cover throughout produces 0.0 attenuated biomass."""
        res = calc_attenuation(herb_b=0.0, hc=0.0, hh=0.0, sc=20.0, sh=50.0)
        self.assertEqual(res, 0.0)

    def test_sparse_covariance_matrix(self):
        """Sparse covariance matrix with mostly zeros evaluates properly in quadratic form."""
        x = [1.0, 2.0, 3.0]
        # Diagonal only (all off-diagonals 0.0)
        sigma = [4.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 6.0]
        # 1^2*4 + 2^2*5 + 3^2*6 = 4 + 20 + 54 = 78.0
        val = calc_s2b_quadratic_form(x, sigma, 3)
        self.assertEqual(val, 78.0)

    def test_empty_holdover_initial_years(self):
        """Holdover with all prior 3 years zero returns 0.0."""
        from tests.rvs_math_oracle import calc_herb_holdover

        self.assertEqual(calc_herb_holdover(0.0, 0.0, 0.0), 0.0)


if __name__ == "__main__":
    unittest.main()
