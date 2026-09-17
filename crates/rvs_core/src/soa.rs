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
}
