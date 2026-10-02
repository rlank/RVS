"""
Tier 3 - Combination 2: Succession & Biomass Feedback.
Verifies annual succession progression, climate-driven growth, holdover history,
and canopy competition attenuation feedback loops.
"""

import unittest

from tests.rvs_math_oracle import (
    calc_attenuation,
    calc_herb_biomass_production,
    calc_herb_holdover,
    calc_s2b_quadratic_form,
    evaluate_herb_growth,
    scale_shrub_cover,
    scale_shrub_height,
)


class TestSuccessionBiomassFeedback(unittest.TestCase):
    def test_climate_growth_drives_biomass_production(self):
        """Climate drivers (PPT, NDVI) produce mean production which feeds stochastic draw."""
        # Step 1: Climate production
        mean_p = evaluate_herb_growth(b0=2.0, b1=0.3, b2=0.5, b3=0.05, ppt=400.0, ndvi=0.5)
        self.assertGreater(mean_p, 0.0)

        # Step 2: Stochastic uncertainty
        s2b = calc_s2b_quadratic_form([1.0, 0.5], [100.0, 10.0, 10.0, 50.0], 2)
        stoch_prod = calc_herb_biomass_production(mean_p, s2b, z_score=0.5)
        self.assertGreater(stoch_prod, 0.0)

    def test_succession_height_growth_scales_competition(self):
        """Annual shrub height and cover growth shifts competitive balance against herbs."""
        initial_shrub_ht = 40.0
        initial_shrub_cov = 25.0
        herb_ht = 20.0
        herb_cov = 30.0
        herb_b = 400.0

        # Year 0 attenuation
        att_y0 = calc_attenuation(herb_b, herb_cov, herb_ht, initial_shrub_cov, initial_shrub_ht)

        # Shrub grows over 5 years
        shrub_ht_y5 = scale_shrub_height(initial_shrub_ht, delta_ht=15.0)
        shrub_cov_y5 = scale_shrub_cover(initial_shrub_cov, delta_cov=20.0)

        # Year 5 attenuation
        att_y5 = calc_attenuation(herb_b, herb_cov, herb_ht, shrub_cov_y5, shrub_ht_y5)

        self.assertLess(att_y5, att_y0)

    def test_annual_biomass_holdover_pipeline(self):
        """Sequential annual production updates 3-year rolling holdover."""
        p_t1 = 500.0
        p_t2 = 450.0
        p_t3 = 600.0
        holdover = calc_herb_holdover(p_t1, p_t2, p_t3)
        expected = 500.0 * 0.3 + 450.0 * 0.1 + 600.0 * 0.05
        self.assertAlmostEqual(holdover, expected, places=4)

    def test_drought_scenario_production_depression(self):
        """Severe drought (low PPT and low NDVI) severely depresses herb production."""
        normal_prod = evaluate_herb_growth(2.0, 0.4, 0.6, 0.05, ppt=500.0, ndvi=0.6)
        drought_prod = evaluate_herb_growth(2.0, 0.4, 0.6, 0.05, ppt=150.0, ndvi=0.2)
        self.assertLess(drought_prod, normal_prod * 0.5)

    def test_holdover_buffering_during_drought(self):
        """Standing dead holdover from previous wet years buffers total available biomass in drought year."""
        wet_p1 = 800.0
        wet_p2 = 900.0
        wet_p3 = 750.0
        holdover = calc_herb_holdover(wet_p1, wet_p2, wet_p3)
        drought_current = 150.0
        total_available = drought_current + holdover
        self.assertGreater(total_available, drought_current * 2.0)


if __name__ == "__main__":
    unittest.main()
