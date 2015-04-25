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

#ifndef GUARD_sql_statement_hpp_9859693450787893
#define GUARD_sql_statement_hpp_9859693450787893

#include "database_connection.hpp"
#include "detail/sql_statement_impl.hpp"
#include <memory>
#include <string>

namespace sqloxx
{

/** Represents an SQL statement.
 *
 * Sqloxx transparently implements a
 * caching mechanism whereby the underlying structure associated with a
 * given statement string is cached for later re-use, avoiding the
 * runtime expense associated with parsing the same string and preparing the
 * same underlying data structure multiple times. To access this caching
 * mechanism, the client needs only to use the SQLStatement class,
 * and the caching/retrieval is done automatically. Each time the client
 * constructs an SQLStatement from a given string "xyz", Sqloxx
 * looks in the cache to see if "xyz" has been parsed before. If it has,
 * then, providing the corresponding
 * SQL statement structure is not currently "checked out" i.e. in use
 * elsewhere, then the existing structure is used rather than parsing
 * the string anew. The caching is managed via the DatabaseConnection
 * class. The maximum number of statement structures that can
 * be cached is determined by the value passed to \e p_cache_capacity in
 * the DatabaseConnection constructor. Once the cache maximum is reached,
 * additional statement strings are no longer added to the cache, but
 * are parsed anew each time (which, as far as client code is concerned,
 * has a performance impact, but not a behavioural impact).
 *
 * If an exception is thrown by a method of SQLStatement, the
 * caller should in general no longer rely on the state of SQLStatement
 * being valid. However, when the SQLStatement goes out of scope or
 * is otherwise destroyed, the underlying structure will be reset to a
 * valid state. Furthermore, a locking mechanism ensures that two
 * SQLStatements cannot simultanously share the same underlying structure. This
 * prevents statement structures that are in an invalid state
 * from being used unless used via the very same SQLStatement that triggered
 * the invalid state.
 *
 * To take best advantage of the caching mechanism, and to minimize the
 * potential for errors and SQL injection exploits, client code should
 * use the bind() functions as much as possible, for introducing specific
 * data into SQL statements, rather than hard-coding data into the statement
 * text. (The caching mechanism stores statement structures keyed by
 * their text as it is before any data is bound into the statement.) See
 * documentation for bind() for more detail.
 */
class SQLStatement
{
public:

    /**
     * Creates an object representing a single SQL statement.
     * 
     * @param p_database_connection is the DatabaseConnection
     * on which the statement will be executed.
     *
     * @param p_statement_text is the text of a single SQL statement. It can be
     * terminated with any mixture of semicolons and/or spaces (but not other
     * forms of whitespace).
     *
     * @throws InvalidConnection if \e p_database_connection is an
     * invalid database connection (i.e. if \e p_database_connection.is_valid()
     * returns false).
     *
     * @throws SQLiteException or an exception derived therefrom, if there
     * is some other problem in preparing the statement, which results in a
     * SQLite error code (that is not \e SQLITE_OK) being returned.
     * 
     * @throws TooManyStatements if the first purported SQL statement
     * in \e p_statement_text is syntactically acceptable to SQLite, \e but
     * there are characters in \e p_statement_text after this statement, other
     * than ';' and ' '.
     * This includes the case where there are further syntactically
     * acceptable SQL statements after the first one - as each
     * SQLStatement can encapsulate only one statement.
     *
     * @throws std::bad_alloc in the very unlikely event of a memory
     * allocation error in execution.
     *
     * <b>Exception safety</b>: <em>strong guarantee</em>.
     */
    SQLStatement
    (   DatabaseConnection& p_database_connection,    
        std::string const& p_statement_text
    );

    // TODO LOW PRIORITY Provide a move constructor (even though
    // I don't want to provide a copy constructor)?
    SQLStatement(SQLStatement const&) = delete;
    SQLStatement(SQLStatement&&) = delete;
    SQLStatement& operator=(SQLStatement const&) = delete;
    SQLStatement& operator=(SQLStatement&&) = delete;

    /**
     * Destructor "clears" the state of the underlying cached
     * SQL statement structure ready for re-use by a subsequent SQLStatement
     * with the same statement text. (Client code does not need to concern
     * itself with the details of this.)
     *
     * <b>Exception safety</b>: <em>nothrow guarantee</em>.
     */
    ~SQLStatement();
    
    /**
     * Wrapper around SQLite "bind" functions for binding named
     * parameters with data.
     *
     * This is only supported with the following types for \b T: \n
     * \b int, \b long, <b>long long</b>, \b double,
     * <b>std::string const&</b> and <b>char const*</b>.
     *
     * Example usage: \n\n
     * <tt>
       *    SQLStatement statement(dbc, "select name from players where score > :score");\n
      *    statement.bind(":score", 50000);
      * </tt>
     *
     * This produces a statement that is semantically equivalent to
     * "select name from players where score > 50000".
     *
     * Note the ":" at the start of the parameter name ":score".
     * This colon is required.
     *
     * <b>NOTE</b>
     * If \e x is of an integral type that is wider than 64 bits, then any
     * attempt instantiate this function with \e x will result in compilation
     * failure. This is done to rule out any possibility of overflow within
     * SQLite.
     * 
     * @throws InvalidConnection if the database connection is invalid.
     * If this occurs, the state of the SQLStatement will be
     * the same as before the \e bind method was called.
     *
     * @throws SQLiteException or an exception derived therefrom,
     * if SQLite could not properly bind the statement. If this occurs,
     * the statement will be reset and all bindings cleared.
     *
     * @param parameter_name named parameter embedded in
     * the SQL statement.
     *
     * @param x value to be bound to the named parameter.
     *
     * <b>Exception safety</b>: <em>basic guarantee</em>.
     */
    template <typename T>
    void bind(std::string const& parameter_name, T x);
    void bind(std::string const& parameter_name, std::string const& x);

    /**
     * Where an SQLStatement has a result set available,
     * this function (template) can be used to extract the value at
     * the \e indexth column of the current row (where \e index starts
     * counting at 0).
     *
     * Currently the following types for T are supported:\n
     * <b>
     *    int\n
     *    long\n
     *    long long\n
     *    double\n
     *    std::string\n
     * </b>
     *
     * Example usage:\n\n
     * <tt>
     *   std::vector<std::string> names;\n
     *   std::vector<int> lives;\n
     *   SQLStatement s("select name, lives from players where score > :score");\n
     *   s.bind(":score", 500);\n
     *   while (s.step())\n
     *   {\n
     *       names.push_back(s.extract<std::string>(0));\n
     *       lives.push_back(s.extract<int>(1));\n
     *   }\n
     * </tt>
     * 
     * @param index is the column number (starting at 0) from which to
     * read the value.
     * 
     * @throws ResultIndexOutOfRange if \e index is out of range.
     *
     * @throws ValueTypeException if the requested column contains a type that
     * is incompatible with T.
     *
     * <b>Exception safety</b>: <em>strong guarantee</em>.
     */
    template <typename T>
    T extract(int index);

    /**
     * Wraps \b sqlite3_step. Used for stepping through result rows in
     * a "select" statement; can also be used simply to execute an "insert", or
     * "create table" or other statement that generally does not yield
     * result rows.
     *
     * Returns \e true only as long as there are further steps to go (i.e.
     * result rows to examine).
     *
     * On stepping beyond the last result row, step() will return false.
     * The statement will then be automatically reset (see reset()).
     *
     * @throws InvalidConnection if the database connection is invalid. If
     * this occurs, the state of the SQLStatement will be the same as
     * before the \e step method was called.
     *
     * @throws SQLiteException or some exception deriving therefrom, if an
     * error occurs that results in a SQLite error code.
     * This function should almost never occur, but it is
     * possible something will fail as the statement is being executed, in
     * which case the resulting SQLite error condition will trigger the
     * corresponding exception class. If this occurs, the SQLStatement
     * will be reset and all bindings cleared.
     *
     * <b>Exception safety</b>: <em>basic guarantee</em>.
     */
    bool step();

    /**
     * Wraps \b sqlite3_step. Like step() except that it throws an
     * exception if a result row still remains after calling. That is,
     * it is equivalent to calling:\n
     * <tt>if (step()) throw UnexpectedResultRow("...")</tt>;\n
     *
     * @throws UnexpectedResultRow if a result set is returned. If this
     * occurs, the statement is reset (but bindings are not cleared).
     * 
     * @throws InvalidConnection if the database connection is invalid. If
     * this occurs, the statement is left in the same state as it was before
     * the step_final() method was called.
     *
     * @throws SQLiteException or an exception derived therefrom if there
     * is any other error in executing the statement. If this happens, the
     * statement will be reset and all bindings cleared.
     *
     * <b>Exception safety</b>: <em>basic guarantee</em>.
     */
    void step_final();

    /**
     * Resets the statement ready for subsequent
     * re-execution - but does not clear the bound parameters.
     * This is a wrapper for \b sqlite3_reset.
     *
     * <b>Exception safety</b>: <em>nothrow guarantee</em>.
     */
    void reset();

    /**
     * Clears the parameter bindings from the statement, setting all
     * to NULL. This is a wrapper for \b sqlite3_clear_bindings.
     *
     * <b>Exception safety</b>: <em>nothrow guarantee</em>.
     */
    void clear_bindings();

private:

    std::shared_ptr<detail::SQLStatementImpl> m_sql_statement;

};


inline
SQLStatement::SQLStatement
(   DatabaseConnection& p_database_connection,
    std::string const& p_statement_text
):
    m_sql_statement
    (   DatabaseConnection::StatementAttorney::provide_sql_statement
        (   p_database_connection,
            p_statement_text
        )
    )
{
}
        
template <>
inline
void
SQLStatement::bind(std::string const& parameter_name, int x)
{
    m_sql_statement->bind(parameter_name, x);
    return;
}

template <>
inline
void
SQLStatement::bind(std::string const& parameter_name, long x)
{
    m_sql_statement->bind(parameter_name, x);
    return;
}

template <>
inline
void
SQLStatement::bind(std::string const& parameter_name, long long x)
{
    m_sql_statement->bind(parameter_name, x);
    return;
}

template <>
inline
void
SQLStatement::bind(std::string const& parameter_name, double x)
{
    m_sql_statement->bind(parameter_name, x);
    return;
}

inline
void
SQLStatement::bind(std::string const& parameter_name, std::string const& x)
{
    m_sql_statement->bind(parameter_name, x.c_str());
    return;
}

template <>
inline
void
SQLStatement::bind(std::string const& parameter_name, char const* x)
{
    m_sql_statement->bind(parameter_name, x);
    return;
}


}  // namespace sqloxx


#endif  // GUARD_sql_statement_hpp_9859693450787893
