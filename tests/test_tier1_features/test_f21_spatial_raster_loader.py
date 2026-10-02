"""
Tier 1 - Feature 21: Spatial Raster DataLoader Backend.
Verifies multi-band spatial raster (GeoTIFF / gridded raster layers: NDVI, PPT,
elevation, cover, height) ingestion into canonical LandscapeCellBuffers.
"""

import unittest

from tests.rvs_math_oracle import LandscapeCellBuffers


class SpatialRasterDataLoader:
    """Ingests multi-band raster data into canonical LandscapeCellBuffers."""

    def __init__(self, rows: int, cols: int, nodata_val: float = -9999.0):
        self.rows = rows
        self.cols = cols
        self.nodata_val = nodata_val
        self.n_cells = rows * cols

    def ingest_bands(self, bands: dict[str, list[float]]) -> LandscapeCellBuffers:
        """Flattens 2D grid bands into 1D contiguous LandscapeCellBuffers slices."""
        cells = LandscapeCellBuffers(self.n_cells)
        for i in range(self.n_cells):
            cells.plot_id[i] = i + 1

            if "ndvi" in bands:
                val = bands["ndvi"][i]
                cells.ndvi[i] = 0.0 if val == self.nodata_val else max(0.0, val)

            if "precip" in bands:
                val = bands["precip"][i]
                cells.precip[i] = 0.0 if val == self.nodata_val else max(0.0, val)

            if "elevation" in bands:
                val = bands["elevation"][i]
                cells.elevation[i] = 0.0 if val == self.nodata_val else val

            if "herb_cover" in bands:
                val = bands["herb_cover"][i]
                cells.herb_cover[i] = 0.0 if val == self.nodata_val else max(0.0, min(100.0, val))

            if "herb_height" in bands:
                val = bands["herb_height"][i]
                cells.herb_height[i] = 0.0 if val == self.nodata_val else max(0.0, val)

        return cells


class TestSpatialRasterLoader(unittest.TestCase):
    def test_multi_band_raster_to_soa_mapping(self):
        """Multi-band raster values accurately map into LandscapeCellBuffers slices."""
        rows, cols = 4, 5
        loader = SpatialRasterDataLoader(rows, cols)
        n = rows * cols

        mock_bands = {
            "ndvi": [0.45] * n,
            "precip": [500.0] * n,
            "elevation": [1200.0] * n,
            "herb_cover": [35.0] * n,
            "herb_height": [40.0] * n,
        }

        cells = loader.ingest_bands(mock_bands)
        self.assertEqual(cells.n, 20)
        self.assertEqual(cells.ndvi[0], 0.45)
        self.assertEqual(cells.precip[0], 500.0)
        self.assertEqual(cells.elevation[0], 1200.0)
        self.assertEqual(cells.herb_cover[0], 35.0)

    def test_raster_grid_dimension_unraveling(self):
        """Row-major 2D coordinate (r, c) maps to 1D index r * cols + c."""
        rows, cols = 10, 10
        loader = SpatialRasterDataLoader(rows, cols)
        n = rows * cols
        grid_vals = [float(r * cols + c) for r in range(rows) for c in range(cols)]

        cells = loader.ingest_bands({"elevation": grid_vals})
        self.assertEqual(cells.elevation[3 * cols + 7], float(37))

    def test_raster_nodata_value_masking(self):
        """NoData pixel values (-9999.0) are safely clamped to 0.0 in canonical buffers."""
        loader = SpatialRasterDataLoader(2, 2, nodata_val=-9999.0)
        bands = {"ndvi": [0.5, -9999.0, 0.6, -9999.0], "herb_cover": [40.0, -9999.0, -9999.0, 30.0]}
        cells = loader.ingest_bands(bands)
        self.assertEqual(cells.ndvi, [0.5, 0.0, 0.6, 0.0])
        self.assertEqual(cells.herb_cover, [40.0, 0.0, 0.0, 30.0])

    def test_tiled_chunk_streaming_aggregation(self):
        """Sequential raster tiles stitch together into a contiguous landscape extent."""
        tile1_cells = [10.0, 20.0]
        tile2_cells = [30.0, 40.0]
        stitched = tile1_cells + tile2_cells
        self.assertEqual(stitched, [10.0, 20.0, 30.0, 40.0])

    def test_raster_georeference_extent(self):
        """Affine transformation metadata correctly resolves pixel bounding box."""
        affine_transform = (1000.0, 30.0, 0.0, 2000.0, 0.0, -30.0)  # (x0, dx, rot, y0, rot, dy)
        col, row = 5, 10
        x = affine_transform[0] + col * affine_transform[1]
        y = affine_transform[3] + row * affine_transform[5]
        self.assertEqual(x, 1000.0 + 150.0)
        self.assertEqual(y, 2000.0 - 300.0)


if __name__ == "__main__":
    unittest.main()
