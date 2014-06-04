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
	RC = create_output_db();
	if (*RC != 0)
	{
		//$$ TODO throw bad output database exception
	}
}

// Destructor. Closes the connection with the db.
RVS::DataManagement::DIO::~DIO(void)
{
	printf("\n\n");
	if (rvsdb != NULL)
	{
		*RC = sqlite3_close(rvsdb);
		rvsdb = NULL;

		if (*RC != 0)
		{
			printf("Warning: DB not closing properly.\n");
		}
		else
		{
			printf("Closing InputDB connection.\n");
		}
	}
	

	if (outdb != NULL)
	{
		*RC = sqlite3_close(outdb);
		outdb = NULL;

		if (*RC != 0)
		{
			printf("Warning: Out DB not closing properly.\n");
		}
		else
		{
			printf("Closing OutDB connection.\n");
		}
	}
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
	*RC = std::remove(path);
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

	*RC = sqlite3_exec(outdb, sql, NULL, 0, &err);

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
	
	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
	sqlite3_stmt* stmt = query_base(selectString);
	*RC = sqlite3_step(stmt);
	std::vector<int> items;
	while (*RC == SQLITE_ROW)
	{
		int plot = sqlite3_column_int(stmt, 0);
		items.push_back(plot);
		*RC = sqlite3_step(stmt);
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

	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
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

	
	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
	sqlite3_stmt* stmt = query_base(selectString);

	delete selectStream;
	delete[] selectString;

	return stmt;
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_input_table(int plot_num)
{
	sqlite3_stmt* stmt = query_base(RVS_INPUT_TABLE, PLOT_NUM_FIELD, plot_num);
	return new DataTable(stmt);
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_shrubs_table(int plot_num)
{
	sqlite3_stmt* stmt = query_base(SHRUB_INPUT_TABLE, PLOT_NUM_FIELD, plot_num);
	return new DataTable(stmt);
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, boost::any* retval)
{
	// Get the column data type from sqlite
	int colType = sqlite3_column_type(stmt, column);
	// Parse the data based on the appropriate data type
	switch (colType)
	{
	case SQLITE_INTEGER:
		*retval = sqlite3_column_int(stmt, column);
		break;
	case SQLITE_FLOAT:
		*retval = sqlite3_column_double(stmt, column);
		break;
	case SQLITE_BLOB:
		*retval = 0.0;
		break;
	case SQLITE_NULL:
		*retval = 0.0;
		break;
	case SQLITE3_TEXT:
		*retval = std::string((char*)sqlite3_column_text(stmt, column));
		break;
		
	}
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, double* retVal)
{
	int colType = sqlite3_column_type(stmt, column);
	if (colType == SQLITE_FLOAT)
	{
		*retVal = sqlite3_column_double(stmt, column);
	}
	else if (colType == SQLITE_NULL)
	{
		*retVal = 0.0;
	}
	else
	{
		//$$ TODO throw data mismatch exception
	}
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, std::string* retVal)
{
	int colType = sqlite3_column_type(stmt, column);
	if (colType == SQLITE3_TEXT)
	{
		*retVal = std::string((char*)sqlite3_column_text(stmt, column));
	}
	else if (colType == SQLITE_NULL)
	{
		*retVal = "";
	}
	else
	{
		//$$ TODO throw data mismatch exception
	}
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, int* retVal)
{
	int colType = sqlite3_column_type(stmt, column);
	if (colType == SQLITE_INTEGER)
	{
		*retVal = sqlite3_column_int(stmt, column);
	}
	else if (colType == SQLITE_NULL)
	{
		*retVal = 0;
	}
	else
	{
		//$$ TODO throw data mismatch exception
	}
}

char* RVS::DataManagement::DIO::streamToCharPtr(std::stringstream* stream)
{
	// Get the string representation of the stream
	std::string nstring = stream->str();
	// Create a character array of the size of string
	char* str = new char[nstring.size() + 1];
	// Copy the string value into the array and terminate
	std::copy(nstring.begin(), nstring.end(), str); // apparently this has been depreciated
	str[nstring.size()] = '\0';
	return str;
}




int RVS::DataManagement::DIO::callback(void* nu, int argc, char** argv, char** azColName)
{
	return 0;
}


#else

#endif




