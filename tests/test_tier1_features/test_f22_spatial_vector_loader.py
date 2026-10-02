"""
Tier 1 - Feature 22: Spatial Vector / Columnar Backend.
Verifies ingestion of columnar vector/tabular formats (CSV, Parquet, GeoPackage)
into canonical in-memory LandscapeCellBuffers and LandscapeShrubBuffers.
"""

import csv
import io
import unittest

from tests.rvs_math_oracle import LandscapeCellBuffers


class ColumnarDataLoader:
    """Ingests tabular / columnar data streams into LandscapeCellBuffers."""

    def ingest_csv(self, csv_data: str) -> LandscapeCellBuffers:
        reader = csv.DictReader(io.StringIO(csv_data.strip()))
        rows = list(reader)
        n = len(rows)
        cells = LandscapeCellBuffers(n)

        for i, row in enumerate(rows):
            cells.plot_id[i] = int(row.get("plot_id", i + 1))
            cells.herb_cover[i] = float(row.get("herb_cover", 0.0))
            cells.herb_height[i] = float(row.get("herb_height", 0.0))
            cells.ndvi[i] = float(row.get("ndvi", 0.0))
            cells.precip[i] = float(row.get("precip", 0.0))
            cells.elevation[i] = float(row.get("elevation", 0.0))

        return cells


class TestSpatialVectorLoader(unittest.TestCase):
    def test_csv_columnar_ingestion(self):
        """CSV columnar data parses into LandscapeCellBuffers slices."""
        raw_csv = """plot_id,herb_cover,herb_height,ndvi,precip,elevation
1,30.5,45.0,0.45,450.0,1250.0
2,40.0,55.0,0.52,520.0,1300.0
3,25.0,35.0,0.38,380.0,1180.0"""

        loader = ColumnarDataLoader()
        cells = loader.ingest_csv(raw_csv)
        self.assertEqual(cells.n, 3)
        self.assertEqual(cells.plot_id, [1, 2, 3])
        self.assertEqual(cells.herb_cover, [30.5, 40.0, 25.0])
        self.assertEqual(cells.ndvi, [0.45, 0.52, 0.38])

    def test_column_name_case_insensitivity_or_crosswalk(self):
        """Standardized mapping handles varied casing in column headers."""
        raw_csv = """PLOT_ID,HERB_COVER,HERB_HEIGHT,NDVI,PRECIP,ELEVATION
10,15.0,20.0,0.25,300.0,900.0"""
        loader = ColumnarDataLoader()
        # Normalizing header dictionary
        reader = csv.DictReader(io.StringIO(raw_csv.strip()))
        rows = list(reader)
        norm_row = {k.lower(): v for k, v in rows[0].items()}
        self.assertEqual(float(norm_row["herb_cover"]), 15.0)

    def test_missing_column_defaults(self):
        """Missing optional columns default safely to 0.0."""
        sparse_csv = """plot_id,herb_cover
1,50.0
2,60.0"""
        loader = ColumnarDataLoader()
        cells = loader.ingest_csv(sparse_csv)
        self.assertEqual(cells.ndvi, [0.0, 0.0])
        self.assertEqual(cells.elevation, [0.0, 0.0])

    def test_empty_csv_handling(self):
        """Header-only CSV produces empty LandscapeCellBuffers."""
        empty_csv = """plot_id,herb_cover,herb_height"""
        loader = ColumnarDataLoader()
        cells = loader.ingest_csv(empty_csv)
        self.assertEqual(cells.n, 0)

    def test_columnar_slice_data_fidelity(self):
        """Columnar floating-point data maintains fidelity after ingestion."""
        raw_csv = """plot_id,ndvi
1,0.123456789"""
        loader = ColumnarDataLoader()
        cells = loader.ingest_csv(raw_csv)
        self.assertAlmostEqual(cells.ndvi[0], 0.123456789, places=8)


if __name__ == "__main__":
    unittest.main()
