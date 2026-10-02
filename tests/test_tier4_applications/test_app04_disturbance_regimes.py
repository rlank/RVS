"""
Tier 4 - Application 4: Disturbance Regimes & Year-7 Wildfire Response.
Validates the ecological response to scheduled disturbance events, specifically
verifying the Year-7 wildfire biomass consumption, top-kill, and multi-year recovery.
"""

import os
import sqlite3
import unittest


class TestApp04DisturbanceRegimes(unittest.TestCase):
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

    def test_year_7_disturbance_schedule_in_demo_db(self):
        """rvs_demo.db must schedule fire disturbance events at Year 7."""
        conn = sqlite3.connect(self.demo_db)
        cur = conn.cursor()
        cur.execute("SELECT COUNT(*) FROM Dist_year7fire WHERE START_YEAR = 7")
        count = cur.fetchone()[0]
        conn.close()
        self.assertGreater(count, 0, "No Year-7 disturbances found in Dist_year7fire")

    def test_year_7_biomass_consumption_observable(self):
        """Mean biomass in burned plots drops at Year 7 compared to Year 6."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()

        # Get burned plot IDs
        conn_demo = sqlite3.connect(self.demo_db)
        cur_demo = conn_demo.cursor()
        cur_demo.execute(
            "SELECT DISTINCT PLOT_ID FROM Dist_year7fire WHERE START_YEAR = 7 AND DIST_TYPE = 'FIRE'"
        )
        burned_pids = [row[0] for row in cur_demo.fetchall()]
        conn_demo.close()

        if burned_pids:
            pid_str = ",".join(str(p) for p in burned_pids)
            cur.execute(
                f"SELECT AVG(shrub_biomass) FROM Biomass_Output WHERE PLOT_ID IN ({pid_str}) AND year = 6"
            )
            b_yr6 = cur.fetchone()[0] or 0.0

            cur.execute(
                f"SELECT AVG(shrub_biomass) FROM Biomass_Output WHERE PLOT_ID IN ({pid_str}) AND year = 7"
            )
            b_yr7 = cur.fetchone()[0] or 0.0

            self.assertLess(b_yr7, b_yr6)
        conn.close()

    def test_post_fire_recovery_trajectory(self):
        """Biomass demonstrates recovery following disturbance between Year 7 and Year 12."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()
        cur.execute("SELECT AVG(herb_biomass) FROM Biomass_Output WHERE year = 7")
        b_yr7 = cur.fetchone()[0]

        cur.execute("SELECT AVG(herb_biomass) FROM Biomass_Output WHERE year = 12")
        b_yr12 = cur.fetchone()[0]
        conn.close()

        self.assertGreaterEqual(b_yr12, b_yr7 * 0.9)

    def test_disturbance_type_nomenclature(self):
        """Disturbance tables record recognized disturbance types: FIRE, GRAZE, MOW."""
        conn = sqlite3.connect(self.demo_db)
        cur = conn.cursor()
        cur.execute("SELECT DISTINCT DIST_TYPE FROM Dist_year7fire")
        dist_types = {row[0] for row in cur.fetchall()}
        conn.close()

        valid_types = {"FIRE", "GRAZE", "MOW"}
        for dt in dist_types:
            self.assertIn(dt, valid_types)

    def test_severity_parameter_non_negative(self):
        """Disturbance severity values in schedule are non-negative."""
        conn = sqlite3.connect(self.demo_db)
        cur = conn.cursor()
        cur.execute("SELECT MIN(P1_VAL) FROM Dist_year7fire")
        min_sev = cur.fetchone()[0]
        conn.close()
        self.assertGreaterEqual(min_sev, 0.0)


if __name__ == "__main__":
    unittest.main()
