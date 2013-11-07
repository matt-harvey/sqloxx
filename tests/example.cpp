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

#include "database_connection.hpp"
#include "example.hpp"
#include "handle.hpp"
#include "persistent_object.hpp"
#include "sql_statement.hpp"
#include "sqloxx_tests_common.hpp"
#include <memory>
#include <string>

using std::shared_ptr;
using std::string;

namespace sqloxx
{
namespace tests
{

void
ExampleA::setup_tables(DatabaseConnection& dbc)
{
	dbc.execute_sql
	(	"create table example_as"
		"(example_a_id integer primary key autoincrement, "
		"x integer not null, y float not null)"
	);
	return;
}

ExampleA::ExampleA
(	IdentityMap& p_identity_map,
	IdentityMap::Signature const& p_sig
):
	DPersistentObject(p_identity_map),
	m_x(0),
	m_y(0)
{
	(void)p_sig;  // silence compiler re. unused param.
}

ExampleA::ExampleA
(	IdentityMap& p_identity_map,
	Id p_id,
	IdentityMap::Signature const& p_sig
):
	DPersistentObject(p_identity_map, p_id),
	m_x(0),
	m_y(0)
{
	(void)p_sig;  // silence compiler re. unused param.
}

int
ExampleA::x()
{
	load();
	return m_x;
}

int
ExampleA::self_test()
{
	int num_failures = 0;
	ExampleFixture fixture;
	DerivedDatabaseConnection& dbc = *fixture.pdbc;
	Handle<ExampleA> dpo1(dbc);
	dpo1->set_x(3);
	dpo1->set_y(4.08);
	dpo1->save();
	if (dpo1->id() != 1) ++num_failures;
	if (dpo1->x() != 3) ++num_failures;
	if (dpo1->y() != 4.08) ++num_failures;
	Handle<ExampleA> dpo2(dbc, 1);
	if (dpo2->id() != 1) ++num_failures;
	if (dpo2->x() != 3) ++num_failures;
	if (dpo2->y() != 4.08) ++num_failures;

	// Check prospective_key() && do_calculate_prospective_key() (default)
	Handle<ExampleA> dpo5(dbc);
	if (dpo5->prospective_key() != 2) ++num_failures;
	dpo5->set_x(-100);
	dpo5->set_y(982734);
	if (dpo5->prospective_key() != 2) ++num_failures;
	bool ok = false;
	try
	{
		dpo1->prospective_key();
	}
	catch (LogicError&)
	{
		ok = true;
	}
	if (!ok) ++num_failures;	
	
	// Check has_id()
	if (!dpo1->has_id()) ++num_failures;
	Handle<ExampleA> dpo7(dbc);
	if (dpo7->has_id()) ++num_failures;

	return num_failures;
}

double
ExampleA::y()
{
	load();
	return m_y;
}

void
ExampleA::set_x(int p_x)
{
	load();
	m_x = p_x;
	return;
}

void
ExampleA::set_y(double p_y)
{
	load();
	m_y = p_y;
	return;
}

void
ExampleA::do_load()
{
	SQLStatement selector
	(	database_connection(),
		"select x, y from example_as where example_a_id = :p"
	);
	selector.bind(":p", id());
	selector.step();
	int temp_x = selector.extract<int>(0);
	double temp_y = selector.extract<double>(1);
	selector.step_final();
	m_x = temp_x;
	m_y = temp_y;
	return;
}

void
ExampleA::do_save_existing()
{
	SQLStatement updater
	(	database_connection(),
		"update example_as set x = :x, y = :y where example_a_id = :id"
	);
	updater.bind(":x", m_x);
	updater.bind(":y", m_y);
	updater.bind(":id", id());
	updater.step_final();
	return;
}

void
ExampleA::do_save_new()
{
	SQLStatement inserter
	(	database_connection(),
		"insert into example_as(x, y) values(:x, :y)"
	);
	inserter.bind(":x", m_x);
	inserter.bind(":y", m_y);
	inserter.step_final();
	return;
}

string
ExampleA::exclusive_table_name()
{
	return "example_as";
}

string
ExampleA::primary_key_name()
{
	return "example_a_id";
}

void
ExampleB::setup_tables(DatabaseConnection& dbc)
{
	dbc.execute_sql
	(	"create table example_bs"
		"(example_b_id integer primary key autoincrement, "
		"s text not null)"
	);
	return;
}

ExampleB::ExampleB
(	IdentityMap& p_identity_map,
	IdentityMap::Signature const& p_sig
):
	DPersistentObject(p_identity_map)
{
	(void)p_sig;  // silence compiler re. unused param.
}

ExampleB::ExampleB
(	IdentityMap& p_identity_map,
	Id p_id,
	IdentityMap::Signature const& p_sig
):
	DPersistentObject(p_identity_map, p_id),
	m_s("")
{
	(void)p_sig;  // silence compiler re. unused param.
}

ExampleB::~ExampleB()
{
}

string
ExampleB::s()
{
	load();
	return m_s;
}

void
ExampleB::set_s(string const& p_s)
{
	load();
	m_s = p_s;
	return;
}

/*
ExampleB::ExampleB(ExampleB const& rhs):
	DPersistentObject(rhs),
	m_s(rhs.m_s)
{
}
*/

void
ExampleB::load_core()
{
	SQLStatement selector
	(	database_connection(),
		"select s from example_bs where example_b_id = :p"
	);
	selector.bind(":p", id());
	selector.step();
	m_s = selector.extract<string>(0);
	return;
}

void
ExampleB::save_existing_core()
{
	SQLStatement updater
	(	database_connection(),
		"update example_bs set s = :s where example_b_id = :id"
	);
	updater.bind(":s", m_s);
	updater.step_final();
	return;
}

Id
ExampleB::save_new_core()
{
	Id const new_id = next_auto_key
	<	DerivedDatabaseConnection,
		Id
	>	(database_connection(), primary_table_name());
	SQLStatement inserter
	(	database_connection(),
		"insert into example_bs(s) values(:s)"
	);
	inserter.bind(":s", m_s);
	inserter.step_final();
	return new_id;
}

string
ExampleB::exclusive_table_name()
{
	return "example_bs";
}

string
ExampleB::primary_key_name()
{
	return "example_b_id";
}

void
ExampleC::setup_tables(DatabaseConnection& dbc)
{
	dbc.execute_sql
	(	"create table example_cs"
		"(example_b_id references example_bs, "
		"p integer not null, "
		"q integer not null)"
	);
	return;
}

ExampleC::ExampleC
(	IdentityMap& p_identity_map,
	IdentityMap::Signature const& p_sig
):
	ExampleB(p_identity_map, p_sig),
	m_p(0),
	m_q(0)
{
}

ExampleC::ExampleC
(	IdentityMap& p_identity_map,
	Id p_id,
	IdentityMap::Signature const& p_sig
):
	ExampleB(p_identity_map, p_id, p_sig),
	m_p(0),
	m_q(0)
{
}

int
ExampleC::p()
{
	load();
	return m_p;
}

int
ExampleC::q()
{
	load();
	return m_q;
}

void
ExampleC::set_p(int p_p)
{
	load();
	m_p = p_p;
	return;
}

void
ExampleC::set_q(int p_q)
{
	load();
	m_q = p_q;
	return;
}

string
ExampleC::exclusive_table_name()
{
	return "example_cs";
}

void
ExampleC::do_load()
{
	load_core();
	SQLStatement selector
	(	database_connection(),
		"select p, q from example_cs where example_b_id = :p"
	);
	selector.bind(":p", id());
	selector.step();
	m_p = selector.extract<int>(0);
	m_q = selector.extract<int>(1);
	return;
}

void
ExampleC::do_save_existing()
{
	save_existing_core();
	SQLStatement statement
	(	database_connection(),
		"update example_cs set p = :p, q = :q where example_b_id = :id"
	);
	statement.bind(":p", m_p);
	statement.bind(":q", m_q);
	statement.bind(":example_b_id", id());
	statement.step_final();
	return;
}

void
ExampleC::do_save_new()
{
	Id const new_id = save_new_core();	
	SQLStatement statement
	(	database_connection(),
		"insert into example_cs (example_b_id, p, q) "
		"values(:example_b_id, :p, :q)"
	);
	statement.bind(":example_b_id", new_id);
	statement.bind(":p", m_p);
	statement.bind(":q", m_q);
	statement.step_final();
	return;
}

void
ExampleC::do_remove()
{
	SQLStatement statement
	(	database_connection(),
		"delete from example_cs where example_b_id = :id"
	);
	statement.bind(":id", id());
	statement.step_final();
	ExampleB::do_remove();
	return;
}

DerivedDatabaseConnection::DerivedDatabaseConnection():
	DatabaseConnection(),
	m_example_a_map(*this),
	m_example_b_map(*this)
{
}


}  // namespace tests
}  // namespace sqloxx
