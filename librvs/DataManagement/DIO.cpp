#include "DIO.h"

// Static object decs
sqlite3* RVS::DataManagement::DIO::rvsdb;

// Constructor
RVS::DataManagement::DIO::DIO(void)
{
	int rc = open_db_connection(RVS_DB_PATH);
}

// Destructor. Closes the connection with the db.
RVS::DataManagement::DIO::~DIO(void)
{
	int rc = sqlite3_close(rvsdb);
	if (rc != 0)
	{
		printf("Warning: DB not closing properly.\n");
	}
	else
	{
		printf("Closing DB connection.\n");
	}
}

// Opens an sqlite connection to the specified database
int RVS::DataManagement::DIO::open_db_connection(char* pathToDb)
{
	int rc = 0;

	// Open the database
	rc = sqlite3_open(pathToDb, &rvsdb);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(rvsdb));
		sqlite3_close(rvsdb);
	}
	return rc;
}


std::vector<int> RVS::DataManagement::DIO::query_analysis_plots()
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT DISTINCT " << PLOT_NUM_FIELD << " FROM " << RVS_INPUT_TABLE << "; ";
	
	char* selectString = streamToCharPtr(selectStream);
	sqlite3_stmt* stmt = query_base(selectString);
	
	std::vector<int> items;
	int rc = SQLITE_OK;
	while (rc == SQLITE_OK || rc == SQLITE_ROW)
	{
		rc = sqlite3_step(stmt);
		int plot = sqlite3_column_int(stmt, 0);
		items.push_back(plot);
	}

	rc = sqlite3_finalize(stmt);
	delete[] selectStream;
	delete[] selectString;

	return items;
}




#if USESQLITE

sqlite3_stmt* RVS::DataManagement::DIO::query_base(char* selectString)
{
	int nByte = -1;
	sqlite3_stmt* stmt;

	// Prepare SQL query as object code
	int rc = sqlite3_prepare_v2(rvsdb, selectString, nByte, &stmt, NULL);
	if (rc)
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

	delete[] selectStream;
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

	delete[] selectStream;
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




