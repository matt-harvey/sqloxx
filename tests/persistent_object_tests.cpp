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
#include "example.hpp"
#include "handle.hpp"
#include "sqloxx_tests_common.hpp"
#include "sql_statement.hpp"
#include "sqloxx_exceptions.hpp"
#include <boost/filesystem.hpp>
#include <jewel/exception.hpp>
#include <jewel/optional.hpp>
#include <jewel/assert.hpp>
#include <iostream>
#include <limits>
#include <memory>
#include <typeinfo>
#include <UnitTest++/UnitTest++.h>
#include <stdexcept>

using jewel::UninitializedOptionalException;
using std::cerr;
using std::endl;
using std::numeric_limits;
using std::shared_ptr;



namespace sqloxx
{
namespace tests
{

namespace filesystem = boost::filesystem;

TEST_FIXTURE(ExampleAFixture, test_example_a_constructor_one_param)
{
	Handle<ExampleA> dpo(*pdbc);
	CHECK_THROW(dpo->id(), UninitializedOptionalException);
	CHECK_EQUAL(dpo->x(), 0);
	dpo->set_y(3.3);
	CHECK_EQUAL(dpo->y(), 3.3);
}


TEST_FIXTURE(ExampleAFixture, test_example_a_constructor_two_params)
{
	Handle<ExampleA> dpo(*pdbc);
	dpo->set_x(10);
	dpo->set_y(3.23);
	dpo->save();
	CHECK_EQUAL(dpo->id(), 1);
	CHECK_EQUAL(dpo->x(), 10);
	CHECK_EQUAL(dpo->y(), 3.23);
	Handle<ExampleA> e(*pdbc, 1);
	CHECK_EQUAL(e->id(), dpo->id());
	CHECK_EQUAL(e->id(), 1);
	CHECK_EQUAL(e->x(), 10);
	CHECK_EQUAL(e->y(), 3.23);
}

TEST_FIXTURE(ExampleAFixture, test_example_a_save_1)
{
	Handle<ExampleA> dpo1(*pdbc);
	dpo1->set_x(78);
	dpo1->set_y(4.5);
	dpo1->save();
	Handle<ExampleA> dpo2(*pdbc);
	dpo2->set_x(234);
	dpo2->set_y(29837.01);
	dpo2->save();
	CHECK_EQUAL(dpo2->id(), 2);
	Handle<ExampleA> dpo2b(*pdbc, 2);
	CHECK_EQUAL(dpo2b->x(), 234);
	CHECK_EQUAL(dpo2b->y(), 29837.01);
	dpo2b->set_y(2.0);
	dpo2b->save();
	Handle<ExampleA> dpo2c(*pdbc, 2);
	CHECK_EQUAL(dpo2c->id(), 2);
	CHECK_EQUAL(dpo2c->x(), 234);
	CHECK_EQUAL(dpo2c->y(), 2.0);
	dpo2c->set_x(-10);  // But don't call save yet
	Handle<ExampleA> dpo2d(*pdbc, 2);
	CHECK_EQUAL(dpo2d->x(), -10); // Reflected before save, due to IdentityMap
	CHECK_EQUAL(dpo2d->y(), 2.0);
	dpo2c->save();  // Now the changed object is saved
	Handle<ExampleA> dpo2e(*pdbc, 2);
	CHECK_EQUAL(dpo2e->x(), -10);  // Still reflected after save.
	CHECK_EQUAL(dpo2e->y(), 2.0);
	Handle<ExampleA> dpo1b(*pdbc, 1);
	dpo1b->save();
	CHECK_EQUAL(dpo1b->x(), 78);
	CHECK_EQUAL(dpo1b->y(), 4.5);
	dpo1b->set_x(1000);
	// All handles to this object reflect the change
	CHECK_EQUAL(dpo1->x(), 1000);
}

TEST_FIXTURE(ExampleAFixture, test_example_a_save_2)
{
	Handle<ExampleA> dpo1(*pdbc);
	dpo1->set_x(978);
	dpo1->set_y(-.238);
	dpo1->save();

	Handle<ExampleA> dpo2(*pdbc);
	dpo2->set_x(20);
	dpo2->set_y(0.00030009);
	dpo2->save();
	CHECK_EQUAL(dpo1->id(), 1);
	CHECK_EQUAL(dpo2->id(), 2);

	// Test TransactionNestingException
	SQLStatement troublesome_statement
	(	*pdbc,
		"insert into example_as(example_a_id, x, y) values"
		"(:i, :x, :y)"
	);
	troublesome_statement.bind(":i", numeric_limits<int>::max());
	troublesome_statement.bind(":x", 30);
	troublesome_statement.bind(":y", 39.091);
	troublesome_statement.step_final();
	SQLStatement check_troublesome
	(	*pdbc,
		"select example_a_id from example_as where x = 30"
	);
	check_troublesome.step();
	CHECK_EQUAL
	(	check_troublesome.extract<int>(0),
		numeric_limits<int>::max()
	);
	check_troublesome.step_final();

	Handle<ExampleA> dpo3(*pdbc);
	dpo3->set_x(100);
	dpo3->set_y(3.2);

	CHECK_THROW(dpo3->save(), TableSizeException);
	
	// But these are still OK
	dpo1->save();
	dpo1->save();
	dpo1->save();
}


TEST_FIXTURE(ExampleAFixture, test_example_a_save_and_transactions)
{
	// Test interaction of save() with DatabaseTransaction

	Handle<ExampleA> dpo1(*pdbc);
	dpo1->set_x(4000);
	dpo1->set_y(0.13);
	dpo1->save();

	DatabaseTransaction transaction1(*pdbc);


	Handle<ExampleA> dpo2(*pdbc);
	dpo2->set_x(-17);
	dpo2->set_y(64.29382);
	dpo2->save();
	
	Handle<ExampleA> dpo2b(*pdbc, 2);
	CHECK_EQUAL(dpo2b->x(), -17);
	CHECK_EQUAL(dpo2b->y(), 64.29382);
	dpo2b->save();

	CHECK_EQUAL(dpo1->id(), 1);
	CHECK_EQUAL(dpo2->id(), 2);
	CHECK_EQUAL(dpo2b->id(), 2);

	Handle<ExampleA> dpo3(*pdbc);
	dpo3->set_x(7834);
	dpo3->set_y(521.520);
	CHECK(!dpo3->has_id());
	dpo3->save();
	CHECK_EQUAL(dpo3->id(), 3);

	Handle<ExampleA> dpo4(*pdbc);
	dpo4->set_y(1324.6);
	dpo4->set_x(321);
	dpo4->save();
	CHECK_EQUAL(dpo4->id(), 4);

	transaction1.cancel();

	SQLStatement statement(*pdbc, "select * from example_as");
	int rows = 0;
	while (statement.step()) ++rows;
	CHECK_EQUAL(rows, 1);

	// The cache is not aware in itself that the save was cancelled...
	/* THIS WOULD CAUSE UNDEFINED BEHAVIOUR
	Handle<ExampleA> dpo2c
	(	Handle<ExampleA>::create_unchecked(*pdbc, 2)
	);
	CHECK_EQUAL(dpo2c->id(), 2);
	CHECK_EQUAL(dpo2c->x(), -17);
	CHECK_EQUAL(dpo2c->y(), 64.29382);
	*/

	// ... That's why we should not use unchecked_get_handle unless
	// we're sure we've got a valid id. The "normal" get_handle
	// throws here.
	bool ok = false;
	try
	{
		Handle<ExampleA> dpo2c_checked(*pdbc, 2);
	}
	catch (BadIdentifier&)
	{
		ok = true;
	}
	CHECK(ok);

	// This will save over the top of the old
	// one...
	Handle<ExampleA> dpo5(*pdbc);
	dpo5->set_x(12);
	dpo5->set_y(19);
	dpo5->save();

	CHECK_EQUAL(dpo5->id(), 2);
	CHECK_EQUAL(dpo5->x(), 12);
	CHECK_EQUAL(dpo5->y(), 19);
	
	// The old objects still exist in memory
	CHECK_EQUAL(dpo2b->x(), -17);
	CHECK_EQUAL(dpo2->y(), 64.29382);

	Handle<ExampleA> dpo2d(*pdbc, 2);
	CHECK_EQUAL(dpo2d->x(), 12);
	CHECK_EQUAL(dpo2d->y(), 19);

	bool ok2 = false;
	try
	{
		Handle<ExampleA> dpo7(*pdbc, 7);
	}
	catch (BadIdentifier&)
	{
		ok2 = true;
	}
	CHECK(ok2);
	
	bool ok3 = false;
	try
	{
		Handle<ExampleA> dpo4b(*pdbc, 4);
	}
	catch (BadIdentifier&)
	{
		ok3 = true;
	}
	CHECK(ok3);

	
	/* If dpo4 still has a "residual id", then this results in
	 * undefined behaviour.
	// dpo4->remove();
	*/
	// But it still exists in memory with its attributes. That's OK.
	CHECK_EQUAL(dpo4->x(), 321);
	CHECK_EQUAL(dpo4->y(), 1324.6);
}

TEST_FIXTURE(ExampleAFixture, test_example_a_exists_and_remove)
{
	Handle<ExampleA> dpo1(*pdbc);
	dpo1->set_x(7);
	dpo1->set_y(5.8);
	dpo1->save();
	SQLStatement selector
	(	*pdbc,
		"select example_a_id from example_as"
	);
	bool check = selector.step();
	CHECK(check);
	bool exists =
		PersistentObject<ExampleA, DerivedDatabaseConnection>::exists
		(	*pdbc,
			1
		);
	CHECK(exists);
	Handle<ExampleA> dpo1b = dpo1;
	dpo1->remove();
	selector.reset();
	check = selector.step();
	CHECK(!check);
	exists =
		PersistentObject<ExampleA, DerivedDatabaseConnection>::exists
		(	*pdbc,
			1
		);
	CHECK(!exists);
	CHECK_THROW(dpo1b->id(), UninitializedOptionalException);
	CHECK_THROW(dpo1->id(), UninitializedOptionalException);

	// Now let's mix with DatabaseTransaction
	Handle<ExampleA> dpo2(*pdbc);
	dpo2->set_x(10);
	dpo2->set_y(50.78);
	dpo2->save();
	selector.reset();
	check = selector.step();
	CHECK(check);
	DatabaseTransaction transaction(*pdbc);
	CHECK_EQUAL(dpo2->id(), 2);
	dpo2->remove();
	bool ok = false;
	try
	{
		Handle<ExampleA> dpo2b(*pdbc, 2);
	}
	catch (BadIdentifier&)
	{
		ok = true;
	}
	CHECK(ok);
	selector.reset();
	check = selector.step();
	CHECK(!check);
	transaction.cancel();  // This should cause the object to go back in
	selector.reset(); 
	check = selector.step();
	CHECK(check);
	Handle<ExampleA> dpo2c(*pdbc, 2);
	CHECK_EQUAL(dpo2c->y(), 50.78);
	CHECK_EQUAL(dpo2c->x(), 10);
}

TEST_FIXTURE(ExampleAFixture, test_example_a_none_saved)
{
	typedef PersistentObject<ExampleA, DerivedDatabaseConnection> DPO;
	bool none_saved = DPO::none_saved(*pdbc);
	CHECK(none_saved);
	Handle<ExampleA> dpo1(*pdbc);
	none_saved = DPO::none_saved(*pdbc);

	DatabaseTransaction transaction_a(*pdbc);
	CHECK(none_saved);
	dpo1->set_x(10);
	dpo1->set_y(3);
	dpo1->save();
	none_saved = DPO::none_saved(*pdbc);
	CHECK(!none_saved);
	Handle<ExampleA> dpo2(*pdbc);
	dpo2->set_x(12);
	dpo2->set_y(3.5);
	dpo2->save();
	none_saved = DPO::none_saved(*pdbc);
	CHECK(!none_saved);
	transaction_a.cancel();

	none_saved = DPO::none_saved(*pdbc);
	CHECK(none_saved);
	Handle<ExampleA> dpo3(*pdbc);
	dpo3->set_x(109383);
	dpo3->set_y(-29834.6);
	dpo3->save();
	none_saved = DPO::none_saved(*pdbc);
	CHECK(!none_saved);

	DatabaseTransaction transaction_b(*pdbc);
	none_saved = DPO::none_saved(*pdbc);
	CHECK(!none_saved);
	dpo3->remove();
	none_saved = DPO::none_saved(*pdbc);
	CHECK(none_saved);
	transaction_b.cancel();

	none_saved = DPO::none_saved(*pdbc);
	CHECK(!none_saved);
	Handle<ExampleA> dpo3b(*pdbc, 1);
	dpo3b->remove();
	none_saved = DPO::none_saved(*pdbc);
	CHECK(none_saved);
}

TEST_FIXTURE(ExampleAFixture, test_example_a_id_getter)
{
	Handle<ExampleA> dpo1(*pdbc);
	CHECK_THROW(dpo1->id(), UninitializedOptionalException);
	dpo1->save();
	CHECK_EQUAL(dpo1->id(), 1);
	Handle<ExampleA> dpo2(*pdbc);
	CHECK_THROW(dpo2->id(), UninitializedOptionalException);
	CHECK_THROW(dpo2->id(), UninitializedOptionalException);
	dpo2->save();
	CHECK_EQUAL(dpo2->id(), 2);
	dpo2->save();
	CHECK_EQUAL(dpo2->id(), 2);
	dpo2->remove();
	CHECK_THROW(dpo2->id(), UninitializedOptionalException);
}

TEST_FIXTURE(ExampleAFixture, test_load_indirectly)
{
	// load is protected method but we here we test it indirectly
	// via getting functions we know call it
	Handle<ExampleA> dpo1(*pdbc);
	int const a = 2097601234;
	double const b = 72973.2987300;
	dpo1->set_x(a);
	dpo1->set_y(b);
	JEWEL_ASSERT (dpo1->x() == a);
	JEWEL_ASSERT (dpo1->y() == b);
	dpo1->save();

	Handle<ExampleA> dpo2(*pdbc, 1);
	CHECK_EQUAL(dpo2->id(), 1);
	CHECK_EQUAL(dpo2->x(), a);  // load called here
	CHECK_EQUAL(dpo2->y(), b);  // and here
}

TEST_FIXTURE(ExampleAFixture, test_ghostify)
{
	Handle<ExampleA> dpo1(*pdbc);
	dpo1->set_x(1290387);
	dpo1->set_y(127);
	dpo1->save();
	Handle<ExampleA> dpo2(*pdbc);
	dpo2->set_x(273);
	dpo2->set_y(-19.986);
	dpo2->save();
	CHECK_EQUAL(dpo1->id(), 1);
	CHECK_EQUAL(dpo1->x(), 1290387);
	CHECK_EQUAL(dpo2->y(), -19.986);
	CHECK_EQUAL(dpo2->id(), 2);
	
	// We ghostify - which should have no visible effect
	dpo2->ghostify();
	CHECK_EQUAL(dpo2->x(), 273);
	CHECK_EQUAL(dpo2->y(), -19.986);
	CHECK_EQUAL(dpo2->id(), 2);
}

TEST(test_example_a_self_test)
{
	// Tests certain protected functions of PersistentObject
	CHECK_EQUAL(ExampleA::self_test(), 0);
}

}  // namespace tests
}  // namespace sqloxx

