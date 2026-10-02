/// ********************************************************** ///
/// Name: DIO.cpp                                              ///
/// Desc: Data Input/output implementation for RVS.            ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#include "DIO.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <iostream>

// Static member definitions
sqlite3* RVS::DataManagement::DIO::rvsdb = nullptr;
sqlite3* RVS::DataManagement::DIO::outdb = nullptr;

std::map<std::string, std::shared_ptr<RVS::DataManagement::DataTable>> RVS::DataManagement::DIO::activeQueries;
std::vector<std::string> RVS::DataManagement::DIO::queuedWrites;

// Constructor
RVS::DataManagement::DIO::DIO(void)
{
	if (rvsdb == nullptr)
	{
		if (!(*USE_MEM))
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

	if (outdb == nullptr)
	{
		RC = create_output_db();
	}
	
	checkDBStatus(outdb);
}

// Destructor. Closes connection with the database.
RVS::DataManagement::DIO::~DIO(void)
{
	RC = finalizeQueries();
	if (rvsdb != nullptr)
	{
		*RC = sqlite3_close(rvsdb);
		rvsdb = nullptr;
		
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
	
	if (outdb != nullptr)
	{
		*RC = sqlite3_close(outdb);
		outdb = nullptr;

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

// Opens an SQLite connection to the specified database
int* RVS::DataManagement::DIO::open_db_connection(const char* pathToDb, sqlite3** db)
{
	*RC = sqlite3_open(pathToDb, db);
	checkDBStatus(*db);

	if (*RC != SQLITE_OK)
	{
		std::fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
		sqlite3_close(*db);
	}

	sqlite3_exec(*db, "PRAGMA synchronous = OFF", nullptr, nullptr, nullptr);
	sqlite3_exec(*db, "PRAGMA journal_mode = MEMORY", nullptr, nullptr, nullptr);

	return RC;
}

int* RVS::DataManagement::DIO::create_output_db()
{
	RC = create_output_db(OUT_DB_PATH);
	return RC;
}

int* RVS::DataManagement::DIO::create_output_db(const char* path)
{
	*RC = std::remove(path);
	RC = open_db_connection(path, &outdb);
	return RC;
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::prep_datatable(const char* sql, sqlite3* db, bool addToActive, bool reset)
{
	std::shared_ptr<DataTable> dt;
	if (isQueryActive(sql))
	{
		dt = activeQueries[sql];
		if (reset)
		{
			*RC = sqlite3_reset(dt->getStmt());
			*RC = sqlite3_step(dt->getStmt());
			*(dt->STATUS()) = *RC;
		}
		else
		{
			*RC = *(dt->STATUS());
		}
	}
	else
	{
		constexpr int nByte = -1;
		sqlite3_stmt* stmt = nullptr;

		// Prepare SQL query as object code
		*RC = sqlite3_prepare_v2(db, sql, nByte, &stmt, nullptr);
		*RC = sqlite3_step(stmt);
		dt = std::make_shared<DataTable>(stmt);

		if (addToActive)
		{
			activeQueries.emplace(sql, dt);
		}

		int* const rc = dt->STATUS();
		*rc = *RC;
	}

	return dt.get();
}

int* RVS::DataManagement::DIO::write_output(void)
{
	char* err = nullptr;
	*RC = sqlite3_exec(outdb, "BEGIN TRANSACTION", nullptr, nullptr, &err);
	if (err)
	{
		checkDBStatus(outdb, "BEGIN TRANSACTION", err);
		sqlite3_free(err);
		err = nullptr;
	}

	for (size_t w = 0; w < queuedWrites.size(); ++w)
	{
		const char* const sql = queuedWrites.at(w).c_str();
		*RC = sqlite3_exec(outdb, sql, nullptr, nullptr, &err);
		checkDBStatus(outdb, sql, err);
		if (err)
		{
			sqlite3_free(err);
			err = nullptr;
		}
	}

	*RC = sqlite3_exec(outdb, "END TRANSACTION", nullptr, nullptr, &err);
	checkDBStatus(outdb, "END TRANSACTION", err);
	if (err)
	{
		sqlite3_free(err);
		err = nullptr;
	}

	queuedWrites.clear();

	return RC;
}

void RVS::DataManagement::DIO::write_debug_msg(const char* msg)
{
	const std::time_t t = std::time(nullptr);
	char strTime[25];
	std::strftime(strTime, sizeof(strTime), "%d/%m/%y %H:%M:%S", std::localtime(&t));

	std::ofstream dfile(DEBUG_FILE, std::ios::app);
	if (dfile.is_open())
	{
		dfile << strTime << " " << (msg ? msg : "") << '\n';
	}
}

std::vector<int> RVS::DataManagement::DIO::query_analysis_plots()
{
	std::stringstream selectStream;
	selectStream << "SELECT DISTINCT " << PLOT_NUM_FIELD << " FROM " << RVS_INPUT_TABLE << "; ";
	
	const char* const selectString = streamToCharPtr(&selectStream);
	RVS::DataManagement::DataTable* const dt = prep_datatable(selectString, rvsdb);
	
	std::vector<int> items;
	int plot = 0;
	while (*RC == SQLITE_ROW)
	{
		getVal(dt->getStmt(), 0, &plot);
		items.push_back(plot);
		*RC = sqlite3_step(dt->getStmt());
	}

	return items;
}

const char* RVS::DataManagement::DIO::query_base(const char* table)
{
	std::stringstream selectStream;
	selectStream << "SELECT * FROM " << table << "; ";
	return streamToCharPtr(&selectStream);
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field)
{
	std::stringstream selectStream;
	selectStream << "SELECT " << field << " FROM " << table << "; ";
	return streamToCharPtr(&selectStream);
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field, int whereclause)
{
	std::stringstream selectStream;
	selectStream << "SELECT * FROM " << table << " WHERE " << field << "=" << whereclause << "; ";
	return streamToCharPtr(&selectStream);
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field, const std::string& whereclause)
{
	std::stringstream selectStream;
	selectStream << "SELECT * FROM " << table << " WHERE " << field << "='" << whereclause << "'; ";
	return streamToCharPtr(&selectStream);
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field, int whereclause, const std::string& order)
{
	std::stringstream selectStream;
	selectStream << "SELECT * FROM " << table << " WHERE " << field << "=" << whereclause
	             << " ORDER BY " << order << ";";
	return streamToCharPtr(&selectStream);
}

const char* RVS::DataManagement::DIO::query_base(const char* table, const char* field, const std::string& whereclause, const std::string& order)
{
	std::stringstream selectStream;
	selectStream << "SELECT * FROM " << table << " WHERE " << field << "='" << whereclause << "'"
	             << " ORDER BY " << order << ";";
	return streamToCharPtr(&selectStream);
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_input_table(void)
{
	std::stringstream sqlStream;
	sqlStream << "SELECT * FROM " << RVS_INPUT_TABLE << ";";
	return prep_datatable(streamToCharPtr(&sqlStream), rvsdb);
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_input_table(int plot_num)
{
	const char* const sql = query_base(RVS_INPUT_TABLE, PLOT_NUM_FIELD, plot_num);
	return prep_datatable(sql, rvsdb, false);
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_shrubs_table(void)
{
	std::stringstream sqlStream;
	sqlStream << "SELECT * FROM " << SHRUB_INPUT_TABLE << ";";
	return prep_datatable(streamToCharPtr(&sqlStream), rvsdb, true);
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, std::any* retval)
{
	if (!retval) return;
	const int colType = sqlite3_column_type(stmt, column);
	switch (colType)
	{
	case SQLITE_INTEGER:
		*retval = sqlite3_column_int(stmt, column);
		break;
	case SQLITE_FLOAT:
		*retval = sqlite3_column_double(stmt, column);
		break;
	case SQLITE_BLOB:
	case SQLITE_NULL:
		*retval = 0.0;
		break;
	case SQLITE3_TEXT:
		*retval = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, column)));
		break;
	default:
		*retval = 0.0;
		break;
	}
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, double* retVal)
{
	if (!retVal) return;
	const int colType = sqlite3_column_type(stmt, column);
	if (colType == SQLITE_FLOAT)
	{
		*retVal = sqlite3_column_double(stmt, column);
	}
	else if (colType == SQLITE_NULL)
	{
		*retVal = 0.0;
	}
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, std::string* retVal)
{
	if (!retVal) return;
	const int colType = sqlite3_column_type(stmt, column);
	if (colType == SQLITE3_TEXT)
	{
		*retVal = reinterpret_cast<const char*>(sqlite3_column_text(stmt, column));
	}
	else if (colType == SQLITE_NULL)
	{
		*retVal = "";
	}
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, int* retVal)
{
	if (!retVal) return;
	const int colType = sqlite3_column_type(stmt, column);
	if (colType == SQLITE_INTEGER)
	{
		*retVal = sqlite3_column_int(stmt, column);
	}
	else if (colType == SQLITE_NULL)
	{
		*retVal = 0;
	}
}

void RVS::DataManagement::DIO::getVal(sqlite3_stmt* stmt, int column, bool* retVal)
{
	if (!retVal) return;
	const int colType = sqlite3_column_type(stmt, column);
	if (colType == SQLITE_INTEGER)
	{
		const int val = sqlite3_column_int(stmt, column);
		*retVal = (val == 1);
	}
	else if (colType == SQLITE_NULL)
	{
		*retVal = false;
	}
}

void RVS::DataManagement::DIO::query_equation_coefficients(int equation_number, std::span<double> coefs)
{
	RVS::DataManagement::DataTable* const dt = query_equation_table(equation_number);
	if (!dt || coefs.size() < 4) return;

	int column = dt->Columns[EQN_COEF_1_FIELD];
	getVal(dt->getStmt(), column, &coefs[0]);
	column = dt->Columns[EQN_COEF_2_FIELD];
	getVal(dt->getStmt(), column, &coefs[1]);
	column = dt->Columns[EQN_COEF_3_FIELD];
	getVal(dt->getStmt(), column, &coefs[2]);
	column = dt->Columns[EQN_COEF_4_FIELD];
	getVal(dt->getStmt(), column, &coefs[3]);
}

void RVS::DataManagement::DIO::query_equation_coefficients(int equation_number, double* coefs)
{
	if (coefs)
	{
		query_equation_coefficients(equation_number, std::span<double>(coefs, 4));
	}
}

void RVS::DataManagement::DIO::query_equation_parameters(int equation_number, std::span<std::string> params)
{
	RVS::DataManagement::DataTable* const dt = query_equation_table(equation_number);
	if (!dt || params.size() < 3) return;

	int column = dt->Columns[EQN_P1_FIELD];
	getVal(dt->getStmt(), column, &params[0]);
	column = dt->Columns[EQN_P2_FIELD];
	getVal(dt->getStmt(), column, &params[1]);
	column = dt->Columns[EQN_P3_FIELD];
	getVal(dt->getStmt(), column, &params[2]);
}

void RVS::DataManagement::DIO::query_equation_parameters(int equation_number, std::string* params)
{
	if (params)
	{
		query_equation_parameters(equation_number, std::span<std::string>(params, 3));
	}
}

void RVS::DataManagement::DIO::query_equation_parameters(int equation_number, std::span<std::string> params, std::span<double> coefs)
{
	query_equation_parameters(equation_number, params);
	query_equation_coefficients(equation_number, coefs);
}

void RVS::DataManagement::DIO::query_equation_parameters(int equation_number, std::string* params, double* coefs)
{
	if (params && coefs)
	{
		query_equation_parameters(equation_number, std::span<std::string>(params, 3), std::span<double>(coefs, 4));
	}
}

void RVS::DataManagement::DIO::query_equation_parameters(int equation_number, std::span<std::string> params, std::span<double> coefs, int* equation_type)
{
	query_equation_parameters(equation_number, params, coefs);
	if (equation_type)
	{
		RVS::DataManagement::DataTable* const dt = query_equation_table(equation_number);
		if (dt)
		{
			const int column = dt->Columns[EQUATION_TYPE_FIELD];
			getVal(dt->getStmt(), column, equation_type);
		}
	}
}

void RVS::DataManagement::DIO::query_equation_parameters(int equation_number, std::string* params, double* coefs, int* equation_type)
{
	if (params && coefs)
	{
		query_equation_parameters(equation_number, std::span<std::string>(params, 3), std::span<double>(coefs, 4), equation_type);
	}
}

void RVS::DataManagement::DIO::query_fuels_basic_info(const int* bps, int* fbfm, bool* isDry)
{
	if (!bps) return;
	const char* const sql = query_base(FUEL_BPS_ATTR_TABLE, BPS_NUM_FIELD, *bps);
	RVS::DataManagement::DataTable* const dt = prep_datatable(sql, rvsdb, true, true);
	if (!dt) return;

	int column = dt->Columns[FC_FBFM_FIELD];
	getVal(dt->getStmt(), column, fbfm);
	column = dt->Columns[FC_ISDRY_FIELD];
	getVal(dt->getStmt(), column, isDry);

	if (*RC == 101)
	{
		std::stringstream s;
		s << "Fuels input not found for BPS " << *bps << ", assuming DRY climate";
		const char* const c = streamToCharPtr(&s);
		write_debug_msg(c);

		if (isDry) *isDry = true;
	}
}

int* RVS::DataManagement::DIO::finalizeQueries(void)
{	
	for (auto& q : activeQueries)
	{
		q.second.reset();
	}
	activeQueries.clear();

	return RC;
}

char* RVS::DataManagement::DIO::streamToCharPtr(std::stringstream* stream)
{
	// Bounded 16-slot ring buffer providing valid C-string pointers across function calls
	// without dynamic heap allocation leaks or deallocation hazards.
	static std::array<std::string, 16> ring;
	static size_t ring_idx = 0;
	ring_idx = (ring_idx + 1) % ring.size();
	ring[ring_idx] = stream ? stream->str() : std::string();
	return ring[ring_idx].data();
}

int RVS::DataManagement::DIO::callback(void* nu, int argc, char** argv, char** azColName)
{
	return 0;
}

bool RVS::DataManagement::DIO::isQueryActive(const std::string& sql)
{
	return activeQueries.contains(sql);
}

bool RVS::DataManagement::DIO::checkDBStatus(sqlite3* db, const char* sql, const char* err)
{
	bool state = true;
	if (!(*RC == SQLITE_OK || *RC == SQLITE_ROW) && db != nullptr)
	{
		state = false;
		if (sql && sql[0] != '\0')
		{
			write_debug_msg(sql);
		}
		if (err && err[0] != '\0')
		{
			write_debug_msg(err);
		}
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

// lifted straight from SQLite backup API
int RVS::DataManagement::DIO::buildInMemDB(sqlite3 *pInMemory, const char *zFilename, int isSave)
{
	int rc;
	sqlite3 *pFile = nullptr;
	sqlite3_backup *pBackup = nullptr;
	sqlite3 *pTo = nullptr;
	sqlite3 *pFrom = nullptr;

	rc = sqlite3_open(zFilename, &pFile);
	if (rc == SQLITE_OK)
	{
		pFrom = (isSave ? pInMemory : pFile);
		pTo = (isSave ? pFile : pInMemory);

		pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
		if (pBackup)
		{
			(void)sqlite3_backup_step(pBackup, -1);
			(void)sqlite3_backup_finish(pBackup);
		}
		rc = sqlite3_errcode(pTo);
	}

	(void)sqlite3_close(pFile);
	return rc;
}

RVS::DataManagement::DataTable* RVS::DataManagement::DIO::query_equation_table(int equation_number)
{
	return nullptr;
}