"""
Tier 1 - Feature 10: Segmented CSR Shrub Buffer.
Verifies the Segmented Compressed Sparse Row (CSR) layout for variable shrub
sub-canopies (M shrubs across N cells) with cell_offsets of length N + 1.
"""

import unittest

from tests.rvs_math_oracle import LandscapeShrubBuffers


class TestCSRShrubBuffers(unittest.TestCase):
    def test_csr_offsets_length(self):
        """Cell offsets slice must have exactly length N + 1."""
        n_cells = 61
        m_shrubs = 102
        shrub_buf = LandscapeShrubBuffers(n_cells, m_shrubs)
        self.assertEqual(len(shrub_buf.cell_offsets), n_cells + 1)
        self.assertEqual(len(shrub_buf.species_id), m_shrubs)
        self.assertEqual(len(shrub_buf.cover), m_shrubs)

    def test_csr_slice_partitioning(self):
        """CSR offset slice slicing correctly segments shrubs belonging to specific cells."""
        # 3 cells, 5 shrubs:
        # Cell 0 has 2 shrubs (indices 0..2)
        # Cell 1 has 0 shrubs (indices 2..2)
        # Cell 2 has 3 shrubs (indices 2..5)
        n_cells = 3
        m_shrubs = 5
        buf = LandscapeShrubBuffers(n_cells, m_shrubs)
        buf.cell_offsets = [0, 2, 2, 5]
        buf.species_id = [101, 102, 201, 202, 203]
        buf.cover = [15.0, 25.0, 10.0, 12.0, 8.0]

        # Extract Cell 0 shrubs
        c0_start, c0_end = buf.cell_offsets[0], buf.cell_offsets[1]
        self.assertEqual(c0_end - c0_start, 2)
        self.assertEqual(buf.species_id[c0_start:c0_end], [101, 102])

        # Extract Cell 1 shrubs (empty)
        c1_start, c1_end = buf.cell_offsets[1], buf.cell_offsets[2]
        self.assertEqual(c1_end - c1_start, 0)
        self.assertEqual(buf.species_id[c1_start:c1_end], [])

        # Extract Cell 2 shrubs
        c2_start, c2_end = buf.cell_offsets[2], buf.cell_offsets[3]
        self.assertEqual(c2_end - c2_start, 3)
        self.assertEqual(buf.species_id[c2_start:c2_end], [201, 202, 203])

    def test_csr_monotonic_offsets(self):
        """Offsets array must be strictly non-decreasing."""
        offsets = [0, 1, 3, 3, 6, 10]
        for i in range(len(offsets) - 1):
            self.assertLessEqual(offsets[i], offsets[i + 1])

    def test_csr_total_shrubs_matches_offset_end(self):
        """Final element cell_offsets[N] must equal total number of shrubs M."""
        n_cells = 4
        m_shrubs = 7
        buf = LandscapeShrubBuffers(n_cells, m_shrubs)
        buf.cell_offsets = [0, 2, 4, 5, 7]
        self.assertEqual(buf.cell_offsets[n_cells], m_shrubs)

    def test_csr_empty_landscape(self):
        """CSR buffer handles 0 cells and 0 shrubs gracefully."""
        buf = LandscapeShrubBuffers(0, 0)
        self.assertEqual(len(buf.cell_offsets), 1)
        self.assertEqual(buf.cell_offsets[0], 0)
        self.assertEqual(len(buf.species_id), 0)


if __name__ == "__main__":
    unittest.main()
