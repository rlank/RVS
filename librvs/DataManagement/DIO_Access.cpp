#include "DIO_Access.h"

int* RVS::DataManagement::DIO_Access::query_analysis_plots()
{
	std::string selectString = "SELECT DISTINCT plot_num FROM RVS_Input_Use";
	DataTable dt; 
	dt = RVS::DataManagement::DIO_Access::query_base(selectString);
	//int[] retAry = new int[dt.Rows.Count];
	//for (int i = 0; i < retAry.Length; i++)
	//{
	//retAry[i] = (int)(short)dt.Rows[i]["plot_num"];
	//}
	//return retAry;
	int i = 0;
	int* ip = &i;
	return ip;
}

RVS::DataManagement::DataTable RVS::DataManagement::DIO_Access::query_biomass_crosswalk_table(std::string spp)
{
	std::string selectString = "SELECT * FROM Bio_Crosswalk WHERE spp_code='" + spp + "'";
	return RVS::DataManagement::DIO_Access::query_base(selectString);
}

RVS::DataManagement::DataTable RVS::DataManagement::DIO_Access::query_biomass_equation_table(std::string spp_code)
{
	std::string selectString = "SELECT * FROM Bio_Equation_Import WHERE SPP_CODE='" + spp_code + "'";
	return RVS::DataManagement::DIO_Access::query_base(selectString);
}

RVS::DataManagement::DataTable RVS::DataManagement::DIO_Access::query_biomass_equation_table(int equation_number)
{
	std::string selectString = "SELECT * FROM Bio_Equation_Import WHERE EQN_NO=" + std::to_string(equation_number);
	return RVS::DataManagement::DIO_Access::query_base(selectString);
}

RVS::DataManagement::DataTable RVS::DataManagement::DIO_Access::query_biomass_herbs_table(int baseBPS)
{
	std::string selectString = "SELECT * FROM Bio_Herbs WHERE BPS_CODE=" + std::to_string(baseBPS);
	return RVS::DataManagement::DIO_Access::query_base(selectString);
}

RVS::DataManagement::DataTable RVS::DataManagement::DIO_Access::query_biomass_input_table(int plot_num)
{
	std::string selectString = "SELECT * FROM Biomass_Input WHERE plot_num=" + std::to_string(plot_num);
	return RVS::DataManagement::DIO_Access::query_base(selectString);
}

RVS::DataManagement::DataTable RVS::DataManagement::DIO_Access::query_base(std::string selectString)
{
	SQLHANDLE hEnv;
	SQLRETURN retCode;

	retCode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	RVS::DataManagement::DIO_Access::CHECK(retCode, "allocate environment handle");

	retCode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	RVS::DataManagement::DIO_Access::CHECK(retCode, "setting the environment attribute setting to ODBC version 3");

	SQLHANDLE hConn;

	RVS::DataManagement::DIO_Access::CHECK(SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hConn), "allocate handle");

	SQLCHAR* dsnName = (SQLCHAR*)RVS::DataManagement::DSN.c_str();
	SQLCHAR* userid = (SQLCHAR*)"";
	SQLCHAR* password = (SQLCHAR*)"";

	retCode = SQLConnectA(hConn, dsnName, SQL_NTS, userid, SQL_NTS, password, SQL_NTS);

	if (!RVS::DataManagement::DIO_Access::CHECK(retCode, "SQLConnect", false))
	{
		RVS::DataManagement::DIO_Access::status(SQL_HANDLE_DBC, hConn, __LINE__);
	}

	SQLHANDLE hStmt;
	RVS::DataManagement::DIO_Access::CHECK(SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt), "allocate handle for statement");

	SQLCHAR* query = (SQLCHAR*)selectString.c_str();
	RVS::DataManagement::DIO_Access::CHECK(SQLExecDirect(hStmt, query, SQL_NTS), "execute query");

	SQLSMALLINT numCols;
	retCode = SQLNumResultCols(hStmt, &numCols);
	SQLCHAR colName[256];

	SQLSMALLINT colNameLen, dataType, numDecimalDigits, allowsNullValues;
	SQLULEN columnSize;


	/*  //## Column tester ##\\
	for (int i = 1; i <= numCols; i++)
	{
		retCode = SQLDescribeCol(hStmt, i, colName, 255, &colNameLen, &dataType, &columnSize, &numDecimalDigits, &allowsNullValues);
		if (RVS::DataManagement::DIO_Access::CHECK(retCode, "SQLDescribeCol"))
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
			if (RVS::DataManagement::DIO_Access::CHECK(retCode, "SQLGetData"))
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


bool RVS::DataManagement::DIO_Access::CHECK(SQLRETURN rc, char* msg, bool printSucceededMsg, bool quit)
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

void RVS::DataManagement::DIO_Access::status(SQLSMALLINT handleType, SQLHANDLE theHandle, int line)
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




