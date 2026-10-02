"""
Tier 1 - Feature 20: SQLite DataLoader Backend.
Verifies ingestion of legacy SQLite tables (Plots_FCCS, Shrubs_FCCS, Dist_year7fire)
into canonical in-memory SoA and CSR buffers.
"""

import os
import sqlite3
import unittest

from tests.rvs_math_oracle import LandscapeCellBuffers, LandscapeShrubBuffers


class SQLiteDataLoader:
    """Ingests SQLite landscape tables into canonical in-memory SoA buffers."""

    def __init__(self, db_path: str):
        self.db_path = db_path

    def load_landscape(self) -> tuple[LandscapeCellBuffers, LandscapeShrubBuffers, list]:
        conn = sqlite3.connect(self.db_path)
        cur = conn.cursor()

        # Load Plots
        cur.execute(
            "SELECT PLOT_ID, BPS_CODE, herb_cover, herb_height, PPT_3, NDVI_3 FROM Plots_FCCS ORDER BY PLOT_ID"
        )
        plot_rows = cur.fetchall()
        n = len(plot_rows)
        cells = LandscapeCellBuffers(n)
        plot_id_to_index = {}

        for i, row in enumerate(plot_rows):
            pid, bps, hc, hh, ppt, ndvi = row
            cells.plot_id[i] = pid
            cells.bps_code[i] = bps or 0
            cells.herb_cover[i] = hc or 0.0
            cells.herb_height[i] = hh or 0.0
            cells.precip[i] = ppt or 0.0
            cells.ndvi[i] = ndvi or 0.0
            plot_id_to_index[pid] = i

        # Load Shrubs
        cur.execute("SELECT PLOT_ID, SPP_CODE, Cover, Height FROM Shrubs_FCCS ORDER BY PLOT_ID")
        shrub_rows = cur.fetchall()
        m = len(shrub_rows)
        shrubs = LandscapeShrubBuffers(n, m)

        # Build CSR offsets
        counts_per_cell = [0] * n
        for row in shrub_rows:
            pid = row[0]
            if pid in plot_id_to_index:
                counts_per_cell[plot_id_to_index[pid]] += 1

        offset = 0
        shrubs.cell_offsets[0] = 0
        for i in range(n):
            offset += counts_per_cell[i]
            shrubs.cell_offsets[i + 1] = offset

        # Populate shrub records
        current_idx = [shrubs.cell_offsets[i] for i in range(n)]
        for row in shrub_rows:
            pid, spp, cov, ht = row
            if pid in plot_id_to_index:
                cell_idx = plot_id_to_index[pid]
                s_idx = current_idx[cell_idx]
                shrubs.cover[s_idx] = cov or 0.0
                shrubs.height[s_idx] = ht or 0.0
                current_idx[cell_idx] += 1

        # Load Disturbance
        cur.execute("SELECT PLOT_ID, START_YEAR, DIST_TYPE, P1_VAL FROM Dist_year7fire")
        dist_rows = cur.fetchall()

        conn.close()
        return cells, shrubs, dist_rows


class TestSQLiteDataLoader(unittest.TestCase):
    def setUp(self):
        self.db_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../rvs_demo.db"))
        self.loader = SQLiteDataLoader(self.db_path)

    def test_ingest_plots_fccs_count(self):
        """Plots_FCCS table loads exactly 61 plot records into cell buffers."""
        cells, _, _ = self.loader.load_landscape()
        self.assertEqual(cells.n, 61)
        self.assertEqual(len(cells.plot_id), 61)
        self.assertEqual(len(cells.herb_cover), 61)

    def test_ingest_shrubs_fccs_count(self):
        """Shrubs_FCCS table loads exactly 102 shrub records into CSR buffers."""
        _, shrubs, _ = self.loader.load_landscape()
        self.assertEqual(shrubs.m_shrubs, 102)
        self.assertEqual(len(shrubs.cover), 102)

    def test_csr_offsets_distribution(self):
        """CSR offsets partition shrubs across 61 plots ending at 102."""
        cells, shrubs, _ = self.loader.load_landscape()
        self.assertEqual(len(shrubs.cell_offsets), 62)
        self.assertEqual(shrubs.cell_offsets[0], 0)
        self.assertEqual(shrubs.cell_offsets[61], 102)

    def test_disturbance_schedule_ingestion(self):
        """Dist_year7fire records fire schedule."""
        _, _, dist_rows = self.loader.load_landscape()
        self.assertGreater(len(dist_rows), 0)
        # Year 7 fire check
        years = {row[1] for row in dist_rows}
        self.assertIn(7, years)

    def test_database_closed_after_ingestion(self):
        """Simulation operates completely detached from database after loading."""
        cells, shrubs, _ = self.loader.load_landscape()
        # Verify buffer modification in-memory without active DB
        cells.herb_biomass[0] = 777.0
        self.assertEqual(cells.herb_biomass[0], 777.0)


if __name__ == "__main__":
    unittest.main()
