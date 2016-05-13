//############################################//
// Preproccessor directives for RVS 
// Last Modified: 18 Apr 16
//############################################//

#pragma once

extern int* RC;
extern int* YEARS;
extern std::string* CLIMATE;
extern bool* SUPPRESS_MSG;
//extern const char* RVS_DB_PATH;
//extern const char* OUT_DB_PATH;
extern const char* DEBUG_FILE;
extern bool* USE_MEM;

// OS-specific includes
#define WIN 0

#ifdef _WIN64
	#undef WIN
	#define WIN 1
#elif _WIN32
	#undef WIN
	#define WIN 1
#elif __linux
#endif

// Define runmode
// 1: normal sim
// 2: single year FCCS shrub test
// 3: 5 year herb test
// 4: shrub equation test
#define runmode 1

#if WIN
	#ifndef RVS_DB_PATH
	#define RVS_DB_PATH "C:/Users/robblankston/Documents/GitHub/RVS/rvs_in.db"
	#endif
#endif

#if WIN && runmode == 1
	#define OUT_DB_PATH "C:/Users/robblankston/Documents/GitHub/RVS/rvs_out.db"
#elif WIN && runmode == 2
	#define OUT_DB_PATH "C:/Users/robblankston/Documents/GitHub/RVS/rvs_out_FCCS_test.db"
#elif WIN && runmode == 3
	#define OUT_DB_PATH "C:/Users/robblankston/Documents/GitHub/RVS/rvs_out_herb_test.db"
#elif WIN && runmode == 4
	#define OUT_DB_PATH "C:/Users/robblankston/Documents/GitHub/RVS/rvs_out_shrub_test.db"
#else
	#ifndef RVS_DB_PATH
	#define RVS_DB_PATH "/home/robb/rvs_bio.db"
	#define OUT_DB_PATH "/home/robb/rvs_out.db"
	#endif
#endif

//$$ TODO Define multithreading environment
#ifndef USEMULTIT
	#define USEMULTIT 0
#endif
