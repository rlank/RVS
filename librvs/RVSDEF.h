//############################################//
// Preproccessor directives for RVS 
// Last Modified: 19 Jun 14
//############################################//

#pragma once

#ifndef RVSDEF_H
#define RVSDEF_H

extern int* RC;
//extern const char* RVS_DB_PATH;
//extern const char* OUT_DB_PATH;
extern const char* DEBUG_FILE;

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

#if WIN
	#ifndef RVS_DB_PATH
	#define RVS_DB_PATH "C:/Users/robb/Documents/GitHub/RVS/rvs_in.db"
	#define OUT_DB_PATH "C:/Users/robb/Documents/GitHub/RVS/rvs_out.db"
	#endif
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


#endif
