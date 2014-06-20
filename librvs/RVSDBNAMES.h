//############################################//
// Temporary file for containing database names,
// field names for input/output. Eventually, this will
// be replaced by the RVS Dataloader which will load
// data into expected tables.
// Last Modified: 19 Jun 14
//############################################//

#pragma once

#ifndef RVSDBNAMES_H
#define RVSDBNAMES_H

namespace RVS
{
	// Primary table names. Replace with externs as needed
	static const char* RVS_INPUT_TABLE = "Plots";
	static const char* SHRUB_INPUT_TABLE = "Shrub_Input";
	static const char* BIOMASS_CROSSWALK_TABLE = "Bio_Crosswalk";
	static const char* BIOMASS_PRIMARYPRODUCTION_TABLE = "Bio_PP_KgH";
	static const char* BIOMASS_EQUATION_TABLE = "Bio_Equation";
	static const char* BIOMASS_MACROGROUP_TABLE = "Bio_Group_LUT";
	static const char* BIOMASS_GROUP_COEFS_TABLE = "Bio_Group_Coef_Cross";
	static const char* FUEL_CROSSWALK_TABLE = "Fuel_Crosswalk";
	static const char* FUEL_EQUATION_TABLE = "Fuel_Equation";
	static const char* FUEL_BPS_ATTR_TABLE = "BPS_Fuelmodels";
	static const char* FUEL_CLASSRULES_TABLE = "Fuel_ClassRules";
	// ********************

	// Field names (primarily from input)
	static const char* PLOT_NUM_FIELD = "PLOT_ID";
	static const char* EVT_NUM_FIELD = "evt_num";
	static const char* EVT_NAME_FIELD = "evt_name";
	static const char* BPS_NUM_FIELD = "BPS";
	static const char* DOM_SPP_FIELD = "dom_spp";
	static const char* SPP_CODE_FIELD = "spp_code";
	static const char* LIFEFORM_FIELD = "lifeform";
	static const char* RET_CODE_FIELD = "ret_code";
	static const char* EQUATION_NUMBER_FIELD = "EQN_NUM";
	static const char* EQUATION_TYPE_FIELD = "eqn_type";
	static const char* GROUP_ID_FIELD = "GRP_ID";
	static const char* GROUP_CONST_FIELD = "GRP_ID_CONST";
	static const char* NDVI_INTERACT_FIELD = "ln_NDVI_GRP_ID";
	static const char* PRCP_INTERACT_FIELD = "ln_PPT_GRP_ID";
	static const char* BIOMASS_HEIGHT_FIELD = "height";
	static const char* BIOMASS_COVER_FIELD = "cover";

	static const char* EQN_COEF_1_FIELD = "CF1";
	static const char* EQN_COEF_2_FIELD = "CF2";
	static const char* EQN_COEF_3_FIELD = "CF3";
	static const char* EQN_COEF_4_FIELD = "CF4";
	static const char* EQN_P1_FIELD = "PA1_CODE";
	static const char* EQN_P2_FIELD = "PA2_CODE";
	static const char* EQN_P3_FIELD = "PA3_CODE";
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
	static const char* FC_SHURB_FUEL_LOWER = "shrub_fuel_lower";
	static const char* FC_SHRUB_FUEL_UPPER = "shrub_fuel_upper";
	static const char* FC_PROPORTION_LOWER = "proportion_lower";
	static const char* FC_PROPORTION_UPPER = "proportion_upper";
	static const char* FC_TOTAL_FUEL_LOWER = "total_fuel_lower";
	static const char* FC_TOTAL_FUEL_UPPER = "total_fuel_upper";

	// ********************

	// Output table names
	static const char* BIOMASS_OUTPUT_TABLE = "Biomass_Output";
	static const char* BIOMASS_INTERMEDIATE_TABLE = "Biomass_Output_Spp";
	static const char* FUELS_OUTPUT_TABLE = "Fuels_Output";
	static const char* FUELS_INTERMEDIATE_TABLE = "Fuels_Output_Spp";
	// ********************

	// Output table fields
	static const char* YEAR_OUT_FIELD = "year";
	static const char* BIOMASS_STEMS_PER_ACRE_FIELD = "stems_per_acre";
	static const char* BIOMASS_HERB_OUT_FIELD = "herb_biomass";
	static const char* BIOMASS_SHRUB_OUT_FIELD = "shrub_biomass";
	static const char* BIOMASS_TOTAL_OUT_FIELD = "total_biomass";
	static const char* FUEL_1HR_OUT_FIELD = "FS1";
	static const char* FUEL_10HR_OUT_FIELD = "FS2";
	static const char* FUEL_100HR_OUT_FIELD = "FS3";
	static const char* FUEL_1HRLIVE_OUT_FIELD = "FL1";
	static const char* FUEL_10HRLIVE_OUT_FIELD = "FL2";
	static const char* FUEL_100HRLIVE_OUT_FIELD = "FL3";
	static const char* FUEL_1HRDEAD_OUT_FIELD = "FD1";
	static const char* FUEL_10HRDEAD_OUT_FIELD = "FD2";
	static const char* FUEL_100HRDEAD_OUT_FIELD = "FD3";
	// ********************
}
#endif
