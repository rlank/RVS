#include "DIO.h"

// Static object decs //

// RVS input database
sqlite3* RVS::DataManagement::DIO::rvsdb;
// RVS ouput database
sqlite3* RVS::DataManagement::DIO::outdb;

map<string, shared_ptr<RVS::DataManagement::DataTable>> RVS::DataManagement::DIO::activeQueries;
vector<const char*> RVS::DataManagement::DIO::queuedWrites;

// Constructor
RVS::DataManagement::DIO::DIO(void)
{
	char* err = NULL;
	if (rvsdb == NULL)
	{
		if (!USE_MEM)
		{
			RC = open_db_connection(RVS_DB_PATH, &rvsdb);
		}
		else
		{
			RC = open_db_connection(":memory:", &rvsdb);
			buildInMemDB(rvsdb, RVS_DB_PATH, 0);
		}
	}
	
	checkDBStatus(rvsdb);

	if (outdb == NULL)
	{
		RC = create_output_db();
	}
	
	checkDBStatus(outdb);
}

// Destructor. Closes the connection with the db.
RVS::DataManagement::DIO::~DIO(void)
{
	RC = finalizeQueries();
	if (rvsdb != NULL)
	{
		*RC = sqlite3_close(rvsdb);
		rvsdb = NULL;
		
		if (*RC != 0)
		{
			write_debug_msg("Warning: DB not closing properly.");
			write_debug_msg(sqlite3_errmsg(rvsdb));
		}
		else
		{
			write_debug_msg("Closing InputDB connection");
		}
	}
	
	if (outdb != NULL)
	{
		*RC = sqlite3_close(outdb);
		outdb = NULL;

		if (*RC != 0)
		{
			write_debug_msg("Warning: DB not closing properly.");
			write_debug_msg(sqlite3_errmsg(outdb));
		}
		else
		{
			write_debug_msg("Closing InputDB connection");
		}
	}
}

// Opens an sqlite connection to the specified database
int* RVS::DataManagement::DIO::open_db_connection(char* pathToDb, sqlite3** db)
{
	// Open the database
	*RC = sqlite3_open(pathToDb, db);
	checkDBStatus(*db);

	if (*RC != SQLITE_OK)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
		sqlite3_close(*db);
	}

	sqlite3_exec(*db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
	sqlite3_exec(*db, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL);

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


RVS::DataManagement::DataTable* RVS::DataManagement::DIO::prep_datatable(const char* sql, sqlite3* db, bool addToActive, bool reset)
{
	shared_ptr<DataTable> dt;
	if (isQueryActive(sql))
	{
		dt = activeQueries[sql];
		if (reset)
		{
			*RC = sqlite3_reset(dt->getStmt());
			*RC = sqlite3_step(dt->getStmt());
			*(dt->STATUS()) = *RC;
			//checkDBStatus(db, sql);
		}
	}
	else
	{
		int nByte = -1;
		sqlite3_stmt* stmt;

		// Prepare SQL query as object code
		*RC = sqlite3_prepare_v2(db, sql, nByte, &stmt, NULL);
		//checkDBStatus(db, sql);
		*RC = sqlite3_step(stmt);
		//checkDBStatus(db, sql);
		dt = shared_ptr<DataTable>(new DataTable(stmt));

		if (addToActive) { activeQueries.insert(pair<string, shared_ptr<DataTable>>(sql, dt)); }

		int* rc = dt->STATUS();
		*rc = *RC;
	}

	return dt.get();
}

int* RVS::DataManagement::DIO::write_output(void)
{
	char* err;
	*RC = sqlite3_exec(outdb, "BEGIN TRANSACTION", NULL, NULL, &err);

	for (int w = 0; w < queuedWrites.size(); w++)
	{
		const char* sql = queuedWrites.at(w);
		*RC = sqlite3_exec(outdb, sql, NULL, NULL, &err);
		checkDBStatus(outdb, sql, err);
		sqlite3_free(err);
	}

	*RC = sqlite3_exec(outdb, "END TRANSACTION", NULL, NULL, &err);
	checkDBStatus(outdb, NULL, err);
	sqlite3_free(err);

	return RC;
}

void RVS::DataManagement::DIO::write_debug_msg(const char* msg)
{
	time_t t = time(NULL);
	char strTime[25];
	strftime(strTime, 25, "%d/%m/%y %H:%M:%S", localtime(&t));

	ofstream* dfile = new ofstream(DEBUG_FILE, ios::app);
	*dfile << strTime << " " << msg << std::endl;
	dfile->close();
}

std::vector<int> RVS::DataManagement::DIO::query_analysis_plots()
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT DISTINCT " << PLOT_NUM_FIELD << " FROM " << RVS_INPUT_TABLE << "; ";
	
	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
	
	RVS::DataManagement::DataTable* dt = prep_datatable(selectString, rvsdb);
	std::vector<int> items;
	int plot;
	while (*RC == SQLITE_ROW)
	{
		getVal(dt->getStmt(), 0, &plot);
		items.push_back(plot);
		*RC = sqlite3_step(dt->getStmt());
	}

	delete selectStream;
	delete selectString;

	return items;
}

const char* RVS::DataManagement::DIO::query_base(const char* table)
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT * FROM " << table << "; ";

	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
	delete selectStream;
	return selectString;
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field)
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT " << field << " FROM " << table << "; ";

	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
	delete selectStream;
	return selectString;
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field, int whereclause)
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT * FROM " << table << " WHERE " << field << "=" << whereclause << "; ";

	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
	delete selectStream;
	return selectString;
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field, string whereclause)
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT * FROM " << table << " WHERE " << field << "='" << whereclause << "'; ";
	
	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
	delete selectStream;
	return selectString;
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field, int whereclause, string order)
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT * FROM " << table << " WHERE " << field << "=" << whereclause;
	*selectStream << " ORDER BY " << order << ";";

	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
	delete selectStream;
	return selectString;
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field, string whereclause, string order)
{
	std::stringstream* selectStream = new std::stringstream();
	*selectStream << "SELECT * FROM " << table << " WHERE " << field << "='" << whereclause << "'";
	*selectStream << " ORDER BY " << order << ";";

	char* selectString = new char;
	selectString = streamToCharPtr(selectStream);
	delete selectStream;
	return selectString;
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_input_table(void)
{
	std::stringstream* sqlStream = new std::stringstream();
	*sqlStream << "SELECT * FROM " << RVS_INPUT_TABLE << ";";
	const char* sql = streamToCharPtr(sqlStream);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb);
	return dt;
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_input_table(int plot_num)
{
	const char* sql = query_base(RVS_INPUT_TABLE, PLOT_NUM_FIELD, plot_num);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb, false);
	return dt;
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_shrubs_table(void)
{
	std::stringstream* sqlStream = new std::stringstream();
	*sqlStream << "SELECT * FROM " << SHRUB_INPUT_TABLE << ";";
	const char* sql = streamToCharPtr(sqlStream);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb, true);
	return dt;
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
		//throw RVS::DataManagement::DataTypeMismatchException(sqlite3_sql(stmt));
	}
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, string* retVal)
{
	int colType = sqlite3_column_type(stmt, column);
	if (colType == SQLITE3_TEXT)
	{
		*retVal = string((char*)sqlite3_column_text(stmt, column));
	}
	else if (colType == SQLITE_NULL)
	{
		*retVal = "";
	}
	else
	{
		//throw RVS::DataManagement::DataTypeMismatchException(sqlite3_sql(stmt));
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
		//throw RVS::DataManagement::DataTypeMismatchException(sqlite3_sql(stmt));
	}
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, bool* retVal)
{
	// SQLITE does not use booleans.  Integer 1 == true, 0 == false
	int colType = sqlite3_column_type(stmt, column);
	if (colType == SQLITE_INTEGER)
	{
		int val = sqlite3_column_int(stmt, column);
		if (val == 1)
		{
			*retVal = true;
		}
		else if (val == 0)
		{
			*retVal = false;
		}
		else
		{
			//throw RVS::DataManagement::DataTypeMismatchException(sqlite3_sql(stmt));
		}
	}
	else if (colType == SQLITE_NULL)
	{
		*retVal = 0;
	}
	else
	{
		//throw RVS::DataManagement::DataTypeMismatchException(sqlite3_sql(stmt));
	}
}

void RVS::DataManagement::DIO::query_equation_coefficients(int equation_number, double* coefs)
{
	// Get the datatable object for the requested equation number
	RVS::DataManagement::DataTable* dt = query_equation_table(equation_number);

	int column = 0;
	column = dt->Columns[EQN_COEF_1_FIELD];
	getVal(dt->getStmt(), column, &coefs[0]);
	column = dt->Columns[EQN_COEF_2_FIELD];
	getVal(dt->getStmt(), column, &coefs[1]);
	column = dt->Columns[EQN_COEF_3_FIELD];
	getVal(dt->getStmt(), column, &coefs[2]);
	column = dt->Columns[EQN_COEF_4_FIELD];
	getVal(dt->getStmt(), column, &coefs[3]);
}

void RVS::DataManagement::DIO::query_equation_parameters(int equation_number, std::string* params)
{
	// Get the datatable object for the requested equation number
	RVS::DataManagement::DataTable* dt = query_equation_table(equation_number);

	int column = 0;
	column = dt->Columns[EQN_P1_FIELD];
	getVal(dt->getStmt(), column, &params[0]);
	column = dt->Columns[EQN_P2_FIELD];
	getVal(dt->getStmt(), column, &params[1]);
	column = dt->Columns[EQN_P3_FIELD];
	getVal(dt->getStmt(), column, &params[2]);
}

void RVS::DataManagement::DIO::query_equation_parameters(int equation_number, std::string* params, double* coefs)
{
	// Get the datatable object for the requested equation number
	RVS::DataManagement::DataTable* dt = query_equation_table(equation_number);

	int column = 0;
	column = dt->Columns[EQN_P1_FIELD];
	getVal(dt->getStmt(), column, &params[0]);
	column = dt->Columns[EQN_P2_FIELD];
	getVal(dt->getStmt(), column, &params[1]);
	column = dt->Columns[EQN_P3_FIELD];
	getVal(dt->getStmt(), column, &params[2]);

	column = dt->Columns[EQN_COEF_1_FIELD];
	getVal(dt->getStmt(), column, &coefs[0]);
	column = dt->Columns[EQN_COEF_2_FIELD];
	getVal(dt->getStmt(), column, &coefs[1]);
	column = dt->Columns[EQN_COEF_3_FIELD];
	getVal(dt->getStmt(), column, &coefs[2]);
	column = dt->Columns[EQN_COEF_4_FIELD];
	getVal(dt->getStmt(), column, &coefs[3]);
}

void RVS::DataManagement::DIO::query_equation_parameters(int equation_number, std::string* params, double* coefs, int* equation_type)
{
	// Get the datatable object for the requested equation number
	RVS::DataManagement::DataTable* dt = query_equation_table(equation_number);

	int column = 0;
	column = dt->Columns[EQN_P1_FIELD];
	getVal(dt->getStmt(), column, &params[0]);
	column = dt->Columns[EQN_P2_FIELD];
	getVal(dt->getStmt(), column, &params[1]);
	column = dt->Columns[EQN_P3_FIELD];
	getVal(dt->getStmt(), column, &params[2]);

	column = dt->Columns[EQN_COEF_1_FIELD];
	getVal(dt->getStmt(), column, &coefs[0]);
	column = dt->Columns[EQN_COEF_2_FIELD];
	getVal(dt->getStmt(), column, &coefs[1]);
	column = dt->Columns[EQN_COEF_3_FIELD];
	getVal(dt->getStmt(), column, &coefs[2]);
	column = dt->Columns[EQN_COEF_4_FIELD];
	getVal(dt->getStmt(), column, &coefs[3]);

	column = dt->Columns[EQUATION_TYPE_FIELD];
	getVal(dt->getStmt(), column, equation_type);
}

void RVS::DataManagement::DIO::query_fuels_basic_info(const int* bps, int* fbfm, bool* isDry)
{
	const char* sql = query_base(FUEL_BPS_ATTR_TABLE, BPS_NUM_FIELD, *bps);
	RVS::DataManagement::DataTable* dt = prep_datatable(sql, rvsdb, true, true);

	int column = 0;
	column = dt->Columns[FC_FBFM_FIELD];
	getVal(dt->getStmt(), column, fbfm);
	column = dt->Columns[FC_ISDRY_FIELD];
	getVal(dt->getStmt(), column, isDry);

	if (*RC == 101)
	{
		stringstream* s = new stringstream();
		*s << "Fuels input not found for BPS " << *bps << ", assuming DRY climate";
		const char* c = streamToCharPtr(s);
		write_debug_msg(c);

		*isDry = true;
	}
}

int* RVS::DataManagement::DIO::finalizeQueries(void)
{	
	for (auto &q : activeQueries)
	{
		q.second.reset();
	}
	activeQueries.clear();

	return RC;
}

char* RVS::DataManagement::DIO::streamToCharPtr(stringstream* stream)
{
	// Get the string representation of the stream
	string nstring = stream->str();
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

bool RVS::DataManagement::DIO::isQueryActive(string sql)
{
	bool ret = false;
	map<string, shared_ptr<DataTable>>::iterator it = activeQueries.find(sql);
	if (it != activeQueries.end())
	{
		ret = true;
	}
	return ret;
}

bool RVS::DataManagement::DIO::checkDBStatus(sqlite3* db, const char* sql, const char* err)
{
	bool state = true;
	if (!(*RC == SQLITE_OK || *RC == SQLITE_ROW) && db != NULL)
	{
		state = false;
		write_debug_msg(sql);
		if (*RC == SQLITE_DONE)
		{
			write_debug_msg("Data not found");
		}
		else
		{
			write_debug_msg(sqlite3_errmsg(db));
		}
	}
	
	return state;
}

// Lifted straight from sqlite website
int RVS::DataManagement::DIO::buildInMemDB(sqlite3 *pInMemory, const char *zFilename, int isSave)
{
	int rc;                   /* Function return code */
	sqlite3 *pFile;           /* Database connection opened on zFilename */
	sqlite3_backup *pBackup;  /* Backup object used to copy data */
	sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
	sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

	/* Open the database file identified by zFilename. Exit early if this fails
	** for any reason. */
	rc = sqlite3_open(zFilename, &pFile);
	if (rc == SQLITE_OK)
	{
		/* If this is a 'load' operation (isSave==0), then data is copied
		** from the database file just opened to database pInMemory.
		** Otherwise, if this is a 'save' operation (isSave==1), then data
		** is copied from pInMemory to pFile.  Set the variables pFrom and
		** pTo accordingly. */
		pFrom = (isSave ? pInMemory : pFile);
		pTo = (isSave ? pFile : pInMemory);

		/* Set up the backup procedure to copy from the "main" database of
		** connection pFile to the main database of connection pInMemory.
		** If something goes wrong, pBackup will be set to NULL and an error
		** code and  message left in connection pTo.
		**
		** If the backup object is successfully created, call backup_step()
		** to copy data from pFile to pInMemory. Then call backup_finish()
		** to release resources associated with the pBackup object.  If an
		** error occurred, then  an error code and message will be left in
		** connection pTo. If no error occurred, then the error code belonging
		** to pTo is set to SQLITE_OK.
		*/
		pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
		if (pBackup)
		{
			(void)sqlite3_backup_step(pBackup, -1);
			(void)sqlite3_backup_finish(pBackup);
		}
		rc = sqlite3_errcode(pTo);
	}

	/* Close the database connection opened on database file zFilename
	** and return the result of this function. */
	(void)sqlite3_close(pFile);
	return rc;
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_equation_table(int equation_number)
{
	return nullptr;
}