"""
Tier 1 - Feature 9: Contiguous SoA Buffer Layout.
Verifies the canonical Structure-of-Arrays (SoA) layout for cell/plot-level buffers (length N).
"""

import unittest

from tests.rvs_math_oracle import LandscapeCellBuffers


class TestSoABuffers(unittest.TestCase):
    def test_soa_buffer_initialization(self):
        """Cell buffers must allocate exactly N elements per attribute column."""
        n = 100
        buf = LandscapeCellBuffers(n)
        self.assertEqual(buf.n, 100)
        self.assertEqual(len(buf.plot_id), 100)
        self.assertEqual(len(buf.herb_cover), 100)
        self.assertEqual(len(buf.herb_height), 100)
        self.assertEqual(len(buf.herb_biomass), 100)
        self.assertEqual(len(buf.shrub_cover), 100)
        self.assertEqual(len(buf.shrub_height), 100)
        self.assertEqual(len(buf.ndvi), 100)
        self.assertEqual(len(buf.precip), 100)
        self.assertEqual(len(buf.fuel_1hr), 100)

    def test_soa_contiguous_slice_lengths(self):
        """All contiguous slice arrays in LandscapeCellBuffers must have identical length N."""
        n = 61  # FCCS plot count in rvs_demo.db
        buf = LandscapeCellBuffers(n)
        attrs = [
            buf.plot_id,
            buf.bps_code,
            buf.herb_cover,
            buf.herb_height,
            buf.herb_biomass,
            buf.shrub_cover,
            buf.shrub_height,
            buf.ndvi,
            buf.precip,
            buf.elevation,
            buf.fuel_1hr,
            buf.fuel_10hr,
            buf.fuel_100hr,
            buf.fuel_1000hr,
        ]
        for arr in attrs:
            self.assertEqual(len(arr), n)

    def test_soa_batch_data_population(self):
        """Batch population of cell attributes preserves index ordering and values."""
        n = 5
        buf = LandscapeCellBuffers(n)
        plot_ids = [101, 102, 103, 104, 105]
        ndvis = [0.2, 0.35, 0.45, 0.6, 0.75]
        precs = [250.0, 300.0, 420.0, 510.0, 680.0]

        for i in range(n):
            buf.plot_id[i] = plot_ids[i]
            buf.ndvi[i] = ndvis[i]
            buf.precip[i] = precs[i]

        self.assertEqual(buf.plot_id, plot_ids)
        self.assertEqual(buf.ndvi, ndvis)
        self.assertEqual(buf.precip, precs)

    def test_soa_zero_length_allocation(self):
        """Zero-sized landscape grid allocation handles empty buffers cleanly."""
        buf = LandscapeCellBuffers(0)
        self.assertEqual(buf.n, 0)
        self.assertEqual(len(buf.plot_id), 0)
        self.assertEqual(len(buf.herb_cover), 0)

    def test_soa_slice_mutation_independence(self):
        """Mutating one attribute slice does not affect or alias other attribute slices."""
        buf = LandscapeCellBuffers(10)
        buf.herb_cover[0] = 45.0
        self.assertEqual(buf.herb_cover[0], 45.0)
        self.assertEqual(buf.shrub_cover[0], 0.0)
        self.assertEqual(buf.ndvi[0], 0.0)


if __name__ == "__main__":
    unittest.main()
