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

#include "database_connection.hpp"
#include "database_transaction.hpp"
#include "detail/sqlite_dbconn.hpp"
#include "sql_statement.hpp"
#include "sqloxx_exceptions.hpp"
#include "detail/sql_statement_impl.hpp"
#include <boost/filesystem.hpp>
#include <jewel/assert.hpp>
#include <jewel/exception.hpp>
#include <jewel/log.hpp>
#include <jewel/optional.hpp>
#include <iostream>
#include <climits>
#include <cstdio>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>

using jewel::value;
using std::bad_alloc;
using std::cout;
using std::clog;
using std::endl;
using std::fprintf;
using std::numeric_limits;
using std::set;
using std::shared_ptr;
using std::string;
using std::unordered_map;

namespace sqloxx
{

// Switch statement later relies on this being INT_MAX, and
// won't compile if it's changed to std::numeric_limits<int>::max().
int const
DatabaseConnection::s_max_nesting = INT_MAX;

DatabaseConnection::DatabaseConnection
(	StatementCache::size_type p_cache_capacity
):
	m_sqlite_dbconn(new detail::SQLiteDBConn),
	m_transaction_nesting_level(0),
	m_cache_capacity(p_cache_capacity)
{
}

DatabaseConnection::~DatabaseConnection()
{
	if (m_transaction_nesting_level > 0)
	{
		// We avoid streams here, because they might throw
		// (in theory, if exceptions have been enabled for the
		// stream).
		fprintf
		(	stderr,
			"Transaction(s) remained incomplete on closure of "
			"DatabaseConnection.\n"
		);
	}
	m_statement_cache.clear();
}

bool
DatabaseConnection::is_valid() const
{
	return m_sqlite_dbconn->is_valid();
}

void
DatabaseConnection::open(boost::filesystem::path const& p_filepath)
{
	m_sqlite_dbconn->open(p_filepath);
	m_filepath = boost::filesystem::absolute(p_filepath);
	do_setup();
	return;
}

void
DatabaseConnection::execute_sql(string const& str)
{
	m_sqlite_dbconn->execute_sql(str);
	return;
}

void
DatabaseConnection::setup_boolean_table()
{
	execute_sql("create table booleans(representation integer primary key)");
	execute_sql("insert into booleans(representation) values(0)");
	execute_sql("insert into booleans(representation) values(1)");
	return;
}

void
DatabaseConnection::do_setup()
{
	// Empty body - this is deliberate - see API documentation as to why.
	return;
}

int
DatabaseConnection::max_nesting()
{
	return s_max_nesting;
}

boost::filesystem::path
DatabaseConnection::filepath() const
{
	if (!is_valid())
	{
		JEWEL_THROW
		(	InvalidConnection,
			"Cannot return filepath of invalid DatabaseConnection."
		);
	}
	JEWEL_ASSERT (m_filepath);

	// Filepath is absolute
	JEWEL_ASSERT
	(	boost::filesystem::absolute(value(m_filepath)) ==
		value(m_filepath)
	);

	return value(m_filepath);
}

void
DatabaseConnection::begin_transaction()
{
	switch (m_transaction_nesting_level)
	{
	case 0:
		unchecked_begin_transaction();
		break;
	case s_max_nesting:
		JEWEL_THROW
		(	TransactionNestingException,
			"Maximum nesting level reached."
		);
		JEWEL_HARD_ASSERT (false);  // Execution never reaches here
	default:
		JEWEL_ASSERT (m_transaction_nesting_level > 0);
		unchecked_set_savepoint();
		break;
	}
	++m_transaction_nesting_level;
	return;
}

void
DatabaseConnection::end_transaction()
{
	switch (m_transaction_nesting_level)
	{
	case 1:
		unchecked_end_transaction();
		break;
	case 0:
		JEWEL_THROW
		(	TransactionNestingException,
			"Cannot end SQL transaction when there in none open."
		);
		JEWEL_HARD_ASSERT (false);  // Execution never reaches here
	default:
		JEWEL_ASSERT (m_transaction_nesting_level > 1);
		unchecked_release_savepoint();
		break;
	}
	JEWEL_ASSERT (m_transaction_nesting_level > 0);
	--m_transaction_nesting_level;
	return;
}

void
DatabaseConnection::cancel_transaction()
{
	switch (m_transaction_nesting_level)
	{
	case 1:
		unchecked_rollback_transaction();
		break;
	case 0:
		JEWEL_THROW
		(	TransactionNestingException,
			"Cannot cancel SQL transaction when there is none open."
		);
		JEWEL_HARD_ASSERT (false);  // Execution never reaches here
	default:
		JEWEL_ASSERT (m_transaction_nesting_level > 1);
		unchecked_rollback_to_savepoint();
		unchecked_release_savepoint();
		break;
	}
	--m_transaction_nesting_level;
	return;
}

shared_ptr<detail::SQLStatementImpl>
DatabaseConnection::provide_sql_statement(string const& statement_text)
{
	if (!is_valid())
	{
		JEWEL_THROW(InvalidConnection, "Invalid database connection.");
	}
	StatementCache::const_iterator const it
	(	m_statement_cache.find(statement_text)
	);
	if (it != m_statement_cache.end())
	{
		shared_ptr<detail::SQLStatementImpl> existing_statement(it->second);
		if (!(existing_statement->is_locked()))
		{
			existing_statement->lock();
			return existing_statement;
		}
	}
	JEWEL_ASSERT (it == m_statement_cache.end() || it->second->is_locked());
	shared_ptr<detail::SQLStatementImpl> new_statement
	(	new detail::SQLStatementImpl(*m_sqlite_dbconn, statement_text)
	);
	new_statement->lock();
	if (m_statement_cache.size() != m_cache_capacity)
	{
		JEWEL_ASSERT (m_statement_cache.size() < m_cache_capacity);
		try
		{
			m_statement_cache[statement_text] = new_statement;
		}
		catch (bad_alloc&)
		{
			m_statement_cache.clear();
			JEWEL_ASSERT (new_statement != 0);
		}
	}
	/*
	else
	{
		// Cache has reached capacity and caching has been
		// discontinued.
	}
	*/
	return new_statement;
}

void
DatabaseConnection::unchecked_begin_transaction()
{
	SQLStatement statement(*this, "begin");
	statement.step();
	return;
}

void
DatabaseConnection::unchecked_end_transaction()
{
	SQLStatement statement(*this, "end");
	statement.step();
	return;
}

void
DatabaseConnection::unchecked_set_savepoint()
{
	SQLStatement statement(*this, "savepoint sp");
	statement.step();
	return;
}

void
DatabaseConnection::unchecked_release_savepoint()
{
	SQLStatement statement(*this, "release sp");
	statement.step();
	return;
}

void
DatabaseConnection::unchecked_rollback_transaction()
{
	SQLStatement statement(*this, "rollback");
	statement.step();
	return;
}

void
DatabaseConnection::unchecked_rollback_to_savepoint()
{
	SQLStatement statement(*this, "rollback to savepoint sp");
	statement.step();
	return;
}

int
DatabaseConnection::self_test()
{
	int ret = 0;
	JEWEL_ASSERT (m_transaction_nesting_level == 0);
	int const original_nesting = m_transaction_nesting_level;
	m_transaction_nesting_level = max_nesting() - 1;
	DatabaseTransaction transaction1(*this);  // Should be ok.
	++ret;
	try
	{
		DatabaseTransaction transaction2(*this);  // Should throw
	}
	catch (TransactionNestingException&)
	{
		--ret;
	}
	transaction1.cancel();
	m_transaction_nesting_level = original_nesting;
	return ret;

}

}  // namespace sqloxx
