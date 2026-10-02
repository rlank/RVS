"""
Tier 3 - Combination 3: Disturbance & Fuels Interaction.
Verifies wildfire biomass consumption, shrub mortality top-kill, post-fire resprout,
and subsequent fuelbed reallocation.
"""

import unittest

from tests.rvs_math_oracle import (
    calc_fire_biomass_consumption,
    calc_fire_herb_post_regrowth,
    calc_fire_shrub_mortality,
    calc_fuel_component_fl,
    calc_fuel_percentages,
    calc_graze_biomass_reduction,
    calc_total_1hr_fuel,
)


class TestDisturbanceFuelsInteraction(unittest.TestCase):
    def test_fire_depletes_available_fuelbed(self):
        """Wildfire consumes biomass, reducing subsequent 1-hr fuel loading."""
        pre_b = 1000.0
        consumed = calc_fire_biomass_consumption(pre_b, severity_k=0.08, fire_intensity=25.0)
        remaining_b = pre_b - consumed
        self.assertLess(remaining_b, pre_b)

        # Pre-fire fuel load vs post-fire fuel load
        pre_fuel = calc_total_1hr_fuel(wb=pre_b * 0.2, fl=pre_b * 0.1, herb=300.0)
        post_fuel = calc_total_1hr_fuel(wb=remaining_b * 0.2, fl=remaining_b * 0.1, herb=50.0)
        self.assertLess(post_fuel, pre_fuel)

    def test_shrub_mortality_resets_canopy_fuels(self):
        """High scorch percentage induces high mortality, diminishing foliar fuel components."""
        mortality = calc_fire_shrub_mortality(scorch_pct=95.0, alpha=-0.5, beta=0.05)
        self.assertGreater(mortality, 0.9)

        surviving_shrub_b = 500.0 * (1.0 - mortality)
        post_fl = calc_fuel_component_fl(surviving_shrub_b, proportion=0.25)
        self.assertLess(post_fl, 15.0)

    def test_post_fire_fuel_size_class_reallocation(self):
        """Fuel size class percentages shift after fine fuels are consumed faster than 1000-hr logs."""
        # Pre-fire: fine fuels abundant
        p1_pre, _, _, p1000_pre, _ = calc_fuel_percentages(200.0, 150.0, 300.0, 500.0)

        # Fire burns 90% of 1-hr, but only 10% of 1000-hr
        f1_post = 200.0 * 0.10
        f1000_post = 500.0 * 0.90
        p1_post, _, _, p1000_post, _ = calc_fuel_percentages(f1_post, 100.0, 250.0, f1000_post)

        # 1-hr percentage drops, 1000-hr percentage increases
        self.assertLess(p1_post, p1_pre)
        self.assertGreater(p1000_post, p1000_pre)

    def test_grazing_utilization_reduces_fine_fuels(self):
        """Herbaceous grazing reduces fine fuel load before fire season."""
        pre_herb_b = 400.0
        post_herb_b = calc_graze_biomass_reduction(pre_herb_b, utilization_pct=60.0)
        self.assertEqual(post_herb_b, 160.0)

        fuel_ungrazed = calc_total_1hr_fuel(wb=20.0, fl=10.0, herb=pre_herb_b * 0.8)
        fuel_grazed = calc_total_1hr_fuel(wb=20.0, fl=10.0, herb=post_herb_b * 0.8)
        self.assertLess(fuel_grazed, fuel_ungrazed)

    def test_post_fire_resprout_rebuilds_fine_fuel(self):
        """Herbaceous resprout after fire provides immediate post-fire fine fuel foundation."""
        resprout = calc_fire_herb_post_regrowth(resprout_pot=300.0, mortality_pct=30.0)
        self.assertEqual(resprout, 210.0)
        fine_fuel = calc_total_1hr_fuel(wb=0.0, fl=0.0, herb=resprout * 0.5)
        self.assertEqual(fine_fuel, 105.0)


if __name__ == "__main__":
    unittest.main()
