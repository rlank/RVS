//############################################//
// Define table and field names here for 
// consistency across classes.
// Last Modified: 24 Apr 14
//############################################//

#pragma once

#ifndef RVSDBNAMES_H
#define RVSDBNAMES_H

// Primary table names. Replace with externs as needed
#define RVS_INPUT_TABLE "RVS_Input_Use"
#define BIOMASS_INPUT_TABLE "Biomass_Input"
#define BIOMASS_CROSSWALK_TABLE "Bio_Crosswalk"
#define BIOMASS_HERBS_TABLE "Bio_Herbs_KgH"
#define BIOMASS_EQUATION_TABLE "Bio_Equation_Import"
#define BIOMASS_MACROGROUP_TABLE "Bio_Group_LUT"
#define BIOMASS_GROUP_COEFS_TABLE "Bio_Group_Coef_Cross"
// ********************

// Field names (primarily from input)
#define PLOT_NUM_FIELD "plot_num"
#define EVT_NUM_FIELD "evt_num"
#define BPS_NUM_FIELD "BPS"
#define DOM_SPP_FIELD "dom_spp"
#define SPP_CODE_FIELD "spp_code"
#define LIFEFORM_FIELD "lifeform"
#define RET_CODE_FIELD "ret_code"
#define EQUATION_NUMBER_FIELD "EQN_NUM"
#define GROUP_ID_FIELD "GRP_ID"
#define GROUP_CONST_FIELD "GRP_ID_CONST"
#define NDVI_INTERACT_FIELD "ln_NDVI_GRP_ID"
#define PRCP_INTERACT_FIELD "ln_PPT_GRP_ID"
// ********************

// Biomass Input -specific input fields
#define BIOMASS_PARAM_1_CODE_FIELD "pa1_code"
#define BIOMASS_PARAM_1_VAL_FIELD "pa1_val"
#define BIOMASS_PARAM_2_CODE_FIELD "pa2_code"
#define BIOMASS_PARAM_2_VAL_FIELD "pa2_code"
#define BIOMASS_HEIGHT_FIELD "height"
#define BIOMASS_COVER_FIELD "pct_cov"
// ********************

// Biomass Equation -specific coefficient fields
#define BIOMASS_EQN_COEF_1_FIELD "CF1"
#define BIOMASS_EQN_COEF_2_FIELD "CF2"
#define BIOMASS_EQN_COEF_3_FIELD "CF3"
#define BIOMASS_EQN_COEF_4_FIELD "CF4"
// ********************

// Output table names
#define BIOMASS_OUTPUT_TABLE "Biomass_Output"
#define BIOMASS_INTERMEDIATE_TABLE "Biomass_Output_Spp"
#define FUELS_OUTPUT_TABLE "Fuels_Output"
#define FUELS_INTERMEDIATE_TABLE "Fuels_Output_Spp"
// ********************

// Output table fields
#define YEAR_OUT_FIELD "year"
#define BIOMASS_HERB_OUT_FIELD "herb_biomass"
#define BIOMASS_SHRUB_OUT_FIELD "shrub_biomass"
#define BIOMASS_TOTAL_OUT_FIELD "total_biomass"
#define FUELS_1HR_OUT_FIELD "fuel_size_1hr"
#define FUELS_10HR_OUT_FIELD "fuel_size_10hr"
#define FUELS_100HR_OUT_FIELD "fuel_size_100hr"
// ********************

#endif
