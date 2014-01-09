// ############################################\\
// Preproccessor directives for RVS 
// Last Modified: 8 Jan 14
// ############################################\\

#pragma once

#ifndef RVSDEF_H
#define RVSDEF_H

#define BIO_PATH "Biomass/"
#define DIO_PATH "DataManagement/"

extern const char* RVS_DB_PATH;
#ifndef RVS_DB_PATH
	#define RVS_DB_PATH "C:/MCR/RVS/Data/rvs_bio.db"
#endif

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
	#include <Windows.h>
#endif

// Define whether this will use sqlite or access
#ifndef USESQLITE
	#define USESQLITE 1
#endif

// Includes for either sqlite or access (odbc)
#if USESQLITE
	#include "sqlite3.h"
#else
	#include <sql.h>
	#include <sqltypes.h>
	#include <sqlext.h>
#endif

#endif