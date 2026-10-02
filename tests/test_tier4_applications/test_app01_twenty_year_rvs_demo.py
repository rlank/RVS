"""
Tier 4 - Application 1: Full 20-Year Simulation on rvs_demo.db.
Executes the full 20-year simulation against the demonstration SQLite database,
verifying complete execution, exit code 0, and output persistence.
"""

import os
import sqlite3
import subprocess
import tempfile
import unittest


class TestApp01TwentyYearRVSDemo(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
        cls.rvs_bin = os.path.join(cls.repo_root, "librvs/build/rvs")
        cls.in_db = os.path.join(cls.repo_root, "rvs_demo.db")
        cls.temp_dir = tempfile.TemporaryDirectory()
        cls.out_db = os.path.join(cls.temp_dir.name, "app01_out.db")

        # Run 20 years
        cmd = [cls.rvs_bin, cls.in_db, cls.out_db, "20"]
        cls.proc = subprocess.run(cmd, cwd=cls.repo_root, capture_output=True, text=True)

    @classmethod
    def tearDownClass(cls):
        cls.temp_dir.cleanup()

    def test_simulation_runs_to_completion_20_years(self):
        """Simulation executable exits cleanly with status 0."""
        self.assertEqual(
            self.proc.returncode,
            0,
            f"Execution failed with code {self.proc.returncode}. Stderr: {self.proc.stderr}",
        )

    def test_stdout_contains_all_twenty_years(self):
        """Standard output traces year increments from YEAR 0 to YEAR 19."""
        stdout = self.proc.stdout
        for y in range(20):
            self.assertIn(f"YEAR {y}", stdout, f"Missing output for YEAR {y}")

    def test_biomass_output_record_count(self):
        """Biomass_Output contains exactly 61 plots * 20 years = 1220 rows."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()
        cur.execute("SELECT COUNT(*) FROM Biomass_Output")
        count = cur.fetchone()[0]
        conn.close()
        self.assertEqual(count, 1220, f"Expected 1220 biomass rows, got {count}")

    def test_fuels_output_record_count(self):
        """Fuels_Output contains exactly 61 plots * 20 years = 1220 rows."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()
        cur.execute("SELECT COUNT(*) FROM Fuels_Output")
        count = cur.fetchone()[0]
        conn.close()
        self.assertEqual(count, 1220, f"Expected 1220 fuels rows, got {count}")

    def test_succession_output_record_count(self):
        """Succession_Output contains exactly 61 plots * 20 years = 1220 rows."""
        conn = sqlite3.connect(self.out_db)
        cur = conn.cursor()
        cur.execute("SELECT COUNT(*) FROM Succession_Output")
        count = cur.fetchone()[0]
        conn.close()
        self.assertEqual(count, 1220, f"Expected 1220 succession rows, got {count}")


if __name__ == "__main__":
    unittest.main()
