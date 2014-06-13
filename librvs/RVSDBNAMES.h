//############################################//
// Define table and field names here for 
// consistency across classes.
// Last Modified: 24 Apr 14
//############################################//

#pragma once

#ifndef RVSDBNAMES_H
#define RVSDBNAMES_H

// Primary table names. Replace with externs as needed
#define RVS_INPUT_TABLE "Plots"
#define SHRUB_INPUT_TABLE "Shrub_Input"
#define BIOMASS_CROSSWALK_TABLE "Bio_Crosswalk"
#define BIOMASS_PRIMARYPRODUCTION_TABLE "Bio_PP_KgH"
#define BIOMASS_EQUATION_TABLE "Bio_Equation"
#define BIOMASS_MACROGROUP_TABLE "Bio_Group_LUT"
#define BIOMASS_GROUP_COEFS_TABLE "Bio_Group_Coef_Cross"
#define FUEL_CROSSWALK_TABLE "Fuel_Crosswalk"
#define FUEL_EQUATION_TABLE "Fuel_Equation"
// ********************

// Field names (primarily from input)
#define PLOT_NUM_FIELD "PLOT_ID"
#define EVT_NUM_FIELD "evt_num"
#define EVT_NAME_FIELD "evt_name"
#define BPS_NUM_FIELD "BPS"
#define DOM_SPP_FIELD "dom_spp"
#define SPP_CODE_FIELD "spp_code"
#define LIFEFORM_FIELD "lifeform"
#define RET_CODE_FIELD "ret_code"
#define EQUATION_NUMBER_FIELD "EQN_NUM"
#define EQUATION_TYPE_FIELD "eqn_type"
#define GROUP_ID_FIELD "GRP_ID"
#define GROUP_CONST_FIELD "GRP_ID_CONST"
#define NDVI_INTERACT_FIELD "ln_NDVI_GRP_ID"
#define PRCP_INTERACT_FIELD "ln_PPT_GRP_ID"
#define BIOMASS_HEIGHT_FIELD "height"
#define BIOMASS_COVER_FIELD "cover"
// ********************


// Equation -specific coefficient fields
#define EQN_COEF_1_FIELD "CF1"
#define EQN_COEF_2_FIELD "CF2"
#define EQN_COEF_3_FIELD "CF3"
#define EQN_COEF_4_FIELD "CF4"
#define EQN_P1_FIELD "PA1_CODE"
#define EQN_P2_FIELD "PA2_CODE"
#define EQN_P3_FIELD "PA3_CODE"
// ********************

// Output table names
#define BIOMASS_OUTPUT_TABLE "Biomass_Output"
#define BIOMASS_INTERMEDIATE_TABLE "Biomass_Output_Spp"
#define FUELS_OUTPUT_TABLE "Fuels_Output"
#define FUELS_INTERMEDIATE_TABLE "Fuels_Output_Spp"
// ********************

// Output table fields
#define YEAR_OUT_FIELD "year"
#define BIOMASS_STEMS_PER_ACRE_FIELD "stems_per_acre"
#define BIOMASS_HERB_OUT_FIELD "herb_biomass"
#define BIOMASS_SHRUB_OUT_FIELD "shrub_biomass"
#define BIOMASS_TOTAL_OUT_FIELD "total_biomass"
#define FUEL_1HR_OUT_FIELD "FS1"
#define FUEL_10HR_OUT_FIELD "FS2"
#define FUEL_100HR_OUT_FIELD "FS3"
#define FUEL_1HRLIVE_OUT_FIELD "FL1"
#define FUEL_10HRLIVE_OUT_FIELD "FL2"
#define FUEL_100HRLIVE_OUT_FIELD "FL3"
#define FUEL_1HRDEAD_OUT_FIELD "FD1"
#define FUEL_10HRDEAD_OUT_FIELD "FD2"
#define FUEL_100HRDEAD_OUT_FIELD "FD3"
// ********************

#endif
