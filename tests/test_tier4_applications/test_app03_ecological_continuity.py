"""
Tier 4 - Application 3: Ecological Continuity & Non-Divergence.
Verifies ecological plausibility, bounded state variables, non-divergence,
and continuous population dynamics across 20-year simulation runs.
"""

import os
import sqlite3
import unittest


class TestApp03EcologicalContinuity(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
        cls.rvs_bin = os.path.join(cls.repo_root, "librvs/build/rvs")
        cls.demo_db = os.path.join(cls.repo_root, "rvs_demo.db")
        cls.out_db = os.path.join(cls.repo_root, "rvs_out_demo.db")

        # If rvs_out_demo.db is missing or lacks Biomass_Output, run simulation to generate it
        need_run = not os.path.exists(cls.out_db)
        if not need_run:
            try:
                conn = sqlite3.connect(cls.out_db)
                cur = conn.cursor()
                cur.execute("SELECT COUNT(*) FROM Biomass_Output")
                if cur.fetchone()[0] == 0:
                    need_run = True
                conn.close()
            except Exception:
                need_run = True

        if need_run and os.path.exists(cls.rvs_bin):
            import subprocess

            subprocess.run(
                [cls.rvs_bin, cls.demo_db, cls.out_db, "20"], cwd=cls.repo_root, capture_output=True
            )

    def test_output_database_present(self):
        """rvs_out_demo.db exists in repository root."""
        self.assertTrue(os.path.exists(self.out_db), f"Expected {self.out_db} to exist")

    def test_twenty_year_biomass_non_divergence(self):
        """Biomass values across all plots and years stay finite and do not diverge to infinity."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()
        cur.execute(
            "SELECT MIN(SHRUB_BIOMASS), MAX(HERB_BIOMASS), MAX(SHRUB_BIOMASS), MAX(avg_shrub_ht) FROM Biomass_Output"
        )
        min_shrub, max_herb, max_shrub, max_shrub_ht = cur.fetchone()
        conn.close()

        self.assertGreaterEqual(min_shrub, 0.0, f"Negative shrub biomass found: {min_shrub}")
        self.assertLess(max_shrub_ht, 1000.0, f"Shrub height exceeded physical bounds: {max_shrub_ht}")
        self.assertLess(max_herb, 1e7, f"Herb biomass diverged to {max_herb}")
        self.assertLess(max_shrub, 1e7, f"Shrub biomass diverged to {max_shrub}")

    def test_cover_bounded_within_physical_limits(self):
        """Cover values must remain within the physical [0.0, 100.0] percentage range."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()
        cur.execute(
            "SELECT MIN(herb_cover), MAX(herb_cover), MIN(tot_shrub_cov), MAX(tot_shrub_cov) FROM Biomass_Output"
        )
        min_hc, max_hc, min_sc, max_sc = cur.fetchone()
        conn.close()

        self.assertGreaterEqual(min_hc, 0.0)
        self.assertLessEqual(max_hc, 100.0 + 1e-4)
        self.assertGreaterEqual(min_sc, 0.0)
        self.assertLessEqual(max_sc, 100.0 + 1e-4)

    def test_fuel_loading_ecological_plausibility(self):
        """1-hr fuel loadings are within physically plausible rangeland ranges (< 100,000 lbs/acre)."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()
        cur.execute("SELECT MAX(fuel_1hr) FROM Fuels_Output")
        max_fuel = cur.fetchone()[0]
        conn.close()

        self.assertGreater(max_fuel, 0.0)
        self.assertLess(max_fuel, 100000.0)

    def test_all_61_plots_represented_continuously(self):
        """Every single plot has an unbroken record from Year 0 to Year 19."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()
        cur.execute("SELECT PLOT_ID, COUNT(DISTINCT year) FROM Biomass_Output GROUP BY PLOT_ID")
        rows = cur.fetchall()
        conn.close()

        self.assertEqual(len(rows), 61, "Not all 61 plots present")
        for pid, yr_count in rows:
            self.assertEqual(yr_count, 20, f"Plot {pid} has {yr_count} years instead of 20")


if __name__ == "__main__":
    unittest.main()
