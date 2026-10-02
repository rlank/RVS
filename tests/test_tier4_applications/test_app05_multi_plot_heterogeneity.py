"""
Tier 4 - Application 5: Multi-Plot Landscape Heterogeneity.
Validates that the simulation engine accurately captures spatial heterogeneity across
the 61 distinct FCCS plots spanning multiple Biophysical Settings and vegetation types.
"""

import os
import sqlite3
import unittest


class TestApp05MultiPlotHeterogeneity(unittest.TestCase):
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

    def test_heterogeneous_bps_codes_represented(self):
        """Plots_FCCS contains multiple distinct Biophysical Settings (BPS) models."""
        conn = sqlite3.connect(self.demo_db)
        cur = conn.cursor()
        cur.execute("SELECT COUNT(DISTINCT BPS_CODE) FROM Plots_FCCS")
        bps_count = cur.fetchone()[0]
        conn.close()
        self.assertGreater(bps_count, 1, "Landscape lacks BPS heterogeneity")

    def test_climate_precipitation_spread(self):
        """Plots exhibit a realistic range of precipitation values across climate regimes."""
        conn = sqlite3.connect(self.demo_db)
        cur = conn.cursor()
        cur.execute("SELECT MIN(PPT_3), MAX(PPT_3) FROM Plots_FCCS")
        min_ppt, max_ppt = cur.fetchone()
        conn.close()
        self.assertGreater(max_ppt - min_ppt, 50.0, "Precipitation distribution is degenerate")

    def test_shrub_species_diversity(self):
        """Shrubs_FCCS includes multiple shrub species across the landscape plots."""
        conn = sqlite3.connect(self.demo_db)
        cur = conn.cursor()
        cur.execute("SELECT COUNT(DISTINCT spp_code) FROM Shrubs_FCCS")
        spp_count = cur.fetchone()[0]
        conn.close()
        self.assertGreater(spp_count, 5, "Fewer than 5 distinct shrub species represented")

    def test_variable_shrub_packing_density_per_plot(self):
        """Different plots support differing numbers of shrub species (from 0 to multiple)."""
        conn = sqlite3.connect(self.demo_db)
        cur = conn.cursor()
        cur.execute("SELECT PLOT_ID, COUNT(*) FROM Shrubs_FCCS GROUP BY PLOT_ID")
        counts = [row[1] for row in cur.fetchall()]
        conn.close()
        self.assertGreater(len(set(counts)), 1, "All plots have identical shrub counts")

    def test_biomass_variance_across_plots(self):
        """Output biomass reflects plot-to-plot heterogeneity rather than a uniform constant."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()
        cur.execute(
            "SELECT MIN(HERB_BIOMASS), MAX(HERB_BIOMASS) FROM Biomass_Output WHERE year = 0"
        )
        min_b, max_b = cur.fetchone()
        conn.close()
        self.assertGreater(max_b - min_b, 10.0, "Year 0 biomass across plots is uniform")


if __name__ == "__main__":
    unittest.main()
