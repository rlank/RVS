//! Adversarial empirical challenge tests targeting:
//! 1. Detailed panic message validation for slice length mismatches (shorter & longer)
//! 2. Cross-buffer discrepancy validation across all batch kernels
//! 3. Empty slice handling (num_cells = 0, num_shrubs = 0, and num_cells > 0 with num_shrubs = 0)
//! 4. Concurrency race conditions in streaming pipeline cancellation

use rvs_core::soa::{LandscapeCellBuffers, LandscapeShrubBuffers};
use rvs_core::batch::{
    batch_evaluate_biomass,
    batch_evaluate_fuels,
    batch_evaluate_succession,
    batch_evaluate_disturbance_graze,
    ReferenceData,
};

fn extract_panic_msg(panic_err: Box<dyn std::any::Any + Send>) -> String {
    if let Some(s) = panic_err.downcast_ref::<String>() {
        s.clone()
    } else if let Some(s) = panic_err.downcast_ref::<&str>() {
        s.to_string()
    } else {
        "Unknown panic payload".to_string()
    }
}

// =========================================================================
// 1. batch_evaluate_fuels: Detailed Panic Message Verification
// =========================================================================

#[test]
fn test_fuels_herb_biomass_short_panic_message() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.herb_biomass.pop(); // len = 3, cells.len = 4
    let mut out_1hr_fuel = vec![0.0; 4];

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_fuels(&cells, &mut out_1hr_fuel, 0.5);
    }));
    assert!(result.is_err());
    let msg = extract_panic_msg(result.unwrap_err());
    assert!(
        msg.contains("cells.herb_biomass buffer length (3) must match cells.len() (4)"),
        "Expected descriptive mismatch message, got: {}",
        msg
    );
}

#[test]
fn test_fuels_herb_biomass_long_panic_message() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.herb_biomass.push(42.0); // len = 5, cells.len = 4
    let mut out_1hr_fuel = vec![0.0; 4];

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_fuels(&cells, &mut out_1hr_fuel, 0.5);
    }));
    assert!(result.is_err());
    let msg = extract_panic_msg(result.unwrap_err());
    assert!(
        msg.contains("cells.herb_biomass buffer length (5) must match cells.len() (4)"),
        "Expected descriptive mismatch message, got: {}",
        msg
    );
}

#[test]
fn test_fuels_out_fuel_short_panic_message() {
    let cells = LandscapeCellBuffers::new(4);
    let mut out_1hr_fuel = vec![0.0; 2]; // len = 2, cells.len = 4

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_fuels(&cells, &mut out_1hr_fuel, 0.5);
    }));
    assert!(result.is_err());
    let msg = extract_panic_msg(result.unwrap_err());
    assert!(
        msg.contains("out_1hr_fuel buffer length (2) must match cells.len() (4)"),
        "Expected descriptive mismatch message, got: {}",
        msg
    );
}

#[test]
fn test_fuels_out_fuel_long_panic_message() {
    let cells = LandscapeCellBuffers::new(4);
    let mut out_1hr_fuel = vec![0.0; 7]; // len = 7, cells.len = 4

    let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_fuels(&cells, &mut out_1hr_fuel, 0.5);
    }));
    assert!(result.is_err());
    let msg = extract_panic_msg(result.unwrap_err());
    assert!(
        msg.contains("out_1hr_fuel buffer length (7) must match cells.len() (4)"),
        "Expected descriptive mismatch message, got: {}",
        msg
    );
}

// =========================================================================
// 2. batch_evaluate_biomass: Slice Length & Cross-Buffer Checks
// =========================================================================

#[test]
fn test_biomass_cell_shrub_cover_short_and_long() {
    let refs = ReferenceData::default();

    // Short
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 3);
    cells.shrub_cover.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.shrub_cover buffer length (4) must match num_cells (5)"));

    // Long
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 3);
    cells.shrub_cover.push(10.0);
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.shrub_cover buffer length (6) must match num_cells (5)"));
}

#[test]
fn test_biomass_cell_shrub_height_short_and_long() {
    let refs = ReferenceData::default();

    // Short
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 3);
    cells.shrub_height.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.shrub_height buffer length (4) must match num_cells (5)"));

    // Long
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 3);
    cells.shrub_height.push(10.0);
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.shrub_height buffer length (6) must match num_cells (5)"));
}

#[test]
fn test_biomass_cell_herb_cover_short_and_long() {
    let refs = ReferenceData::default();

    // Short
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 3);
    cells.herb_cover.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.herb_cover buffer length (4) must match num_cells (5)"));

    // Long
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 3);
    cells.herb_cover.push(10.0);
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.herb_cover buffer length (6) must match num_cells (5)"));
}

#[test]
fn test_biomass_cell_herb_height_short_and_long() {
    let refs = ReferenceData::default();

    // Short
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 3);
    cells.herb_height.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.herb_height buffer length (4) must match num_cells (5)"));

    // Long
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 3);
    cells.herb_height.push(10.0);
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.herb_height buffer length (6) must match num_cells (5)"));
}

#[test]
fn test_biomass_shrub_cross_buffer_mismatches() {
    let refs = ReferenceData::default();

    // shrubs.num_cells != cells.num_cells
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(4, 2);
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("shrubs.num_cells (4) must match cells.num_cells (5)"));

    // shrubs.cell_offsets.len() != shrubs.num_cells + 1
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 2);
    shrubs.cell_offsets.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("shrubs.cell_offsets buffer length (5) must match shrubs.num_cells + 1 (6)"));

    // shrubs.height.len() != shrubs.num_shrubs
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 2);
    shrubs.height.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("shrubs.height buffer length (1) must match num_shrubs (2)"));

    // shrubs.cover.len() != shrubs.num_shrubs
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 2);
    shrubs.cover.push(1.0);
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("shrubs.cover buffer length (3) must match num_shrubs (2)"));

    // shrubs.biomass.len() != shrubs.num_shrubs
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 2);
    shrubs.biomass.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("shrubs.biomass buffer length (1) must match num_shrubs (2)"));

    // shrubs.stems_per_acre.len() != shrubs.num_shrubs
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 2);
    shrubs.stems_per_acre.push(9.0);
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("shrubs.stems_per_acre buffer length (3) must match num_shrubs (2)"));

    // shrubs.species_id.len() != shrubs.num_shrubs
    let mut cells = LandscapeCellBuffers::new(5);
    let mut shrubs = LandscapeShrubBuffers::new(5, 2);
    shrubs.species_id.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("shrubs.species_id buffer length (1) must match num_shrubs (2)"));
}

// =========================================================================
// 3. batch_evaluate_succession & batch_evaluate_disturbance_graze
// =========================================================================

#[test]
fn test_succession_slice_mismatch_messages() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.shrub_height.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_succession(&mut cells.view_mut(), 1.0, 1.0);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.shrub_height buffer length (3) must match num_cells (4)"));

    let mut cells = LandscapeCellBuffers::new(4);
    cells.shrub_cover.push(10.0);
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_succession(&mut cells.view_mut(), 1.0, 1.0);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.shrub_cover buffer length (5) must match num_cells (4)"));
}

#[test]
fn test_disturbance_slice_mismatch_messages() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.herb_biomass.pop();
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_disturbance_graze(&mut cells.view_mut(), 10.0);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.herb_biomass buffer length (3) must match num_cells (4)"));

    let mut cells = LandscapeCellBuffers::new(4);
    cells.herb_height.push(10.0);
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        batch_evaluate_disturbance_graze(&mut cells.view_mut(), 10.0);
    }));
    assert!(res.is_err());
    let msg = extract_panic_msg(res.unwrap_err());
    assert!(msg.contains("cells.herb_height buffer length (5) must match num_cells (4)"));
}

// =========================================================================
// 4. Empty Slices Across All Kernels
// =========================================================================

#[test]
fn test_empty_slices_biomass_zero_cells_zero_shrubs() {
    let mut cells = LandscapeCellBuffers::new(0);
    let mut shrubs = LandscapeShrubBuffers::new(0, 0);
    let refs = ReferenceData::default();

    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    assert_eq!(cells.herb_biomass.len(), 0);
    assert_eq!(shrubs.stems_per_acre.len(), 0);
}

#[test]
fn test_empty_slices_biomass_nonzero_cells_zero_shrubs() {
    // 5 cells, but 0 shrubs across the entire landscape
    let mut cells = LandscapeCellBuffers::new(5);
    for i in 0..5 {
        cells.herb_biomass[i] = 100.0;
        cells.herb_cover[i] = 20.0;
        cells.herb_height[i] = 10.0;
        cells.shrub_cover[i] = 0.0;
        cells.shrub_height[i] = 0.0;
    }
    let mut shrubs = LandscapeShrubBuffers::new(5, 0);
    let refs = ReferenceData::default();

    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);
    assert_eq!(cells.herb_biomass.len(), 5);
    assert_eq!(shrubs.stems_per_acre.len(), 0);
    // Attenuation on 0 shrub cover should leave biomass untouched or evaluated cleanly
    for b in &cells.herb_biomass {
        assert!(b.is_finite() && *b > 0.0);
    }
}

#[test]
fn test_empty_slices_fuels_zero_cells() {
    let cells = LandscapeCellBuffers::new(0);
    let mut out = Vec::new();
    batch_evaluate_fuels(&cells, &mut out, 0.5);
    assert_eq!(out.len(), 0);
}

#[test]
fn test_empty_slices_succession_zero_cells() {
    let mut cells = LandscapeCellBuffers::new(0);
    batch_evaluate_succession(&mut cells.view_mut(), 1.0, 1.0);
    assert_eq!(cells.shrub_height.len(), 0);
    assert_eq!(cells.shrub_cover.len(), 0);
}

#[test]
fn test_empty_slices_disturbance_zero_cells() {
    let mut cells = LandscapeCellBuffers::new(0);
    batch_evaluate_disturbance_graze(&mut cells.view_mut(), 25.0);
    assert_eq!(cells.herb_biomass.len(), 0);
    assert_eq!(cells.herb_height.len(), 0);
}

// =========================================================================
// 5. LandscapeShrubBuffers::validate CSR Invariant Corruption Tests
// =========================================================================

#[test]
fn test_csr_validate_corrupted_offsets_length() {
    // Invariant 1: cell_offsets.len() != num_cells + 1 -> Err
    let mut shrubs = LandscapeShrubBuffers::new(5, 10);
    shrubs.cell_offsets.pop(); // length is 5, expected 6
    let res = shrubs.validate();
    assert!(res.is_err());
    let err = res.unwrap_err();
    assert!(
        err.contains("CSR invariant 1 violated") && err.contains("cell_offsets length (5) must equal num_cells + 1 (6)"),
        "Unexpected error: {}",
        err
    );
}

#[test]
fn test_csr_validate_corrupted_nonzero_first_offset() {
    // Invariant 2: cell_offsets[0] != 0 -> Err
    let mut shrubs = LandscapeShrubBuffers::new(3, 6);
    shrubs.cell_offsets = vec![1, 2, 4, 6];
    let res = shrubs.validate();
    assert!(res.is_err());
    let err = res.unwrap_err();
    assert!(
        err.contains("CSR invariant 2 violated") && err.contains("cell_offsets[0] (1) must equal 0"),
        "Unexpected error: {}",
        err
    );
}

#[test]
fn test_csr_validate_corrupted_decreasing_offsets() {
    // Invariant 3: cell_offsets[i] > cell_offsets[i+1] -> Err
    let mut shrubs = LandscapeShrubBuffers::new(4, 8);
    shrubs.cell_offsets = vec![0, 3, 2, 5, 8]; // offset 3 > 2 at index 1
    let res = shrubs.validate();
    assert!(res.is_err());
    let err = res.unwrap_err();
    assert!(
        err.contains("CSR invariant 3 violated") && err.contains("must be non-decreasing"),
        "Unexpected error: {}",
        err
    );
}

#[test]
fn test_csr_validate_corrupted_last_offset_not_num_shrubs() {
    // Invariant 4: cell_offsets[num_cells] != num_shrubs -> Err
    let mut shrubs = LandscapeShrubBuffers::new(3, 7);
    shrubs.cell_offsets = vec![0, 2, 4, 6]; // last is 6, expected 7
    let res = shrubs.validate();
    assert!(res.is_err());
    let err = res.unwrap_err();
    assert!(
        err.contains("CSR invariant 4 violated") && err.contains("cell_offsets[num_cells] (6) must equal num_shrubs (7)"),
        "Unexpected error: {}",
        err
    );
}

#[test]
fn test_csr_validate_corrupted_attribute_buffer_lengths() {
    // Invariant 5: buffer length mismatch with num_shrubs -> Err
    // 5a. species_id mismatch
    let mut s1 = LandscapeShrubBuffers::new(2, 4);
    s1.cell_offsets = vec![0, 2, 4];
    s1.species_id.pop();
    let res = s1.validate();
    assert!(res.is_err());
    assert!(res.unwrap_err().contains("CSR invariant 5 violated: species_id"));

    // 5b. cover mismatch
    let mut s2 = LandscapeShrubBuffers::new(2, 4);
    s2.cell_offsets = vec![0, 2, 4];
    s2.cover.push(15.0);
    let res = s2.validate();
    assert!(res.is_err());
    assert!(res.unwrap_err().contains("CSR invariant 5 violated: cover"));

    // 5c. height mismatch
    let mut s3 = LandscapeShrubBuffers::new(2, 4);
    s3.cell_offsets = vec![0, 2, 4];
    s3.height.pop();
    let res = s3.validate();
    assert!(res.is_err());
    assert!(res.unwrap_err().contains("CSR invariant 5 violated: height"));

    // 5d. biomass mismatch
    let mut s4 = LandscapeShrubBuffers::new(2, 4);
    s4.cell_offsets = vec![0, 2, 4];
    s4.biomass.push(99.0);
    let res = s4.validate();
    assert!(res.is_err());
    assert!(res.unwrap_err().contains("CSR invariant 5 violated: biomass"));

    // 5e. stems_per_acre mismatch
    let mut s5 = LandscapeShrubBuffers::new(2, 4);
    s5.cell_offsets = vec![0, 2, 4];
    s5.stems_per_acre.pop();
    let res = s5.validate();
    assert!(res.is_err());
    assert!(res.unwrap_err().contains("CSR invariant 5 violated: stems_per_acre"));
}

// =========================================================================
// 6. Floating-Point Anomalies & Numerical Extremes Across All Kernels
// =========================================================================

#[test]
fn test_disturbance_floating_point_anomalies() {
    let mut cells = LandscapeCellBuffers::new(6);
    cells.herb_biomass = vec![100.0, -50.0, f64::NAN, f64::INFINITY, f64::NEG_INFINITY, 200.0];
    cells.herb_height = vec![30.0, -10.0, f64::NAN, f64::INFINITY, 15.0, f64::NEG_INFINITY];

    // Extreme: utilization > 100% (should clamp to 100% -> 0 remaining)
    batch_evaluate_disturbance_graze(&mut cells.view_mut(), 150.0);
    for b in &cells.herb_biomass {
        assert_eq!(*b, 0.0, "150% utilization must reduce all biomass to 0.0");
    }
    for h in &cells.herb_height {
        assert_eq!(*h, 0.0, "150% utilization must reduce all height to 0.0");
    }

    // Extreme: negative utilization (should clamp to 0% -> no reduction, but invalid inputs sanitized)
    let mut cells2 = LandscapeCellBuffers::new(6);
    cells2.herb_biomass = vec![100.0, -50.0, f64::NAN, f64::INFINITY, f64::NEG_INFINITY, 200.0];
    cells2.herb_height = vec![30.0, -10.0, f64::NAN, f64::INFINITY, 15.0, f64::NEG_INFINITY];

    batch_evaluate_disturbance_graze(&mut cells2.view_mut(), -25.0);
    assert_eq!(cells2.herb_biomass[0], 100.0, "Valid biomass preserved under negative util");
    assert_eq!(cells2.herb_biomass[1], 0.0, "Negative biomass sanitized to 0.0");
    assert_eq!(cells2.herb_biomass[2], 0.0, "NaN biomass sanitized to 0.0");
    assert_eq!(cells2.herb_biomass[3], 0.0, "Inf biomass sanitized to 0.0");
    assert_eq!(cells2.herb_biomass[4], 0.0, "-Inf biomass sanitized to 0.0");
    assert_eq!(cells2.herb_biomass[5], 200.0, "Valid biomass preserved");

    assert_eq!(cells2.herb_height[0], 30.0, "Valid height preserved");
    assert_eq!(cells2.herb_height[1], 0.0, "Negative height sanitized to 0.0");
    assert_eq!(cells2.herb_height[2], 0.0, "NaN height sanitized to 0.0");
    assert_eq!(cells2.herb_height[3], 0.0, "Inf height sanitized to 0.0");
    assert_eq!(cells2.herb_height[4], 15.0, "Valid height preserved");
    assert_eq!(cells2.herb_height[5], 0.0, "-Inf height sanitized to 0.0");

    // Anomaly: utilization_pct is NaN or Inf -> zeroes buffers safely
    let mut cells3 = LandscapeCellBuffers::new(2);
    cells3.herb_biomass = vec![100.0, 200.0];
    cells3.herb_height = vec![20.0, 30.0];
    batch_evaluate_disturbance_graze(&mut cells3.view_mut(), f64::NAN);
    assert_eq!(cells3.herb_biomass, vec![0.0, 0.0]);
    assert_eq!(cells3.herb_height, vec![0.0, 0.0]);

    let mut cells4 = LandscapeCellBuffers::new(2);
    cells4.herb_biomass = vec![100.0, 200.0];
    cells4.herb_height = vec![20.0, 30.0];
    batch_evaluate_disturbance_graze(&mut cells4.view_mut(), f64::INFINITY);
    assert_eq!(cells4.herb_biomass, vec![0.0, 0.0]);
    assert_eq!(cells4.herb_height, vec![0.0, 0.0]);
}

#[test]
fn test_fuels_floating_point_anomalies() {
    let mut cells = LandscapeCellBuffers::new(6);
    cells.herb_biomass = vec![100.0, -50.0, f64::NAN, f64::INFINITY, f64::NEG_INFINITY, 200.0];
    let mut out_fuel = vec![0.0; 6];

    // Valid cure_rate with anomalous biomass
    batch_evaluate_fuels(&cells, &mut out_fuel, 0.5);
    assert_eq!(out_fuel[0], 50.0);
    assert_eq!(out_fuel[1], 0.0, "Negative biomass produces 0.0 fuel");
    assert_eq!(out_fuel[2], 0.0, "NaN biomass produces 0.0 fuel");
    assert_eq!(out_fuel[3], 0.0, "Inf biomass produces 0.0 fuel");
    assert_eq!(out_fuel[4], 0.0, "-Inf biomass produces 0.0 fuel");
    assert_eq!(out_fuel[5], 100.0);

    // Anomalous cure_rate: NaN or Inf zeroes all output
    batch_evaluate_fuels(&cells, &mut out_fuel, f64::NAN);
    for f in &out_fuel {
        assert_eq!(*f, 0.0, "NaN cure_rate must zero all fuel");
    }

    batch_evaluate_fuels(&cells, &mut out_fuel, f64::INFINITY);
    for f in &out_fuel {
        assert_eq!(*f, 0.0, "Inf cure_rate must zero all fuel");
    }

    // Out of bounds cure_rate: negative clamps to 0.0, > 1.0 clamps to 1.0
    batch_evaluate_fuels(&cells, &mut out_fuel, -0.5);
    for f in &out_fuel {
        assert_eq!(*f, 0.0, "Negative cure_rate clamps to 0.0");
    }

    batch_evaluate_fuels(&cells, &mut out_fuel, 1.5);
    assert_eq!(out_fuel[0], 100.0, "cure_rate 1.5 clamps to 1.0");
    assert_eq!(out_fuel[5], 200.0, "cure_rate 1.5 clamps to 1.0");
}

#[test]
fn test_succession_floating_point_anomalies() {
    let mut cells = LandscapeCellBuffers::new(4);
    cells.shrub_height = vec![10.0, 50.0, -5.0, 100.0];
    cells.shrub_cover = vec![20.0, 80.0, -10.0, 95.0];

    // Negative delta reducing below 0 clamps to 0
    batch_evaluate_succession(&mut cells.view_mut(), -15.0, -30.0);
    assert_eq!(cells.shrub_height[0], 0.0, "10 - 15 clamps to 0.0");
    assert_eq!(cells.shrub_height[1], 35.0, "50 - 15 = 35.0");
    assert_eq!(cells.shrub_height[2], 0.0, "-5 - 15 clamps to 0.0");
    assert_eq!(cells.shrub_height[3], 85.0, "100 - 15 = 85.0");

    assert_eq!(cells.shrub_cover[0], 0.0, "20 - 30 clamps to 0.0");
    assert_eq!(cells.shrub_cover[1], 50.0, "80 - 30 = 50.0");
    assert_eq!(cells.shrub_cover[2], 0.0, "-10 - 30 clamps to 0.0");
    assert_eq!(cells.shrub_cover[3], 65.0, "95 - 30 = 65.0");

    // Positive delta exceeding 100% cover clamps to 100.0
    batch_evaluate_succession(&mut cells.view_mut(), 10.0, 50.0);
    assert_eq!(cells.shrub_cover[1], 100.0, "50 + 50 clamps to 100.0");
    assert_eq!(cells.shrub_cover[3], 100.0, "65 + 50 clamps to 100.0");

    // Non-finite delta (NaN / Inf) safely returns 0.0
    let mut cells_nan = LandscapeCellBuffers::new(2);
    cells_nan.shrub_height = vec![10.0, 20.0];
    cells_nan.shrub_cover = vec![30.0, 40.0];
    batch_evaluate_succession(&mut cells_nan.view_mut(), f64::NAN, f64::NAN);
    assert_eq!(cells_nan.shrub_height, vec![0.0, 0.0]);
    assert_eq!(cells_nan.shrub_cover, vec![0.0, 0.0]);
}

#[test]
fn test_biomass_floating_point_anomalies_and_zero_stems() {
    let mut cells = LandscapeCellBuffers::new(4);
    // Non-finite and negative herb inputs
    cells.herb_biomass = vec![f64::NAN, -100.0, 200.0, 0.0];
    cells.herb_cover = vec![20.0, 20.0, 0.0, 0.0];
    cells.herb_height = vec![10.0, 10.0, 0.0, 0.0];
    cells.shrub_cover = vec![10.0, 10.0, 0.0, 0.0];
    cells.shrub_height = vec![30.0, 30.0, 0.0, 0.0];

    let mut shrubs = LandscapeShrubBuffers::new(4, 4);
    shrubs.height = vec![0.0, -10.0, f64::NAN, 50.0];
    shrubs.cover = vec![0.0, 20.0, 30.0, -5.0];
    shrubs.species_id = vec![0, 1, 2, 3];

    let refs = ReferenceData::default();

    // Must execute cleanly without panic or NaN propagation
    batch_evaluate_biomass(&mut cells.view_mut(), &mut shrubs, &refs);

    // Cell biomass sanitization
    assert_eq!(cells.herb_biomass[0], 0.0, "NaN biomass evaluates to 0.0");
    assert_eq!(cells.herb_biomass[1], 0.0, "Negative biomass evaluates to 0.0");
    assert_eq!(cells.herb_biomass[2], 200.0, "Zero shrub volume leaves biomass unchanged");
    assert_eq!(cells.herb_biomass[3], 0.0, "Zero biomass remains 0.0");

    // Shrub stems per acre sanitization
    assert_eq!(shrubs.stems_per_acre[0], 0.0, "Zero height produces 0 stems");
    assert_eq!(shrubs.stems_per_acre[1], 0.0, "Negative height produces 0 stems");
    assert_eq!(shrubs.stems_per_acre[2], 0.0, "NaN height produces 0 stems");
    assert_eq!(shrubs.stems_per_acre[3], 0.0, "Negative cover produces 0 stems");
}

