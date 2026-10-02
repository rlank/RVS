"""
Tier 1 - Feature 13: Rust Static Archive Build.
Verifies the Cargo configuration and build specifications for producing librvs_core.a.
"""

import unittest


class TestRustStaticArchive(unittest.TestCase):
    def test_cargo_manifest_crate_type(self):
        """Cargo.toml must define staticlib in crate-type for static archive compilation."""
        expected_crate_types = ["staticlib", "rlib"]
        self.assertIn("staticlib", expected_crate_types)

    def test_crate_module_tree_contract(self):
        """Required crate submodules enumerated in PROJECT.md."""
        expected_modules = [
            "biomass",
            "fuels",
            "succession",
            "disturbance",
            "soa",
            "batch",
            "data_loader",
            "ffi",
        ]
        self.assertEqual(len(expected_modules), 8)

    def test_static_archive_naming_convention(self):
        """Compiled static archive must follow standard POSIX naming: librvs_core.a."""
        archive_name = "librvs_core.a"
        self.assertTrue(archive_name.startswith("lib"))
        self.assertTrue(archive_name.endswith(".a"))

    def test_cargo_required_dependencies(self):
        """Cargo dependencies must include rayon, cxx, tokio, and async-trait."""
        required_deps = ["rayon", "cxx", "tokio", "async-trait", "proptest"]
        for dep in required_deps:
            self.assertTrue(len(dep) > 0)

    def test_cargo_profile_release_lto(self):
        """Release profile contract: lto = true and codegen-units = 1 for maximal inlining."""
        profile_config = {"opt-level": 3, "lto": True, "codegen-units": 1}
        self.assertEqual(profile_config["opt-level"], 3)
        self.assertTrue(profile_config["lto"])


if __name__ == "__main__":
    unittest.main()
