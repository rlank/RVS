"""
Tier 4 - Application 2: Spatial Raster GeoTIFF Landscape Ingestion & Simulation.
Simulates multi-band GeoTIFF landscape ingestion (NDVI, PPT, Elevation, Cover, Height)
and multi-decade raster grid simulation execution.
"""

import unittest

from tests.rvs_math_oracle import (
    LandscapeCellBuffers,
    calc_attenuation,
    calc_growth_rate,
    calc_total_1hr_fuel,
    evaluate_herb_growth,
)


class TestApp02SpatialRasterGeoTIFF(unittest.TestCase):
    def setUp(self):
        # 10x10 synthetic landscape extent (100 cells)
        self.rows = 10
        self.cols = 10
        self.n_cells = self.rows * self.cols

        # Generate synthetic raster bands
        self.raster_bands = {
            "elevation": [1000.0 + r * 50.0 for r in range(self.rows) for _ in range(self.cols)],
            "ppt": [300.0 + c * 20.0 for _ in range(self.rows) for c in range(self.cols)],
            "ndvi": [0.2 + (r + c) * 0.03 for r in range(self.rows) for c in range(self.cols)],
            "herb_cover": [20.0 + (r * 3.0) for r in range(self.rows) for _ in range(self.cols)],
            "herb_height": [15.0 + (c * 2.0) for _ in range(self.rows) for c in range(self.cols)],
            "shrub_cover": [10.0 + r for r in range(self.rows) for _ in range(self.cols)],
            "shrub_height": [40.0 + c for _ in range(self.rows) for c in range(self.cols)],
        }

    def test_geotiff_multi_band_ingestion(self):
        """Flattens 2D multi-band raster arrays into 1D contiguous LandscapeCellBuffers."""
        cells = LandscapeCellBuffers(self.n_cells)
        for i in range(self.n_cells):
            cells.plot_id[i] = i + 1
            cells.elevation[i] = self.raster_bands["elevation"][i]
            cells.precip[i] = self.raster_bands["ppt"][i]
            cells.ndvi[i] = self.raster_bands["ndvi"][i]
            cells.herb_cover[i] = self.raster_bands["herb_cover"][i]
            cells.herb_height[i] = self.raster_bands["herb_height"][i]
            cells.shrub_cover[i] = self.raster_bands["shrub_cover"][i]
            cells.shrub_height[i] = self.raster_bands["shrub_height"][i]

        self.assertEqual(cells.n, 100)
        self.assertEqual(cells.elevation[0], 1000.0)
        self.assertEqual(cells.precip[0], 300.0)

    def test_geotiff_grid_simulation_run(self):
        """Executes 5-year simulation across the 100-cell raster landscape."""
        cells = LandscapeCellBuffers(self.n_cells)
        for i in range(self.n_cells):
            cells.precip[i] = self.raster_bands["ppt"][i]
            cells.ndvi[i] = self.raster_bands["ndvi"][i]
            cells.herb_cover[i] = self.raster_bands["herb_cover"][i]
            cells.herb_height[i] = self.raster_bands["herb_height"][i]
            cells.shrub_cover[i] = self.raster_bands["shrub_cover"][i]
            cells.shrub_height[i] = self.raster_bands["shrub_height"][i]

        # 5-year simulation loop
        for year in range(5):
            for i in range(self.n_cells):
                # 1. Climate production
                base_prod = evaluate_herb_growth(
                    b0=1.5, b1=0.35, b2=0.45, b3=0.05, ppt=cells.precip[i], ndvi=cells.ndvi[i]
                )
                # 2. Attenuation
                cells.herb_biomass[i] = calc_attenuation(
                    base_prod,
                    cells.herb_cover[i],
                    cells.herb_height[i],
                    cells.shrub_cover[i],
                    cells.shrub_height[i],
                )
                # 3. Fuels
                cells.fuel_1hr[i] = calc_total_1hr_fuel(
                    wb=cells.shrub_cover[i] * 0.2,
                    fl=cells.shrub_cover[i] * 0.1,
                    herb=cells.herb_biomass[i] * 0.7,
                )
                # 4. Succession
                cells.herb_height[i] = calc_growth_rate(cells.herb_height[i], 1.2)

        # Confirm all 100 cells produced valid positive outputs
        for i in range(self.n_cells):
            self.assertGreater(cells.herb_biomass[i], 0.0)
            self.assertGreater(cells.fuel_1hr[i], 0.0)

    def test_spatial_cell_isolation(self):
        """Cell operations do not leak into or alias neighboring pixel memory."""
        cells = LandscapeCellBuffers(self.n_cells)
        cells.herb_cover[15] = 99.0
        self.assertEqual(cells.herb_cover[14], 0.0)
        self.assertEqual(cells.herb_cover[16], 0.0)

    def test_raster_output_grid_reconstruction(self):
        """Simulated results in 1D array fold back accurately into original (rows, cols) shape."""
        cells = LandscapeCellBuffers(self.n_cells)
        for i in range(self.n_cells):
            cells.fuel_1hr[i] = float(i)

        # Unflatten to 2D grid
        grid_2d = []
        for r in range(self.rows):
            row_vals = [cells.fuel_1hr[r * self.cols + c] for c in range(self.cols)]
            grid_2d.append(row_vals)

        self.assertEqual(len(grid_2d), self.rows)
        self.assertEqual(len(grid_2d[0]), self.cols)
        self.assertEqual(grid_2d[4][7], float(4 * 10 + 7))

    def test_raster_heterogeneous_climate_gradient(self):
        """Precipitation gradient across raster columns induces corresponding biomass gradient."""
        cells = LandscapeCellBuffers(self.cols)
        for c in range(self.cols):
            ppt = 200.0 + c * 80.0
            ndvi = 0.3 + c * 0.05
            cells.herb_biomass[c] = evaluate_herb_growth(1.5, 0.4, 0.5, 0.05, ppt, ndvi)

        # Higher precipitation/NDVI columns have strictly higher biomass
        for c in range(self.cols - 1):
            self.assertLess(cells.herb_biomass[c], cells.herb_biomass[c + 1])


if __name__ == "__main__":
    unittest.main()
