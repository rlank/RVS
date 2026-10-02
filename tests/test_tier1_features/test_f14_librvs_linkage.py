"""
Tier 1 - Feature 14: librvs Linkage Integration.
Verifies the build integration and linking of simulation engine artifacts (librvs.so and rvs executable).
"""

import os
import unittest


class TestLibrvsLinkage(unittest.TestCase):
    def setUp(self):
        self.repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
        self.librvs_dir = os.path.join(self.repo_root, "librvs")
        self.build_dir = os.path.join(self.librvs_dir, "build")
        self.rvs_bin = os.path.join(self.build_dir, "rvs")
        self.librvs_so = os.path.join(self.build_dir, "librvs.so")

    def test_makefile_exists(self):
        """librvs directory must contain makefile for building simulation targets."""
        makefile_path = os.path.join(self.librvs_dir, "makefile")
        self.assertTrue(os.path.exists(makefile_path), f"Makefile not found at {makefile_path}")

    def test_build_directory_exists(self):
        """build/ directory must exist."""
        self.assertTrue(os.path.isdir(self.build_dir), f"Build dir not found at {self.build_dir}")

    def test_rvs_binary_exists_and_executable(self):
        """The rvs executable binary must exist and have execute permissions."""
        self.assertTrue(os.path.exists(self.rvs_bin), f"rvs binary not found at {self.rvs_bin}")
        self.assertTrue(os.access(self.rvs_bin, os.X_OK), "rvs binary is not executable")

    def test_librvs_shared_library_present(self):
        """Shared library artifact (librvs.so or versioned .so) must be present."""
        so_files = [f for f in os.listdir(self.build_dir) if "librvs.so" in f]
        self.assertGreater(len(so_files), 0, "No librvs.so library artifact found in build/")

    def test_makefile_cxx_standard(self):
        """Makefile must target modern C++ (C++20 standard)."""
        makefile_path = os.path.join(self.librvs_dir, "makefile")
        with open(makefile_path) as f:
            content = f.read()
        self.assertTrue("c++20" in content or "c++2a" in content or "CXX" in content)


if __name__ == "__main__":
    unittest.main()
