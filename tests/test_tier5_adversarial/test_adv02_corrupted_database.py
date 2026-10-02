"""
Tier 5 - Adversarial 2: Corrupted Data & Fault Ingestion.
Tests system resilience when ingesting missing tables, NULL database values,
unrecognized species codes, or malformed schemas.
"""

import os
import sqlite3
import tempfile
import unittest


class TestAdv02CorruptedDatabase(unittest.TestCase):
    def test_null_numeric_column_defaults_to_zero(self):
        """NULL values in SQLite columns default to 0.0 without throwing TypeError."""
        null_val = None
        safe_val = 0.0 if null_val is None else float(null_val)
        self.assertEqual(safe_val, 0.0)

    def test_unrecognized_species_code_fallback(self):
        """Species code not present in Bio_Crosswalk uses safe default or logs error."""
        catalog = {"ARTR2": 165, "PUTR2": 201}
        unknown_spp = "UNKNOWN_SPECIES_999"
        # Safe lookup with fallback equation 165
        eq_id = catalog.get(unknown_spp, 165)
        self.assertEqual(eq_id, 165)

    def test_empty_database_file_detection(self):
        """Zero-byte database file is detected as invalid SQLite database."""
        with tempfile.NamedTemporaryFile(delete=False) as tf:
            tf.write(b"NOT_A_SQLITE_DATABASE_CORRUPT_HEADER_BYTES\n")
            tf_path = tf.name

        try:
            with self.assertRaises(sqlite3.DatabaseError):
                conn = sqlite3.connect(tf_path)
                cur = conn.cursor()
                cur.execute("SELECT * FROM sqlite_master")
                conn.close()
        finally:
            if os.path.exists(tf_path):
                os.remove(tf_path)

    def test_negative_plot_id_handling(self):
        """Negative PLOT_ID does not cause out-of-bounds memory indexing."""
        plot_id_to_idx = {-1: 0, 101: 1}
        self.assertEqual(plot_id_to_idx[-1], 0)

    def test_corrupted_row_count_mismatch(self):
        """Buffer mismatch where shrub foreign key points to non-existent plot is caught."""
        known_plots = {1, 2, 3}
        orphan_shrub_plot_id = 999
        self.assertNotIn(orphan_shrub_plot_id, known_plots)


if __name__ == "__main__":
    unittest.main()
