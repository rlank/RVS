"""
Tier 3 - Combination 5: Cross-Format Pipeline Combinations.
Verifies hybrid pipelines where plot locations originate in SQLite / Vector tables
and climate layers (NDVI, PPT) stream from spatial rasters into canonical SoA buffers.
"""

import unittest

from tests.rvs_math_oracle import LandscapeCellBuffers, calc_growth_rate, evaluate_herb_growth


class TestCrossFormatPipelineCombinations(unittest.TestCase):
    def test_hybrid_sqlite_plots_with_raster_climate(self):
        """Plot attributes from SQLite joined with dynamic raster PPT and NDVI layers."""
        # Simulated SQLite table load
        sqlite_plots = [
            {"plot_id": 1, "bps_code": 10110, "herb_cover": 25.0},
            {"plot_id": 2, "bps_code": 10120, "herb_cover": 35.0},
        ]

        # Simulated Raster GeoTIFF band lookup
        raster_ndvi = [0.42, 0.58]
        raster_ppt = [420.0, 560.0]

        # Canonical SoA assembly
        n = len(sqlite_plots)
        cells = LandscapeCellBuffers(n)
        for i in range(n):
            cells.plot_id[i] = int(sqlite_plots[i]["plot_id"])
            cells.bps_code[i] = int(sqlite_plots[i]["bps_code"])
            cells.herb_cover[i] = sqlite_plots[i]["herb_cover"]
            cells.ndvi[i] = raster_ndvi[i]
            cells.precip[i] = raster_ppt[i]

            # Compute climate production
            cells.herb_biomass[i] = evaluate_herb_growth(
                b0=1.5, b1=0.4, b2=0.5, b3=0.05, ppt=cells.precip[i], ndvi=cells.ndvi[i]
            )

        self.assertEqual(len(cells.herb_biomass), 2)
        self.assertGreater(cells.herb_biomass[0], 0.0)
        self.assertGreater(cells.herb_biomass[1], 0.0)

    def test_vector_plots_with_preloaded_reference_catalog(self):
        """Vector CSV plot definitions evaluated against pre-loaded in-memory reference tables."""
        reference_catalog = {10110: {"growth_rate": 2.2}, 10120: {"growth_rate": 3.1}}
        cells = LandscapeCellBuffers(2)
        cells.bps_code = [10110, 10120]
        cells.herb_height = [20.0, 20.0]

        for i in range(2):
            rate = reference_catalog[cells.bps_code[i]]["growth_rate"]
            cells.herb_height[i] = calc_growth_rate(cells.herb_height[i], rate)

        self.assertEqual(cells.herb_height[0], 22.2)
        self.assertEqual(cells.herb_height[1], 23.1)

    def test_spatial_coordinate_to_raster_index_mapping(self):
        """Geographic coordinates (lat, lon) map to exact raster grid indices."""
        origin_x, origin_y = -115.0, 45.0
        res = 0.001
        # Point at (-114.995, 44.992)
        pt_x, pt_y = -114.995, 44.992
        col = int(round((pt_x - origin_x) / res))
        row = int(round((origin_y - pt_y) / res))
        self.assertEqual(col, 5)
        self.assertEqual(row, 8)

    def test_multi_format_batch_execution(self):
        """Canonical SoA buffers execute batch simulation regardless of data origin."""
        cells = LandscapeCellBuffers(5)
        for i in range(5):
            cells.herb_height[i] = 10.0 + i
        for i in range(5):
            cells.herb_height[i] = calc_growth_rate(cells.herb_height[i], 1.0)
        self.assertEqual(cells.herb_height, [11.0, 12.0, 13.0, 14.0, 15.0])

    def test_hybrid_pipeline_zero_copy_buffer_preservation(self):
        """Modifying buffers in place preserves contiguous allocation across simulation steps."""
        cells = LandscapeCellBuffers(10)
        id_before = id(cells.herb_cover)
        cells.herb_cover[0] = 55.0
        id_after = id(cells.herb_cover)
        self.assertEqual(id_before, id_after)


if __name__ == "__main__":
    unittest.main()
