//############################################//
// Preproccessor directives for RVS 
//############################################//

#pragma once

extern int* RC;
extern int* YEARS;
extern std::string* CLIMATE;
extern bool* SUPPRESS_MSG;
extern char* RVS_DB_PATH;
extern char* OUT_DB_PATH;
extern const int* runmode;
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


#ifndef USEMULTIT
	#define USEMULTIT 0
#endif
