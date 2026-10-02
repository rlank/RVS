//! Structure-of-Arrays (SoA) representations for contiguous landscape grid cells.
//!
//! Enforces Data-Oriented Design (DoD) per optimization standards, enabling
//! cache-friendly linear memory access and SIMD auto-vectorization across
//! cell plots and shrub sub-canopies.

/// Contiguous Structure-of-Arrays buffers for N landscape cell plots.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct LandscapeCellBuffers {
    pub num_cells: usize,
    pub shrub_cover: Vec<f64>,
    pub shrub_height: Vec<f64>,
    pub herb_cover: Vec<f64>,
    pub herb_height: Vec<f64>,
    pub herb_biomass: Vec<f64>,
    pub ndvi: Vec<f64>,
    pub precip: Vec<f64>,
    pub elevation: Vec<f64>,
}

impl LandscapeCellBuffers {
    /// Creates a new `LandscapeCellBuffers` initialized with zeros for `num_cells`.
    pub fn new(num_cells: usize) -> Self {
        Self {
            num_cells,
            shrub_cover: vec![0.0; num_cells],
            shrub_height: vec![0.0; num_cells],
            herb_cover: vec![0.0; num_cells],
            herb_height: vec![0.0; num_cells],
            herb_biomass: vec![0.0; num_cells],
            ndvi: vec![0.0; num_cells],
            precip: vec![0.0; num_cells],
            elevation: vec![0.0; num_cells],
        }
    }

    /// Returns the number of cells in the buffer.
    #[inline]
    pub fn len(&self) -> usize {
        self.num_cells
    }

    /// Returns whether the buffer contains zero cells.
    #[inline]
    pub fn is_empty(&self) -> bool {
        self.num_cells == 0
    }

    /// Returns a zero-copy mutable slice view over the cell arrays.
    pub fn view_mut(&mut self) -> LandscapeCellBuffersMutView<'_> {
        LandscapeCellBuffersMutView {
            num_cells: self.num_cells,
            shrub_cover: &mut self.shrub_cover,
            shrub_height: &mut self.shrub_height,
            herb_cover: &mut self.herb_cover,
            herb_height: &mut self.herb_height,
            herb_biomass: &mut self.herb_biomass,
            ndvi: &self.ndvi,
            precip: &self.precip,
            elevation: &self.elevation,
        }
    }
}

/// Zero-copy mutable slice view for batch execution across N cells.
pub struct LandscapeCellBuffersMutView<'a> {
    pub num_cells: usize,
    pub shrub_cover: &'a mut [f64],
    pub shrub_height: &'a mut [f64],
    pub herb_cover: &'a mut [f64],
    pub herb_height: &'a mut [f64],
    pub herb_biomass: &'a mut [f64],
    pub ndvi: &'a [f64],
    pub precip: &'a [f64],
    pub elevation: &'a [f64],
}

/// Segmented CSR (Compressed Sparse Row) representation for variable shrub sub-canopies.
///
/// For cell $i \in [0, N)$, its associated shrubs reside in index range
/// `cell_offsets[i]..cell_offsets[i+1]`.
#[derive(Debug, Clone, PartialEq)]
pub struct LandscapeShrubBuffers {
    pub num_cells: usize,
    pub num_shrubs: usize,
    pub cell_offsets: Vec<usize>, // Length N + 1
    pub species_id: Vec<u32>,
    pub cover: Vec<f64>,
    pub height: Vec<f64>,
    pub biomass: Vec<f64>,
    pub stems_per_acre: Vec<f64>,
}

impl Default for LandscapeShrubBuffers {
    fn default() -> Self {
        Self::new(0, 0)
    }
}

impl LandscapeShrubBuffers {
    /// Creates a new `LandscapeShrubBuffers` with allocated capacity.
    pub fn new(num_cells: usize, num_shrubs: usize) -> Self {
        Self {
            num_cells,
            num_shrubs,
            cell_offsets: vec![0; num_cells + 1],
            species_id: vec![0; num_shrubs],
            cover: vec![0.0; num_shrubs],
            height: vec![0.0; num_shrubs],
            biomass: vec![0.0; num_shrubs],
            stems_per_acre: vec![0.0; num_shrubs],
        }
    }

    /// Returns the shrub slice range for a given cell index.
    #[inline]
    pub fn shrub_range_for_cell(&self, cell_idx: usize) -> std::ops::Range<usize> {
        assert_eq!(
            self.cell_offsets.len(),
            self.num_cells + 1,
            "cell_offsets length must equal num_cells + 1"
        );
        assert!(
            cell_idx < self.num_cells,
            "Cell index out of bounds: {} >= {}",
            cell_idx,
            self.num_cells
        );
        let start = self.cell_offsets[cell_idx];
        let end = self.cell_offsets[cell_idx + 1];
        assert!(
            start <= end,
            "CSR start offset ({}) exceeds end offset ({})",
            start,
            end
        );
        assert!(
            end <= self.num_shrubs,
            "CSR end offset ({}) exceeds num_shrubs ({})",
            end,
            self.num_shrubs
        );
        start..end
    }

    /// Validates the 5 canonical CSR (Compressed Sparse Row) invariants:
    ///
    /// 1. $\operatorname{len}(\text{cell\_offsets}) = N + 1 = \text{num\_cells} + 1$
    /// 2. $\text{cell\_offsets}[0] = 0$
    /// 3. $\forall i \in [0, N-1], \quad \text{cell\_offsets}[i] \le \text{cell\_offsets}[i+1]$
    /// 4. $\text{cell\_offsets}[N] = M = \text{num\_shrubs}$
    /// 5. $\forall \text{array} \in \{\text{species\_id}, \text{cover}, \text{height}, \text{biomass}, \text{stems\_per\_acre}\}, \; \operatorname{len}(\text{array}) = M$
    pub fn validate(&self) -> Result<(), String> {
        // Invariant 1: cell_offsets length must equal num_cells + 1
        if self.cell_offsets.len() != self.num_cells + 1 {
            return Err(format!(
                "CSR invariant 1 violated: cell_offsets length ({}) must equal num_cells + 1 ({})",
                self.cell_offsets.len(),
                self.num_cells + 1
            ));
        }

        // Invariant 2: cell_offsets[0] must equal 0
        if self.cell_offsets[0] != 0 {
            return Err(format!(
                "CSR invariant 2 violated: cell_offsets[0] ({}) must equal 0",
                self.cell_offsets[0]
            ));
        }

        // Invariant 3: Monotonic non-decreasing offsets
        for (i, window) in self.cell_offsets.windows(2).enumerate() {
            if window[0] > window[1] {
                return Err(format!(
                    "CSR invariant 3 violated: cell_offsets[{}] ({}) > cell_offsets[{}] ({}) (must be non-decreasing)",
                    i,
                    window[0],
                    i + 1,
                    window[1]
                ));
            }
        }

        // Invariant 4: Final offset must equal num_shrubs
        if self.cell_offsets[self.num_cells] != self.num_shrubs {
            return Err(format!(
                "CSR invariant 4 violated: cell_offsets[num_cells] ({}) must equal num_shrubs ({})",
                self.cell_offsets[self.num_cells],
                self.num_shrubs
            ));
        }

        // Invariant 5: Attribute buffer lengths must equal num_shrubs
        if self.species_id.len() != self.num_shrubs {
            return Err(format!(
                "CSR invariant 5 violated: species_id length ({}) must equal num_shrubs ({})",
                self.species_id.len(),
                self.num_shrubs
            ));
        }
        if self.cover.len() != self.num_shrubs {
            return Err(format!(
                "CSR invariant 5 violated: cover length ({}) must equal num_shrubs ({})",
                self.cover.len(),
                self.num_shrubs
            ));
        }
        if self.height.len() != self.num_shrubs {
            return Err(format!(
                "CSR invariant 5 violated: height length ({}) must equal num_shrubs ({})",
                self.height.len(),
                self.num_shrubs
            ));
        }
        if self.biomass.len() != self.num_shrubs {
            return Err(format!(
                "CSR invariant 5 violated: biomass length ({}) must equal num_shrubs ({})",
                self.biomass.len(),
                self.num_shrubs
            ));
        }
        if self.stems_per_acre.len() != self.num_shrubs {
            return Err(format!(
                "CSR invariant 5 violated: stems_per_acre length ({}) must equal num_shrubs ({})",
                self.stems_per_acre.len(),
                self.num_shrubs
            ));
        }

        Ok(())
    }

    /// Returns a slice of shrub canopy cover values for a given cell.
    #[inline]
    pub fn cell_shrub_cover_slice(&self, cell_idx: usize) -> &[f64] {
        let range = self.shrub_range_for_cell(cell_idx);
        &self.cover[range]
    }

    /// Returns a slice of shrub crown height values for a given cell.
    #[inline]
    pub fn cell_shrub_height_slice(&self, cell_idx: usize) -> &[f64] {
        let range = self.shrub_range_for_cell(cell_idx);
        &self.height[range]
    }

    /// Returns a slice of shrub biomass values for a given cell.
    #[inline]
    pub fn cell_shrub_biomass_slice(&self, cell_idx: usize) -> &[f64] {
        let range = self.shrub_range_for_cell(cell_idx);
        &self.biomass[range]
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_csr_validation_success() {
        let mut shrubs = LandscapeShrubBuffers::new(3, 4);
        shrubs.cell_offsets = vec![0, 1, 3, 4];
        assert!(shrubs.validate().is_ok());
    }

    #[test]
    fn test_csr_validation_invariants() {
        // Invariant 1: length mismatch
        let mut s1 = LandscapeShrubBuffers::new(2, 2);
        s1.cell_offsets = vec![0, 2]; // len 2, expected 3
        assert!(s1.validate().is_err());

        // Invariant 2: cell_offsets[0] != 0
        let mut s2 = LandscapeShrubBuffers::new(2, 2);
        s2.cell_offsets = vec![1, 1, 2];
        assert!(s2.validate().is_err());

        // Invariant 3: non-monotonic
        let mut s3 = LandscapeShrubBuffers::new(2, 2);
        s3.cell_offsets = vec![0, 2, 1];
        assert!(s3.validate().is_err());

        // Invariant 4: last offset != num_shrubs
        let mut s4 = LandscapeShrubBuffers::new(2, 3);
        s4.cell_offsets = vec![0, 1, 2]; // last is 2, expected 3
        assert!(s4.validate().is_err());

        // Invariant 5: attribute buffer lengths
        let mut s5 = LandscapeShrubBuffers::new(2, 2);
        s5.cell_offsets = vec![0, 1, 2];
        s5.cover.pop();
        assert!(s5.validate().is_err());
    }

    #[test]
    fn test_shrub_slice_helpers() {
        let mut shrubs = LandscapeShrubBuffers::new(2, 3);
        shrubs.cell_offsets = vec![0, 2, 3];
        shrubs.cover = vec![10.0, 20.0, 30.0];
        shrubs.height = vec![50.0, 60.0, 70.0];
        shrubs.biomass = vec![100.0, 200.0, 300.0];

        assert_eq!(shrubs.cell_shrub_cover_slice(0), &[10.0, 20.0]);
        assert_eq!(shrubs.cell_shrub_cover_slice(1), &[30.0]);
        assert_eq!(shrubs.cell_shrub_height_slice(0), &[50.0, 60.0]);
        assert_eq!(shrubs.cell_shrub_height_slice(1), &[70.0]);
        assert_eq!(shrubs.cell_shrub_biomass_slice(0), &[100.0, 200.0]);
        assert_eq!(shrubs.cell_shrub_biomass_slice(1), &[300.0]);
    }
}
