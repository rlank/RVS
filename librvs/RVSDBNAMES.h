//############################################//
// Temporary file for containing database names,
// field names for input/output. Eventually, this will
// be replaced by the RVS Dataloader which will load
// data into expected tables.
//############################################//

#pragma once

namespace RVS
{
	// Primary table names. Replace with externs as needed
	static const char* RVS_INPUT_TABLE = "Plots_Active";
	static const char* SHRUB_INPUT_TABLE = "Shrubs_Active";
	static const char* BIOMASS_CROSSWALK_TABLE = "Bio_Crosswalk";
	static const char* BIOMASS_EQUATION_TABLE = "Bio_Equation";
	static const char* BIOMASS_MACROGROUP_TABLE = "Bio_Group_LUT";
	static const char* BIOMASS_GROUP_COEFS_TABLE = "Bio_Group_Coef_Cross";
	static const char* BIOMASS_GROUP_COVARIANCE_TABLE = "Bio_Group_Covariance_Cross";
	static const char* COVARIANCE_TABLE = "Covariance_Matrix_NoGroup";
	static const char* FUEL_CROSSWALK_TABLE = "Fuel_Crosswalk";
	static const char* FUEL_EQUATION_TABLE = "Fuel_Equation";
	static const char* FUEL_BPS_ATTR_TABLE = "BPS_Fuelmodels";
	static const char* FUEL_CLASSRULES_TABLE = "Fuel_ClassRules";
	static const char* SUCCESSION_TABLE = "BPS_Combined_Growthrates";
	static const char* PLANTS_TABLE = "Plants";
	static const char* HERB_GROWTH_TABLE = "Herb_Growth";
	static const char* DISTURBANCE_PLOT_TABLE = "Dist_year7fire";
	static const char* DISTURBANCE_TABLE = "Disturbance";
	// ********************

	// Field names (primarily from input)
	static const char* PLOT_NUM_FIELD = "PLOT_ID";
	static const char* PLOT_NAME_FIELD = "PLOT_NAME";
	static const char* EVT_NUM_FIELD = "evt_num";
	static const char* EVT_NAME_FIELD = "evt_name";
	static const char* HERB_COVER_FIELD = "herb_cover";
	static const char* HERB_HEIGHT_FIELD = "herb_height";
	static const char* BPS_NUM_FIELD = "BPS_CODE";
	static const char* BIOMASS_EQUATION_FIELD = "BAT2";
	static const char* BIOMASS_BACKUP_SPP_CODE = "ARTR2";
	static const char* STEMS_PER_ACRE_EQUATION_FIELD = "PCH";
	static const char* FUELS_BACKUP_SPP_CODE = "ARTRT";
	static const char* DOM_SPP_FIELD = "dom_spp";
	static const char* SPP_CODE_FIELD = "spp_code";
	static const char* LIFEFORM_FIELD = "Lifeform2";
	static const char* RET_CODE_FIELD = "ret_code";
	static const char* EQUATION_NUMBER_FIELD = "EQN_NUM";
	static const char* EQUATION_TYPE_FIELD = "eqn_type";
	static const char* GROUP_ID_FIELD = "GRP_ID";
	static const char* GROUP_CONST_FIELD = "GRP_ID_CONST";
	static const char* NDVI_INTERACT_FIELD = "ln_NDVI_GRP_ID";
	static const char* PRCP_INTERACT_FIELD = "ln_PPT_GRP_ID";
	static const char* BIOMASS_HEIGHT_FIELD = "height";
	static const char* BIOMASS_COVER_FIELD = "cover";
	static const char* BPS_MODEL_FIELD = "BPS_MODEL";
	static const char* LATITUDE_FIELD = "latitude";
	static const char* LONGITUDE_FIELD = "longitude";


	static const char* EQN_COEF_1_FIELD = "CF1";
	static const char* EQN_COEF_2_FIELD = "CF2";
	static const char* EQN_COEF_3_FIELD = "CF3";
	static const char* EQN_COEF_4_FIELD = "CF4";
	static const char* EQN_P1_FIELD = "PA1_CODE";
	static const char* EQN_P2_FIELD = "PA2_CODE";
	static const char* EQN_P3_FIELD = "PA3_CODE";

	static const char* HERB_CC_GROWTH_FIELD = "CC_Slope";
	static const char* HERB_HT_GROWTH_FIELD = "HT_Slope";

	static const char* DIST_TYPE_FIELD = "DIST_TYPE";
	static const char* DIST_SUBTYPE_FIELD = "DIST_SUBTYPE";
	static const char* DIST_BEGIN_FIELD = "START_YEAR";
	static const char* DIST_END_FIELD = "STOP_YEAR";
	static const char* DIST_FREQ_FIELD = "FREQ";
	static const char* DIST_VAL1_FIELD = "P1_VAL";
	static const char* DIST_VAL2_FIELD = "P2_VAL";
	static const char* DIST_VAL3_FIELD = "P3_VAL";
	static const char* DIST_VAL1_NAME_FIELD = "P1_NAME";
	static const char* DIST_VAL2_NAME_FIELD = "P2_NAME";
	static const char* DIST_VAL3_NAME_FIELD = "P3_NAME";


	// ********************

	// Fuel classification fields
	static const char* FC_PRIORITY_FIELD = "Priority";
	static const char* FC_SHRUB_COV_LOWER = "shrub_cov_lower";
	static const char* FC_SHRUB_COV_UPPER = "shrub_cov_upper";
	static const char* FC_HERB_PROD_LOWER = "herb_production_lower";
	static const char* FC_HERB_PROD_UPPER = "herb_production_upper";
	static const char* FC_RULE_TABLE_FIELD = "rule_table";

	static const char* FC_FBFM_FIELD = "FBFM";
	static const char* FC_FBFM_DRY_FIELD = "FBFM_Dry";
	static const char* FC_FBFM_HUMID_FIELD = "FBFM_Humid";
	static const char* FC_ISDRY_FIELD = "isDry";

	static const char* FC_HERB_1HR_LOWER = "herb_1hrfuel_lower";
	static const char* FC_HERB_1HR_UPPER = "herb_1hrfuel_upper";
	static const char* FC_HT_LOWER = "HT_lower";
	static const char* FC_HT_UPPER = "HT_upper";
	static const char* FC_SHRUB_FUEL_LOWER = "shrub_fuel_lower";
	static const char* FC_SHRUB_FUEL_UPPER = "shrub_fuel_upper";
	static const char* FC_PROPORTION_LOWER = "proportion_lower";
	static const char* FC_PROPORTION_UPPER = "proportion_upper";
	static const char* FC_TOTAL_FUEL_LOWER = "total_fuel_lower";
	static const char* FC_TOTAL_FUEL_UPPER = "total_fuel_upper";

	// ********************

	// Succession table names
	static const char* COHORT_FIELD = "COHORT";
	static const char* START_AGE_FIELD = "StartAge";
	static const char* END_AGE_FIELD = "EndAge";
	static const char* HT_GR_PER_YEAR_FIELD = "GR_HT_YR";
	static const char* COVER_GR_PER_YEAR_FIELD = "GR_COV_YR";
	static const char* COHORT_TYPE_FIELD = "COHORT_TYPE";
	static const char* SUCCESSION_SPECIES_1_FIELD = "Species1";
	static const char* SUCCESSION_SPECIES_2_FIELD = "Species2";
	static const char* SUCCESSION_SPECIES_3_FIELD = "Species3";
	static const char* SUCCESSION_SPECIES_4_FIELD = "Species4";
	static const char* SUCCESSION_CLASS_FIELD = "sclass";

	// ********************

	// Output table names
	static const char* BIOMASS_OUTPUT_TABLE = "Biomass_Output";
	static const char* BIOMASS_INTERMEDIATE_TABLE = "Biomass_Output_Spp";
	static const char* FUELS_OUTPUT_TABLE = "Fuels_Output";
	static const char* FUELS_INTERMEDIATE_TABLE = "Fuels_Output_Spp";
	static const char* DISTURBANCE_OUTPUT_TABLE = "Disturbance_Output";
	static const char* DISTURBANCE_INTERMEDIATE_TABLE = "Disturbance_Output_Spp";
	// ********************

	// Output table fields
	static const char* YEAR_OUT_FIELD = "year";
	static const char* AVG_SHRUB_HEIGHT_FIELD = "avg_shrub_ht";
	static const char* TOT_SHRUB_COVER_FIELD = "tot_shrub_cov";
	static const char* BIOMASS_STEMS_PER_ACRE_FIELD = "stems_per_acre";
	static const char* BIOMASS_HERB_OUT_FIELD = "herb_biomass";
	static const char* BIOMASS_SHRUB_OUT_FIELD = "shrub_biomass";
	static const char* BIOMASS_TOTAL_OUT_FIELD = "total_biomass";
	static const char* BIOMASS_SHRUB_CALC_FIELD = "biomass_calc";
	static const char* FUEL_1HR_SHRUB_WB = "shrub_1hr_woodbark";
	static const char* FUEL_1HR_SHRUB_FOLIAGE = "shrub_1hr_foliage";
	static const char* FULE_1HR_HERB = "herb_fuel";
	static const char* FUEL_1HR_TOTAL = "fuel_1hr";
	static const char* FUEL_10HR_FIELD = "fuel_10hr";
	static const char* FUEL_100HR_FIELD = "fuel_100hr";
	static const char* FUEL_1000HR_FIELD = "fuel_1000hr";
	static const char* FUEL_TOTAL_FIELD = "total_fuels";
	static const char* UPPER_BOUND_FIELD = "upper_bound";
	static const char* LOWER_BOUND_FIELD = "lower_bound";
	static const char* S2Y_FIELD = "s2y";

	static const char* PCH_EQU_NUM = "pch_eq";
	static const char* PCH_CALC_FIELD = "pch";
	static const char* BIOMASS_EQU_NUM = "bio_eq";

	static const char* BIOMASS_RAW_PRODUCTION_FIELD = "raw_production";
	static const char* BIOMASS_HERB_PP_FIELD = "primary_production";
	static const char* BIOMASS_HERB_HOLDOVER_FIELD = "herb_holdover";

	static const char* DISTURBANCE_HERB_FIELD = "herb_disturbance";
	static const char* DISTURBANCE_SHRUB_FIELD = "shrub_disturbance";
	static const char* DISTURBANCE_AMOUNT_FIELD = "disturbance_amount";

	// ********************
}

