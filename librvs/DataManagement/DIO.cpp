#include "DIO.h"

// Static object decs //

// RVS input database
sqlite3* RVS::DataManagement::DIO::rvsdb;
// RVS ouput database
sqlite3* RVS::DataManagement::DIO::outdb;

// Constructor
RVS::DataManagement::DIO::DIO(void)
{
	RC = open_db_connection(RVS_DB_PATH, &rvsdb);
	if (*RC != 0)
	{
		//$$ TODO throw bad input database exception here
	}
}

// Destructor. Closes the connection with the db.
RVS::DataManagement::DIO::~DIO(void)
{
	*RC = sqlite3_close(rvsdb);
	if (*RC != 0)
	{
		printf("Warning: DB not closing properly.\n");
	}
	else
	{
		printf("Closing InputDB connection.\n");
	}

	if (outdb != NULL)
	{
		*RC = sqlite3_close(outdb);
	}
	if (*RC != 0)
	{
		printf("Warning: Out DB not closing properly.\n");
	}
	else
	{
		printf("Closing OutDB connection.\n");
	}

	delete RC;  // This is the last thing the program does. Delete the return pointer
}

// Opens an sqlite connection to the specified database
int* RVS::DataManagement::DIO::open_db_connection(char* pathToDb, sqlite3** db)
{
	// Open the database
	*RC = sqlite3_open(pathToDb, db);

	if (*RC != SQLITE_OK)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
		sqlite3_close(*db);
	}
	return RC;
}

int* RVS::DataManagement::DIO::create_output_db()
{
	RC = create_output_db(OUT_DB_PATH);
	return RC;
}

int* RVS::DataManagement::DIO::create_output_db(char* path)
{
	RC = open_db_connection(path, &outdb);
	return RC;
}

int* RVS::DataManagement::DIO::create_table(char* sql)
{
	if (outdb == NULL)
	{
		RC = RVS::DataManagement::DIO::create_output_db();
	}

	if (*RC != 0)
	{
		//$$ TODO throw create output error
	}

	char* err;
	int(*cb)(void*, int, char**, char**);
	cb = &RVS::DataManagement::DIO::callback;

	*RC = sqlite3_exec(outdb, sql, cb, 0, &err);

	if (*RC != SQLITE_OK)
	{
		sqlite3_free(err);
	}

	return RC;
}

int* RVS::DataManagement::DIO::exec_sql(char* sql)
{
	char* err;
	int(*cb)(void*, int, char**, char**);
	cb = &RVS::DataManagement::DIO::callback;

	*RC = sqlite3_exec(outdb, sql, cb, 0, &err);

	if (*RC != SQLITE_OK)
	{
		sqlite3_free(err);
	}

	return RC;
}

std::vector<int> RVS::DataManagement::DIO::query_analysis_plots()
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT DISTINCT " << PLOT_NUM_FIELD << " FROM " << RVS_INPUT_TABLE << "; ";
	
	char* selectString = streamToCharPtr(selectStream);
	sqlite3_stmt* stmt = query_base(selectString);
	
	std::vector<int> items;
	*RC = SQLITE_OK;
	while (*RC == SQLITE_OK || *RC == SQLITE_ROW)
	{
		*RC = sqlite3_step(stmt);
		int plot = sqlite3_column_int(stmt, 0);
		items.push_back(plot);
	}

	*RC = sqlite3_finalize(stmt);
	delete selectStream;
	delete[] selectString;

	return items;
}


#if USESQLITE

sqlite3_stmt* RVS::DataManagement::DIO::query_base(char* selectString)
{
	int nByte = -1;
	sqlite3_stmt* stmt;

	// Prepare SQL query as object code
	*RC = sqlite3_prepare_v2(rvsdb, selectString, nByte, &stmt, NULL);
	if (*RC != SQLITE_OK)
	{
		fprintf(stderr, "DB Operation Error: %s\n", sqlite3_errmsg(rvsdb));
	}

	return stmt;
}

sqlite3_stmt* RVS::DataManagement::DIO::query_base(char* table, char* field)
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT " << field << " FROM " << table << "; ";

	char* selectString = streamToCharPtr(selectStream);
	sqlite3_stmt* stmt = query_base(selectString);

	delete selectStream;
	delete[] selectString;

	return stmt;
}

sqlite3_stmt* RVS::DataManagement::DIO::query_base(char* table, char* field, boost::any whereclause)
{
	std::stringstream* selectStream = new std::stringstream();

	if (whereclause.type() == typeid(std::string))
	{
		*selectStream << "SELECT * FROM " << table << " WHERE " << field << "='" << boost::any_cast<std::string>(whereclause) << "'; ";
	}
	else if (whereclause.type() == typeid(int))
	{
		*selectStream << "SELECT * FROM " << table << " WHERE " << field << "=" << boost::any_cast<int>(whereclause) << "; ";
	}

	
	char* selectString = streamToCharPtr(selectStream);
	sqlite3_stmt* stmt = query_base(selectString);

	delete selectStream;
	delete[] selectString;

	return stmt;
}

char* RVS::DataManagement::DIO::streamToCharPtr(std::stringstream* stream)
{
	// Get the string representation of the stream
	std::string nstring = stream->str();
	// Create a character array of the size of string
	char* str = new char[nstring.size() + 1];
	// Copy the string value into the array and terminate
	std::copy(nstring.begin(), nstring.end(), str);
	str[nstring.size()] = '\0';
	return str;
}

int RVS::DataManagement::DIO::callback(void* nu, int argc, char** argv, char** azColName)
{
	return 0;
}

/*
RVS::DataManagement::DataTable RVS::DataManagement::DIO::query_base_old(char* selectString)
{
	sqlite3* rvsdb;
	int rc = 0;

	DataTable dt;

	// Open the database
	rc = sqlite3_open(RVS_DB_PATH, &rvsdb);
	
	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(rvsdb));
		sqlite3_close(rvsdb);
		return dt;
	}

	int nByte = -1;
	sqlite3_stmt* stmt;

	// Prepare SQL query as object code
	rc = sqlite3_prepare_v2(rvsdb, selectString, nByte, &stmt, NULL);
	// Run initial step
	rc = sqlite3_step(stmt);

	if (rc == 0 || rc == 100 || rc == 101)
	{
		int colCount = 0;
		colCount = sqlite3_data_count(stmt);
		printf("Columns: %i\n", colCount);
	}

	while (rc == 100)
	{
		rc = sqlite3_step(stmt);
		printf("Stepping\n");
	}

	rc = sqlite3_finalize(stmt);
	rc = sqlite3_close(rvsdb);

	return dt;
}
*/
#else
RVS::DataManagement::DataTable RVS::DataManagement::DIO::query_base(std::string selectString)
{
	SQLHANDLE hEnv;
	SQLRETURN retCode;

	retCode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	RVS::DataManagement::DIO::CHECK(retCode, "allocate environment handle");

	retCode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	RVS::DataManagement::DIO::CHECK(retCode, "setting the environment attribute setting to ODBC version 3");

	SQLHANDLE hConn;

	RVS::DataManagement::DIO::CHECK(SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hConn), "allocate handle");

	SQLCHAR* dsnName = (SQLCHAR*)RVS::DataManagement::DIO::PATHTODB;
	SQLCHAR* userid = (SQLCHAR*)"";
	SQLCHAR* password = (SQLCHAR*)"";

	retCode = SQLConnectA(hConn, dsnName, SQL_NTS, userid, SQL_NTS, password, SQL_NTS);

	if (!RVS::DataManagement::DIO::CHECK(retCode, "SQLConnect", false))
	{
		RVS::DataManagement::DIO::status(SQL_HANDLE_DBC, hConn, __LINE__);
	}

	SQLHANDLE hStmt;
	RVS::DataManagement::DIO::CHECK(SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt), "allocate handle for statement");

	SQLCHAR* query = (SQLCHAR*)selectString.c_str();
	RVS::DataManagement::DIO::CHECK(SQLExecDirect(hStmt, query, SQL_NTS), "execute query");

	SQLSMALLINT numCols;
	retCode = SQLNumResultCols(hStmt, &numCols);
	SQLCHAR colName[256];

	SQLSMALLINT colNameLen, dataType, numDecimalDigits, allowsNullValues;
	SQLULEN columnSize;


	/*  //## Column tester ##\\
	for (int i = 1; i <= numCols; i++)
	{
	retCode = SQLDescribeCol(hStmt, i, colName, 255, &colNameLen, &dataType, &columnSize, &numDecimalDigits, &allowsNullValues);
	if (RVS::DataManagement::DIO::CHECK(retCode, "SQLDescribeCol"))
	{
	printf("Column #%d: '%s', datatype=%d size=%d decimaldigits=%d nullable=%d\n",
	i, colName, dataType, columnSize, numDecimalDigits, allowsNullValues);
	}
	}
	*/


	RVS::DataManagement::DataTable dt;

	// This gets each row by calling SQLFetch until it fails
	while (SQL_SUCCEEDED(SQLFetch(hStmt)))
	{
		char buf[256];
		SQLLEN numBytes;

		for (int j = 1; j <= numCols; j++)
		{
			retCode = SQLDescribeCol(hStmt, j, colName, 255, &colNameLen, &dataType, &columnSize, &numDecimalDigits, &allowsNullValues);
			retCode = SQLGetData(hStmt, j, SQL_C_CHAR, buf, 255, &numBytes);

			// Print this column's data
			if (RVS::DataManagement::DIO::CHECK(retCode, "SQLGetData"))
			{
				printf("%s:\t%s", colName, buf);
			}
			puts("");
			*buf = NULL;
		}
		puts("");
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	SQLFreeHandle(SQL_HANDLE_DBC, hConn);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

	return dt;
}


bool RVS::DataManagement::DIO::CHECK(SQLRETURN rc, char* msg, bool printSucceededMsg, bool quit)
{
	if (SQL_SUCCEEDED(rc))
	{
		if (printSucceededMsg)
			printf("%s succeeded\n", msg);
		return true;
	}
	else
	{
		printf("%s has FAILED!\n", msg);
		if (quit)
			FatalAppExitA(0, msg);
		return false;
	}
}

void RVS::DataManagement::DIO::status(SQLSMALLINT handleType, SQLHANDLE theHandle, int line)
{
	SQLCHAR sqlState[6];
	SQLINTEGER nativeError;
	SQLCHAR msgStr[256];
	SQLSMALLINT overBy;  // number of characters that msgStr buffer was short by

	SQLRETURN retCode;

	for (int i = 1; i < 20; i++)
	{
		retCode = SQLGetDiagRecA(
			handleType,		// The type of object you're checking the status of.
			theHandle,		// Handle to the actual object you want the status of.
			i,				// WHICH status message you want.
			sqlState,		// OUT:  gives back 5 characters (the HY*** style error code).
			&nativeError,	// Numerical error number.
			msgStr,			// Buffer to store the DESCRIPTION OF THE ERROR.
			255,			// The number of characters in msgStr.
			&overBy			// Again in case the function has A LOT to tell you.
			);


		if (CHECK(retCode, "SQLGetDiagRecA"))
		{
			printf("Line %d: [%s][%d] %s\n", line, sqlState, nativeError, msgStr);
		}
		else
		{
			// Stop looping when retCode comes back
			// as a failure, because it means there are
			// no more messages to tell you
			break;
		}
	}
}
#endif




