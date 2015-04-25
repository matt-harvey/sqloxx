/*
 * Copyright 2012, 2013 Matthew Harvey
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

#include "sqloxx_exceptions.hpp"
#include "detail/sqlite_dbconn.hpp"
#include "detail/sqlite3.h" // Compiling directly into build
#include <boost/filesystem.hpp>
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

namespace
{
    // Ensures sqlite3_initialize() is called exactly once and
    // sqlite3_shutdown() is called exactly once.
    class SQLiteController
    {
    private:
        SQLiteController()
        {
            JEWEL_LOG_TRACE();
            if (sqlite3_initialize() != SQLITE_OK)
            {
                JEWEL_LOG_TRACE();
                JEWEL_THROW
                (   SQLiteInitializationError,
                    "SQLite could not be initialized."
                );
            }
            JEWEL_LOG_TRACE();
        }
        ~SQLiteController()
        {
            JEWEL_LOG_TRACE();
            if (sqlite3_shutdown() != SQLITE_OK)
            {
                JEWEL_LOG_TRACE();
                clog << "SQLite3 shutdown failed." << endl;
                terminate();
            }
            JEWEL_LOG_TRACE();
        }
    public:
        static void register_connection()
        {
            // This is thread-safe when compiled with C++11
            static SQLiteController dummy;
            (void)dummy;  // silence compiler re. unused variable
        }
    };

}  // end anonymous namespace

SQLiteDBConn::SQLiteDBConn(): m_connection(nullptr)
{
    SQLiteController::register_connection();
}

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
}

bool
SQLiteDBConn::is_valid() const
{
    return m_connection != nullptr;
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
        JEWEL_THROW(MultipleConnectionException, "Database already connected.");
    }
    // Open the connection
    throw_on_failure    
    (   sqlite3_open_v2
        (   filepath.generic_string().c_str(),
            &m_connection,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
            nullptr
        )
    );
    execute_sql("pragma foreign_keys = on;");
    return;
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
    case SQLITE_OK:             return;
    case SQLITE_DONE:           return;
    case SQLITE_ROW:            return;
    default:                    break;
    }
    if (errcode != sqlite3_errcode(m_connection))
    {
        throw logic_error
        (   "Parameter errcode passed to throw_on_failure does not correspond"
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
    JEWEL_ASSERT (msg != nullptr);
    JEWEL_ASSERT (errcode == sqlite3_errcode(m_connection));
    switch (errcode)
    {
    case SQLITE_ERROR:         JEWEL_THROW(SQLiteError, msg);
    case SQLITE_INTERNAL:      JEWEL_THROW(SQLiteInternal, msg);
    case SQLITE_PERM:          JEWEL_THROW(SQLitePerm, msg);
    case SQLITE_ABORT:         JEWEL_THROW(SQLiteAbort, msg);
    case SQLITE_BUSY:          JEWEL_THROW(SQLiteBusy, msg);
    case SQLITE_LOCKED:        JEWEL_THROW(SQLiteLocked, msg);
    case SQLITE_NOMEM:         JEWEL_THROW(SQLiteNoMem, msg);
    case SQLITE_READONLY:      JEWEL_THROW(SQLiteReadOnly, msg);
    case SQLITE_INTERRUPT:     JEWEL_THROW(SQLiteInterrupt, msg);
    case SQLITE_IOERR:         JEWEL_THROW(SQLiteIOErr, msg);
    case SQLITE_CORRUPT:       JEWEL_THROW(SQLiteCorrupt, msg);
    case SQLITE_NOTFOUND:      JEWEL_THROW(SQLiteNotFound, msg);
    case SQLITE_FULL:          JEWEL_THROW(SQLiteFull, msg);
    case SQLITE_CANTOPEN:      JEWEL_THROW(SQLiteCantOpen, msg);
    case SQLITE_PROTOCOL:      JEWEL_THROW(SQLiteProtocol, msg);
    case SQLITE_EMPTY:         JEWEL_THROW(SQLiteEmpty, msg);
    case SQLITE_SCHEMA:        JEWEL_THROW(SQLiteSchema, msg);
    case SQLITE_TOOBIG:        JEWEL_THROW(SQLiteTooBig, msg);
    case SQLITE_CONSTRAINT:    JEWEL_THROW(SQLiteConstraint, msg);
    case SQLITE_MISMATCH:      JEWEL_THROW(SQLiteMismatch, msg);
    case SQLITE_MISUSE:        JEWEL_THROW(SQLiteMisuse, msg);
    case SQLITE_NOLFS:         JEWEL_THROW(SQLiteNoLFS, msg);
    case SQLITE_AUTH:          JEWEL_THROW(SQLiteAuth, msg);
    case SQLITE_FORMAT:        JEWEL_THROW(SQLiteFormat, msg);
    case SQLITE_RANGE:         JEWEL_THROW(SQLiteRange, msg);
    case SQLITE_NOTADB:        JEWEL_THROW(SQLiteNotADB, msg);

#   ifndef NDEBUG
        case SQLITE_OK:        JEWEL_HARD_ASSERT (false);
        case SQLITE_ROW:       JEWEL_HARD_ASSERT (false);
        case SQLITE_DONE:      JEWEL_HARD_ASSERT (false);
#   endif

    default:                   JEWEL_THROW(SQLiteUnknownErrorCode, msg);
    }
    JEWEL_HARD_ASSERT (false);  // Execution should never reach here.
}

void
SQLiteDBConn::execute_sql(string const& str)
{
    throw_on_failure
    (   sqlite3_exec(m_connection,str.c_str(), nullptr, nullptr, nullptr)
    );
    return;
}

}  // namespace detail
}  // namespace sqloxx
