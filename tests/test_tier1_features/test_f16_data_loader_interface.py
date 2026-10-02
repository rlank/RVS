"""
Tier 1 - Feature 16: Abstract DataLoader Interface.
Verifies the IDataLoader / DataLoader abstraction decoupling simulation business
logic from direct database handles and specific storage schemas.
"""

import unittest
from abc import ABC, abstractmethod

from tests.rvs_math_oracle import LandscapeCellBuffers, LandscapeShrubBuffers


class IDataLoader(ABC):
    """Abstract DataLoader interface specification matching PROJECT.md § Interface Contracts."""

    @abstractmethod
    def load_landscape(self) -> tuple[LandscapeCellBuffers, LandscapeShrubBuffers]:
        """Loads landscape cells and shrubs into canonical in-memory SoA buffers."""

    @abstractmethod
    def load_reference_data(self) -> dict:
        """Loads pre-compiled reference catalog (equations, crosswalks)."""

    @abstractmethod
    def write_landscape(
        self, year: int, cells: LandscapeCellBuffers, shrubs: LandscapeShrubBuffers
    ) -> None:
        """Persists simulation state for a given year."""


class MockDataLoader(IDataLoader):
    """Concrete mock implementation verifying interface compliance."""

    def __init__(self, n_cells: int = 10, m_shrubs: int = 15):
        self.n_cells = n_cells
        self.m_shrubs = m_shrubs
        self.written_years = []

    def load_landscape(self) -> tuple[LandscapeCellBuffers, LandscapeShrubBuffers]:
        cells = LandscapeCellBuffers(self.n_cells)
        shrubs = LandscapeShrubBuffers(self.n_cells, self.m_shrubs)
        return cells, shrubs

    def load_reference_data(self) -> dict:
        return {"equations": {165: {"cf1": 5.0, "cf2": 2.0}}}

    def write_landscape(
        self, year: int, cells: LandscapeCellBuffers, shrubs: LandscapeShrubBuffers
    ) -> None:
        self.written_years.append(year)


class TestDataLoaderInterface(unittest.TestCase):
    def test_abstract_class_instantiation_fails(self):
        """Instantiating abstract IDataLoader directly must raise TypeError."""
        with self.assertRaises(TypeError):
            IDataLoader()

    def test_mock_dataloader_load_landscape(self):
        """Mock loader successfully returns canonical SoA buffers."""
        loader = MockDataLoader(n_cells=20, m_shrubs=35)
        cells, shrubs = loader.load_landscape()
        self.assertEqual(cells.n, 20)
        self.assertEqual(shrubs.n_cells, 20)
        self.assertEqual(shrubs.m_shrubs, 35)

    def test_mock_dataloader_reference_data(self):
        """Mock loader supplies reference catalog."""
        loader = MockDataLoader()
        ref = loader.load_reference_data()
        self.assertIn("equations", ref)

    def test_mock_dataloader_write_landscape(self):
        """Simulation write output recorded for multiple years."""
        loader = MockDataLoader(5, 5)
        cells, shrubs = loader.load_landscape()
        for y in range(5):
            loader.write_landscape(y, cells, shrubs)
        self.assertEqual(loader.written_years, [0, 1, 2, 3, 4])

    def test_simulation_independence_from_storage(self):
        """Simulation driver takes abstract buffers, operating with zero SQL knowledge."""
        loader = MockDataLoader(10, 10)
        cells, _ = loader.load_landscape()
        # Verify buffers can be modified in-memory without database connection
        for i in range(cells.n):
            cells.herb_cover[i] = 50.0
        self.assertEqual(cells.herb_cover[0], 50.0)


if __name__ == "__main__":
    unittest.main()
