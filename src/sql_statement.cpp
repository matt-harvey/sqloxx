/*
 * Copyright 2012-2013 Matthew Harvey
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

#include "sql_statement.hpp"
#include "database_connection.hpp"
#include "detail/sql_statement_impl.hpp"
#include <jewel/log.hpp>
#include <memory>
#include <string>

using std::shared_ptr;
using std::endl;
using std::string;


namespace sqloxx
{



SQLStatement::~SQLStatement()
{
	m_sql_statement->reset();
	m_sql_statement->clear_bindings();
	m_sql_statement->unlock();
}



template <>
int
SQLStatement::extract<int>(int index)
{
	return m_sql_statement->extract<int>(index);
}


template <>
long
SQLStatement::extract<long>(int index)
{
	return m_sql_statement->extract<long>(index);
}

template <>
long long
SQLStatement::extract<long long>(int index)
{
	return m_sql_statement->extract<long long>(index);
}


// All these total specialisations of SQLStatement::extract
// are to avoid having to put the call to SQLStatementImpl::extract in the
// header - which would introduce unwanted compilation dependencies.

template <>
double
SQLStatement::extract<double>(int index)
{
	return m_sql_statement->extract<double>(index);
}


template <>
string
SQLStatement::extract<string>(int index)
{
	return m_sql_statement->extract<string>(index);
}

bool
SQLStatement::step()
{
	return m_sql_statement->step();
}


void
SQLStatement::step_final()
{
	m_sql_statement->step_final();
	return;
}


void
SQLStatement::reset()
{
	m_sql_statement->reset();
	return;
}


void
SQLStatement::clear_bindings()
{
	m_sql_statement->clear_bindings();
	return;
}



}  // namespace sqloxx
