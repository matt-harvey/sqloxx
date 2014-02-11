/*
 * Copyright 2013 Matthew Harvey
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GUARD_sqlite_dbconn_hpp_8577399267152603
#define GUARD_sqlite_dbconn_hpp_8577399267152603

// Hide from Doxygen
/// @cond

/** @file
 *
 * @brief Header file pertaining to SQLiteDBConn class.
 */

#include "../sqloxx_exceptions.hpp"
#include "sql_statement_impl.hpp"
#include <jewel/checked_arithmetic.hpp>
#include "sqlite3.h"  // Compiling directly into build
#include <boost/filesystem/path.hpp>
#include <limits>
#include <string>
#include <vector>

namespace sqloxx
{
namespace detail
{

/**
 * @class SQLiteDBConn
 *
 * This class is designed to be used only by other classes within
 * the Sqloxx module, and should not be accessed by external client
 * code.
 *
 * This class is intended to encapsulate calls to the C API
 * provided by SQLite, in relation to managing a database connection
 * represented by a sqlite3* struct.
 *
 * SQLiteDBConn is in designed to be contained
 * within an instance of DatabaseConnection; and it is DatabaseConnection
 * that provides the higher-level interface with clients outside of
 * Sqloxx, as well as a range of convenience functions that are not
 * provided by this lower-level class.
 *
 * @todo MEDIUM PRIORITY Supply public member functions to close
 * any database connections and to shut down SQLite3. Current this is done
 * in the destructor, but destructors should not throw, so the destructor
 * calls std::terminate() if close or shut-down fails.
 */
class SQLiteDBConn
{
	friend class SQLStatementImpl;

public:

	/**
	 * Initializes SQLite3 and creates a database connection
	 * initially set to null.
	 *
	 * @throws SQLiteInitializationError if initialization fails
	 * for any reason.
	 */
	SQLiteDBConn();

	SQLiteDBConn(SQLiteDBConn const&) = delete;
	SQLiteDBConn(SQLiteDBConn&&) = delete;
	SQLiteDBConn& operator=(SQLiteDBConn const&) = delete;
	SQLiteDBConn& operator=(SQLiteDBConn&&) = delete;

	/**
	 * Closes any open SQLite3 database connection, and also
	 * shuts down SQLite3.
	 *
	 * Does not throw. If SQLite3 connection closure or shutdown fails,
	 * the application is aborted with a diagnostic message written to
	 * std::clog.
	 */
	~SQLiteDBConn();

	/**
	 * Returns \c true iff the SQLiteDBConn is currently connected to a 
	 * database file. Does not throw.
	 */
	bool is_valid() const;

	/**
	 * Implements DatabaseConnection::open.
	 */
	void open(boost::filesystem::path const& filepath);	

	/**
	 * Implements DatabaseConnection::execute_sql
	 */
	void execute_sql(std::string const& str);

	/**
	 * At this point this function does not fully support SQLite extended
	 * error codes; only the basic error codes. If errcode is an extended
	 * error code that is not also a basic error code, and is not
	 * SQLITE_OK, SQLITE_DONE or SQLITE_ROW, then the function will
	 * throw SQLiteUnknownErrorCode. If errcode is a basic error code that
	 * is not SQLITE_OK, SQLITE_DONE or SQLITE_ROW, then it will throw an
	 * exception derived from
	 * SQLiteException, with the exception thrown corresponding to the
	 * error code (see sqloxx_exceptions.hpp) and the error message returned
	 * by called what() on the exception corresponding to the error message
	 * produced by SQLite.
	 *
	 * If the database connection is invalid (in particular, if the connection
	 * is not open), then InvalidConnection will always be thrown, regardless
	 * of the value of errcode.
	 *
	 * errcode should be the return value of an operation just executed on
	 * the SQLite API on this database connection. The function assumes that
	 * no other operation has been executed on the API since the operation
	 * that produced errcode.
	 *
	 * @throws InvalidConnection if the database connection is invalid. This
	 * takes precedence over other exceptions that might be thrown.
	 *
	 * @throws an exception derived from SQLiteException if and only if
	 * errcode is something other than SQLITE_OK, BUT
	 *
	 * @throws std::logic_error if errcode is not the latest error code
	 * produced by a call to the SQLite API on this database connection.
	 *
	 * @param a SQLite error code.
	 *
	 * <b>Exception safety</b>: <em>strong guarantee</em>.
	 */
	void throw_on_failure(int errcode);

private:

	
	/**
	 * A connection to a SQLite3 database file.
	 *
	 * (Note this is a raw pointer not a smart pointer
	 * to facilitate more straightforward interaction with the SQLite
	 * C API.)
	 */
	sqlite3* m_connection;


};




}  // namespace detail
}  // namespace sqloxx


/// @endcond
// End hiding from Doxygen

#endif  // GUARD_sqlite_dbconn_hpp_8577399267152603
