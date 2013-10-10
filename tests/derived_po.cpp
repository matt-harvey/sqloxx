/*
 * This file is part of the Sqloxx project and is distributed under the
 * terms of the license contained in the file LICENSE.txt distributed
 * with this package.
 * 
 * Author: Matthew Harvey <matthew@matthewharvey.net>
 *
 * Copyright (c) 2012-2013, Matthew Harvey.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "database_connection.hpp"
#include "derived_po.hpp"
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
DerivedPO::setup_tables(DatabaseConnection& dbc)
{
	dbc.execute_sql
	(	"create table derived_pos"
		"(derived_po_id integer primary key autoincrement, "
		"x integer not null, y float not null)"
	);
	return;
}

DerivedPO::DerivedPO
(	IdentityMap& p_identity_map,
	Id p_id,
	IdentityMap::Signature const& p_sig
):
	DPersistentObject
	(	p_identity_map,
		p_id
	),
	m_x(0),
	m_y(0)
{
	(void)p_sig;  // silence compiler re. unused param.
}

DerivedPO::DerivedPO
(	IdentityMap& p_identity_map,
	IdentityMap::Signature const& p_sig
):
	DPersistentObject(p_identity_map),
	m_x(0),
	m_y(0)
{
	(void)p_sig;  // silence compiler re. unused param.
}

int
DerivedPO::x()
{
	load();
	return m_x;
}

int
DerivedPO::self_test()
{
	int num_failures = 0;
	DerivedPOFixture fixture;
	DerivedDatabaseConnection& dbc = *fixture.pdbc;
	Handle<DerivedPO> dpo1(dbc);
	dpo1->set_x(3);
	dpo1->set_y(4.08);
	dpo1->save();
	if (dpo1->id() != 1) ++num_failures;
	if (dpo1->x() != 3) ++num_failures;
	if (dpo1->y() != 4.08) ++num_failures;
	Handle<DerivedPO> dpo2(dbc, 1);
	if (dpo2->id() != 1) ++num_failures;
	if (dpo2->x() != 3) ++num_failures;
	if (dpo2->y() != 4.08) ++num_failures;

	// Check prospective_key() && do_calculate_prospective_key() (default)
	Handle<DerivedPO> dpo5(dbc);
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
	Handle<DerivedPO> dpo7(dbc);
	if (dpo7->has_id()) ++num_failures;

	return num_failures;
}

double
DerivedPO::y()
{
	load();
	return m_y;
}

void
DerivedPO::set_x(int p_x)
{
	load();
	m_x = p_x;
	return;
}

void
DerivedPO::set_y(double p_y)
{
	load();
	m_y = p_y;
	return;
}

DerivedPO::DerivedPO(DerivedPO const& rhs):
	DPersistentObject(rhs),
	m_x(rhs.m_x),
	m_y(rhs.m_y)
{
}

void
DerivedPO::do_load()
{
	SQLStatement selector
	(	database_connection(),
		"select x, y from derived_pos where derived_po_id = :p"
	);
	selector.bind(":p", id());
	selector.step();
	int temp_x = selector.extract<int>(0);
	double temp_y = selector.extract<double>(1);
	selector.step_final();
	m_x = temp_x;
	m_y = temp_y;
}

void
DerivedPO::do_save_existing()
{
	SQLStatement updater
	(	database_connection(),
		"update derived_pos set x = :x, y = :y where derived_po_id = :id"
	);
	updater.bind(":x", m_x);
	updater.bind(":y", m_y);
	updater.bind(":id", id());
	updater.step_final();
	return;
}

void
DerivedPO::do_save_new()
{
	SQLStatement inserter
	(	database_connection(),
		"insert into derived_pos(x, y) values(:x, :y)"
	);
	inserter.bind(":x", m_x);
	inserter.bind(":y", m_y);
	inserter.step_final();
}

void
DerivedPO::do_ghostify()
{
	// No point doing anything here really, but in any
	// case let's set m_x and m_y to values that look
	// suspicious, to suggest ghostness.
	m_x = -999999999;
	m_y = -9.99999999;
}

string
DerivedPO::exclusive_table_name()
{
	return "derived_pos";
}

string
DerivedPO::primary_key_name()
{
	return "derived_po_id";
}

DerivedDatabaseConnection::DerivedDatabaseConnection():
	DatabaseConnection(),
	m_derived_po_map(*this)
{
}

}  // namespace tests
}  // namespace sqloxx
