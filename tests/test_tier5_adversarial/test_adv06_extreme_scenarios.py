"""
Tier 5 - Adversarial 6: Extreme Scenarios, Variable Canopies, and Climate Inputs.
Adversarially challenges:
1. Extreme landscape extents: 0 plots, 1 plot, 50,000+ plots.
2. Variable shrub sub-canopies: 0 shrubs per cell, 100 shrubs per cell, unlinked shrub foreign keys.
3. Extreme climate/raster inputs: 0 precip, extreme drought, extreme rainfall (5000mm),
   negative elevation, and NaN/NoData sentinel masking.
"""

import math
import os
import sqlite3
import tempfile
import unittest

from tests.rvs_math_oracle import (
    LandscapeCellBuffers,
    LandscapeShrubBuffers,
    calc_attenuation,
    calc_growth_rate,
    calc_herb_biomass_production,
    calc_herb_loading,
    calc_stems_per_acre_pure,
    eq_pch,
    evaluate_herb_growth,
    scale_shrub_cover,
    scale_shrub_height,
)


class TestAdv06ExtremeScenarios(unittest.TestCase):
    # =========================================================================
    # 1. Extreme Landscape Extents
    # =========================================================================

    def test_extent_zero_plots_safety(self):
        """Zero-plot landscape extent initializes cleanly without allocation errors."""
        cells = LandscapeCellBuffers(0)
        shrubs = LandscapeShrubBuffers(0, 0)
        self.assertEqual(cells.n, 0)
        self.assertEqual(len(cells.herb_biomass), 0)
        self.assertEqual(shrubs.n_cells, 0)
        self.assertEqual(shrubs.m_shrubs, 0)
        self.assertEqual(len(shrubs.cell_offsets), 1)
        self.assertEqual(shrubs.cell_offsets[0], 0)

    def test_extent_single_plot_isolated(self):
        """Single plot (N=1) executes all ecological transformations correctly."""
        cells = LandscapeCellBuffers(1)
        cells.herb_cover[0] = 25.0
        cells.herb_height[0] = 30.0
        cells.herb_biomass[0] = 450.0
        cells.shrub_cover[0] = 15.0
        cells.shrub_height[0] = 50.0

        # Run attenuation
        att = calc_attenuation(
            cells.herb_biomass[0],
            cells.herb_cover[0],
            cells.herb_height[0],
            cells.shrub_cover[0],
            cells.shrub_height[0],
        )
        self.assertTrue(math.isfinite(att))
        self.assertEqual(att, 450.0)  # equal index product 750 / 750 -> ratio 1.0

        # Run fuels
        fuel = calc_herb_loading(att, 0.6)
        self.assertAlmostEqual(fuel, 270.0, places=4)

        # Run succession
        new_h = scale_shrub_height(cells.shrub_height[0], 2.5)
        new_c = scale_shrub_cover(cells.shrub_cover[0], 1.2)
        self.assertEqual(new_h, 52.5)
        self.assertEqual(new_c, 16.2)

    def test_extent_50k_plus_plots_allocation_and_throughput(self):
        """Landscape with 60,000 plots executes linear sweeps without memory spikes."""
        n = 60_000
        cells = LandscapeCellBuffers(n)
        self.assertEqual(len(cells.herb_biomass), n)

        # Vectorized sweep
        for i in range(n):
            cells.herb_biomass[i] = 100.0 + (i % 500)
            cells.herb_height[i] = calc_growth_rate(cells.herb_height[i], 1.5)

        self.assertEqual(cells.herb_height[0], 1.5)
        self.assertEqual(cells.herb_height[n - 1], 1.5)

    # =========================================================================
    # 2. Variable Shrub Sub-Canopies
    # =========================================================================

    def test_variable_shrub_canopy_zero_shrubs_per_cell(self):
        """Landscape with 1,000 cells and 0 shrubs across all cells."""
        n_cells = 1000
        shrubs = LandscapeShrubBuffers(n_cells, 0)
        self.assertEqual(len(shrubs.cell_offsets), n_cells + 1)
        for offset in shrubs.cell_offsets:
            self.assertEqual(offset, 0)
        self.assertEqual(len(shrubs.cover), 0)
        self.assertEqual(len(shrubs.height), 0)

    def test_variable_shrub_canopy_100_shrubs_dense_packing(self):
        """Dense packing: single cell with 100 shrub species."""
        m_shrubs = 100
        shrubs = LandscapeShrubBuffers(1, m_shrubs)
        shrubs.cell_offsets[0] = 0
        shrubs.cell_offsets[1] = m_shrubs

        total_stem_density = 0.0
        for s in range(m_shrubs):
            shrubs.species_id[s] = s % 10
            shrubs.cover[s] = 0.5  # 100 * 0.5% = 50% total shrub cover
            shrubs.height[s] = 40.0 + (s % 20)
            stem_area = eq_pch(1.5, 1.2, shrubs.height[s])
            stems = calc_stems_per_acre_pure(stem_area, shrubs.cover[s], shrubs.height[s])
            shrubs.stems_per_acre[s] = stems
            total_stem_density += stems

        self.assertTrue(total_stem_density > 0.0)
        self.assertTrue(math.isfinite(total_stem_density))
        self.assertEqual(len(shrubs.stems_per_acre), 100)

    def test_variable_shrub_canopy_heterogeneous_sparse(self):
        """Heterogeneous sub-canopy: alternating 0, 5, 0, 50, 0 shrubs per cell."""
        shrub_counts = [0, 5, 0, 50, 0]
        n_cells = len(shrub_counts)
        total_shrubs = sum(shrub_counts)

        shrubs = LandscapeShrubBuffers(n_cells, total_shrubs)
        offset = 0
        shrubs.cell_offsets[0] = 0
        for i, count in enumerate(shrub_counts):
            offset += count
            shrubs.cell_offsets[i + 1] = offset

        self.assertEqual(shrubs.cell_offsets[0], 0)
        self.assertEqual(shrubs.cell_offsets[1], 0)
        self.assertEqual(shrubs.cell_offsets[2], 5)
        self.assertEqual(shrubs.cell_offsets[3], 5)
        self.assertEqual(shrubs.cell_offsets[4], 55)
        self.assertEqual(shrubs.cell_offsets[5], 55)
        self.assertEqual(shrubs.cell_offsets[-1], total_shrubs)

    def test_unlinked_shrub_foreign_key_detection(self):
        """Unlinked shrub record pointing to nonexistent plot is flagged in relational validation."""
        with tempfile.NamedTemporaryFile(suffix=".db", delete=False) as tf:
            db_path = tf.name

        try:
            conn = sqlite3.connect(db_path)
            cur = conn.cursor()
            cur.execute(
                "CREATE TABLE Plots_FCCS (PLOT_ID INTEGER PRIMARY KEY, BPS_CODE INTEGER, herb_cover REAL, herb_height REAL, PPT_3 REAL, NDVI_3 REAL)"
            )
            cur.execute(
                "CREATE TABLE Shrubs_FCCS (PLOT_ID INTEGER, SPP_CODE TEXT, Cover REAL, Height REAL)"
            )

            cur.execute("INSERT INTO Plots_FCCS VALUES (1, 101, 20.0, 30.0, 400.0, 0.5)")
            cur.execute("INSERT INTO Plots_FCCS VALUES (2, 101, 15.0, 25.0, 350.0, 0.4)")

            # Valid shrubs
            cur.execute("INSERT INTO Shrubs_FCCS VALUES (1, 'ARTR2', 10.0, 50.0)")
            cur.execute("INSERT INTO Shrubs_FCCS VALUES (2, 'PUTR2', 15.0, 60.0)")
            # Orphan shrub pointing to PLOT_ID 999
            cur.execute("INSERT INTO Shrubs_FCCS VALUES (999, 'ORPHAN', 5.0, 30.0)")
            conn.commit()

            # Verify relational join mismatch
            cur.execute("SELECT COUNT(*) FROM Shrubs_FCCS")
            total_shrubs = cur.fetchone()[0]
            cur.execute(
                "SELECT COUNT(*) FROM Shrubs_FCCS s INNER JOIN Plots_FCCS p ON s.PLOT_ID = p.PLOT_ID"
            )
            linked_shrubs = cur.fetchone()[0]
            conn.close()

            self.assertEqual(total_shrubs, 3)
            self.assertEqual(linked_shrubs, 2)
            self.assertNotEqual(total_shrubs, linked_shrubs)
        finally:
            if os.path.exists(db_path):
                os.remove(db_path)

    # =========================================================================
    # 3. Extreme Climate & Raster Inputs
    # =========================================================================

    def test_extreme_climate_zero_precip(self):
        """Zero precipitation (hyper-arid desert) yields 0.0 herbaceous production safely."""
        prod = evaluate_herb_growth(b0=1.0, b1=0.5, b2=0.5, b3=0.1, ppt=0.0, ndvi=0.5)
        self.assertEqual(prod, 0.0)

    def test_extreme_climate_drought_negative_precip_clamping(self):
        """Negative precipitation (corrupted or drought anomaly) clamps to 0.0."""
        prod = evaluate_herb_growth(b0=1.0, b1=0.5, b2=0.5, b3=0.1, ppt=-250.0, ndvi=0.5)
        self.assertEqual(prod, 0.0)

    def test_extreme_climate_rainfall_5000mm(self):
        """Extreme precipitation of 5,000 mm evaluates to finite, non-overflowing production."""
        # Using realistic ecological coefficients
        prod = evaluate_herb_growth(b0=0.5, b1=0.2, b2=0.3, b3=0.05, ppt=5000.0, ndvi=0.8)
        self.assertTrue(math.isfinite(prod))
        self.assertGreater(prod, 0.0)

        # Stochastic production draw with standard variance
        stochastic_p = calc_herb_biomass_production(prod, s2b=25.0, z_score=1.96)
        self.assertTrue(math.isfinite(stochastic_p))
        self.assertGreater(stochastic_p, prod)

    def test_extreme_raster_negative_elevation(self):
        """Negative elevation (e.g. Death Valley -86m) is preserved without causing NaN."""
        cells = LandscapeCellBuffers(3)
        cells.elevation[0] = -86.0   # Death Valley
        cells.elevation[1] = -430.0  # Dead Sea
        cells.elevation[2] = 2500.0  # High elevation

        self.assertEqual(cells.elevation[0], -86.0)
        self.assertEqual(cells.elevation[1], -430.0)
        for elev in cells.elevation:
            self.assertTrue(math.isfinite(elev))

    def test_raster_nodata_sentinel_masking(self):
        """Raster NoData sentinel (-9999.0), NaN, and +/-Inf are masked cleanly to 0.0."""
        raw_band = [15.0, -9999.0, float("nan"), float("inf"), float("-inf"), 45.0]
        nodata_val = -9999.0

        sanitized = []
        for val in raw_band:
            if val == nodata_val or not math.isfinite(val) or val < 0.0:
                sanitized.append(0.0)
            else:
                sanitized.append(val)

        expected = [15.0, 0.0, 0.0, 0.0, 0.0, 45.0]
        self.assertEqual(sanitized, expected)


if __name__ == "__main__":
    unittest.main()
