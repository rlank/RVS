//############################################//
// Preproccessor directives for RVS 
// Last Modified: 22 Jan 14
//############################################//

#pragma once

#ifndef RVSDEF_H
#define RVSDEF_H

extern int* RC;
extern float* GROWTH_MULT;
//extern const char* RVS_DB_PATH;
//extern const char* OUT_DB_PATH;


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
	#define RVS_DB_PATH "C:/MCR/RVS/Data/rvs_bio.db"
	#define OUT_DB_PATH "C:/MCR/RVS/Data/rvs_out.db"
	#endif
#else
	#ifndef RVS_DB_PATH
	#define RVS_DB_PATH "/home/robb/rvs_bio.db"
	#define OUT_DB_PATH "/home/robb/rvs_out.db"
	#endif
#endif

// Define whether this will use sqlite or access
#ifndef USESQLITE
	#define USESQLITE 1
#endif

// Includes for either sqlite or access (odbc)
#if USESQLITE
	#include <sqlite3.h>
#else
	#include <Windows.h>
	#include <sql.h>
	#include <sqltypes.h>
	#include <sqlext.h>
#endif




#endif
