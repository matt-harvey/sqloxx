
/** \file sqlite_dbconn.cpp
 *
 * \brief Source file pertaining to SQLiteDBConn class.
 *
 * \author Matthew Harvey
 * \date 04 July 2012.
 *
 * Copyright (c) 2012, Matthew Harvey. All rights reserved.
 */

#include "sqloxx_exceptions.hpp"
#include "detail/sqlite_dbconn.hpp"
#include <boost/filesystem.hpp>
#include "sqlite3.h" // Compiling directly into build
#include <jewel/assert.hpp>
#include <jewel/exception.hpp>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using std::terminate;
using std::clog;
using std::endl;
using std::logic_error;
using std::runtime_error;
using std::string;
using std::vector;


namespace sqloxx
{
namespace detail
{


SQLiteDBConn::SQLiteDBConn():
	m_connection(0)
{
	// Initialize SQLite3
	if (sqlite3_initialize() != SQLITE_OK)
	{
		JEWEL_THROW
		(	SQLiteInitializationError,
			"SQLite could not be initialized."
		);
	}

}


void
SQLiteDBConn::open(boost::filesystem::path const& filepath)
{
	if (filepath.string().empty())
	{
		JEWEL_THROW(InvalidFilename, "Cannot open file with empty filename.");
	}
	// Throw if already connected or if filename is empty
	if (m_connection)
	{
		JEWEL_THROW
		(	MultipleConnectionException,
			"Database already connected."
		);
	}
	// Open the connection
	throw_on_failure	
	(	sqlite3_open_v2
		(	filepath.string().c_str(),
			&m_connection,
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
			0
		)
	);
	execute_sql("pragma foreign_keys = on;");
	return;

}


// Remember - don't throw exceptions from destructors!
// Remember - don't call virtual functions from destructors!
SQLiteDBConn::~SQLiteDBConn()
{

	if (m_connection)
	{
		if (sqlite3_close(m_connection) != SQLITE_OK)
		{
			clog << "SQLite3 database connection could not be "
			             "successfully "
			             "closed in SQLiteDBConn destructor. " << endl;
			terminate();
		}
	}
	if (sqlite3_shutdown() != SQLITE_OK)
	{
		clog << "SQLite3 shutdown failed in SQLiteDBConn destructor."
		     << endl;
		terminate();
	}
}

bool
SQLiteDBConn::is_valid() const
{
	return m_connection != 0;
}

void
SQLiteDBConn::throw_on_failure(int errcode)
{
	if (!is_valid())
	{
		JEWEL_THROW(InvalidConnection, "Database connection is invalid.");
	}
	switch (errcode)
	{
	case SQLITE_OK:
	case SQLITE_DONE:
	case SQLITE_ROW:
		return;
	default:
		break;
	}
	if (errcode != sqlite3_errcode(m_connection))
	{
		throw logic_error
		(	"Parameter errcode passed to throw_on_failure does not correspond"
			" to error code produced by latest call to SQLite API on this "
			"database connection."
		);
	}
	JEWEL_ASSERT (errcode != SQLITE_OK);
	JEWEL_ASSERT (sqlite3_errcode(m_connection) != SQLITE_OK);
	char const* msg = sqlite3_errmsg(m_connection);
	if (!msg)
	{
		JEWEL_THROW(SQLiteException, "");  // Keep it minimal in this case.
	}
	JEWEL_ASSERT (msg != 0);
	JEWEL_ASSERT (errcode == sqlite3_errcode(m_connection));
	switch (errcode)
	{
	// Redundant "breaks" here are retained deliberately out of "respect".
	case SQLITE_ERROR:
		JEWEL_THROW(SQLiteError, msg);
		break;
	case SQLITE_INTERNAL:
		JEWEL_THROW(SQLiteInternal, msg);
		break;
	case SQLITE_PERM:
		JEWEL_THROW(SQLitePerm, msg);
		break;
	case SQLITE_ABORT:
		JEWEL_THROW(SQLiteAbort, msg);
		break;
	case SQLITE_BUSY:
		JEWEL_THROW(SQLiteBusy, msg);
		break;
	case SQLITE_LOCKED:
		JEWEL_THROW(SQLiteLocked, msg);
		break;
	case SQLITE_NOMEM:
		JEWEL_THROW(SQLiteNoMem, msg);
		break;
	case SQLITE_READONLY:
		JEWEL_THROW(SQLiteReadOnly, msg);
		break;
	case SQLITE_INTERRUPT:
		JEWEL_THROW(SQLiteInterrupt, msg);
		break;
	case SQLITE_IOERR:
		JEWEL_THROW(SQLiteIOErr, msg);
		break;
	case SQLITE_CORRUPT:
		JEWEL_THROW(SQLiteCorrupt, msg);
		break;
	case SQLITE_FULL:
		JEWEL_THROW(SQLiteFull, msg);
		break;
	case SQLITE_CANTOPEN:
		JEWEL_THROW(SQLiteCantOpen, msg);
		break;
	case SQLITE_EMPTY:
		JEWEL_THROW(SQLiteEmpty, msg);
		break;
	case SQLITE_SCHEMA:
		JEWEL_THROW(SQLiteSchema, msg);
		break;
	case SQLITE_TOOBIG:
		JEWEL_THROW(SQLiteTooBig, msg);
		break;
	case SQLITE_CONSTRAINT:
		JEWEL_THROW(SQLiteConstraint, msg);
		break;
	case SQLITE_MISMATCH:
		JEWEL_THROW(SQLiteMismatch, msg);
		break;
	case SQLITE_MISUSE:
		JEWEL_THROW(SQLiteMisuse, msg);
		break;
	case SQLITE_NOLFS:
		JEWEL_THROW(SQLiteNoLFS, msg);
		break;
	case SQLITE_AUTH:
		JEWEL_THROW(SQLiteAuth, msg);
		break;
	case SQLITE_FORMAT:
		JEWEL_THROW(SQLiteFormat, msg);
		break;
	case SQLITE_RANGE:
		JEWEL_THROW(SQLiteRange, msg);
		break;
	case SQLITE_NOTADB:
		JEWEL_THROW(SQLiteNotADB, msg);
		break;

	#ifndef NDEBUG
		case SQLITE_OK:
		case SQLITE_ROW:
		case SQLITE_DONE:
			JEWEL_HARD_ASSERT (false);  // Should never reach here
	#endif

	default:
		JEWEL_THROW(SQLiteUnknownErrorCode, msg);
	}
	JEWEL_HARD_ASSERT (false);  // Execution should never reach here.
	return;
}


void
SQLiteDBConn::execute_sql(string const& str)
{
	throw_on_failure(sqlite3_exec(m_connection, str.c_str(), 0, 0, 0));
	return;
}



}  // namespace detail
}  // namespace sqloxx
