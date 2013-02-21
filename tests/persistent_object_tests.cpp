#include "derived_po.hpp"
#include "sqloxx_tests_common.hpp"
#include "../database_connection.hpp"
#include "../handle.hpp"
#include "../sql_statement.hpp"
#include "../sqloxx_exceptions.hpp"
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <jewel/exception.hpp>
#include <jewel/optional.hpp>
#include <cassert>
#include <iostream>
#include <limits>
#include <typeinfo>
#include <UnitTest++/UnitTest++.h>
#include <stdexcept>

using boost::shared_ptr;
using jewel::UninitializedOptionalException;
using std::cerr;
using std::endl;
using std::numeric_limits;

#define LOG_POSITION std::cout << __FILE__ << ", line " << __LINE__ << std::endl


namespace sqloxx
{
namespace tests
{

namespace filesystem = boost::filesystem;

TEST_FIXTURE(DerivedPOFixture, test_derived_po_constructor_one_param)
{
	Handle<DerivedPO> dpo(*pdbc);
	CHECK_THROW(dpo->id(), UninitializedOptionalException);
	CHECK_EQUAL(dpo->x(), 0);
	dpo->set_y(3.3);
	CHECK_EQUAL(dpo->y(), 3.3);
}


TEST_FIXTURE(DerivedPOFixture, test_derived_po_constructor_two_params)
{
	Handle<DerivedPO> dpo(*pdbc);
	dpo->set_x(10);
	dpo->set_y(3.23);
	dpo->save();
	CHECK_EQUAL(dpo->id(), 1);
	CHECK_EQUAL(dpo->x(), 10);
	CHECK_EQUAL(dpo->y(), 3.23);
	Handle<DerivedPO> e(*pdbc, 1);
	CHECK_EQUAL(e->id(), dpo->id());
	CHECK_EQUAL(e->id(), 1);
	CHECK_EQUAL(e->x(), 10);
	CHECK_EQUAL(e->y(), 3.23);
}

TEST_FIXTURE(DerivedPOFixture, test_derived_po_save_1)
{
	Handle<DerivedPO> dpo1(*pdbc);
	dpo1->set_x(78);
	dpo1->set_y(4.5);
	dpo1->save();
	Handle<DerivedPO> dpo2(*pdbc);
	dpo2->set_x(234);
	dpo2->set_y(29837.01);
	dpo2->save();
	CHECK_EQUAL(dpo2->id(), 2);
	Handle<DerivedPO> dpo2b(*pdbc, 2);
	CHECK_EQUAL(dpo2b->x(), 234);
	CHECK_EQUAL(dpo2b->y(), 29837.01);
	dpo2b->set_y(2.0);
	dpo2b->save();
	Handle<DerivedPO> dpo2c(*pdbc, 2);
	CHECK_EQUAL(dpo2c->id(), 2);
	CHECK_EQUAL(dpo2c->x(), 234);
	CHECK_EQUAL(dpo2c->y(), 2.0);
	dpo2c->set_x(-10);  // But don't call save yet
	Handle<DerivedPO> dpo2d(*pdbc, 2);
	CHECK_EQUAL(dpo2d->x(), -10); // Reflected before save, due to IdentityMap
	CHECK_EQUAL(dpo2d->y(), 2.0);
	dpo2c->save();  // Now the changed object is saved
	Handle<DerivedPO> dpo2e(*pdbc, 2);
	CHECK_EQUAL(dpo2e->x(), -10);  // Still reflected after save.
	CHECK_EQUAL(dpo2e->y(), 2.0);
	Handle<DerivedPO> dpo1b(*pdbc, 1);
	dpo1b->save();
	CHECK_EQUAL(dpo1b->x(), 78);
	CHECK_EQUAL(dpo1b->y(), 4.5);
	dpo1b->set_x(1000);
	// All handles to this object reflect the change
	CHECK_EQUAL(dpo1->x(), 1000);
}

TEST_FIXTURE(DerivedPOFixture, test_derived_po_save_2)
{
	Handle<DerivedPO> dpo1(*pdbc);
	dpo1->set_x(978);
	dpo1->set_y(-.238);
	dpo1->save();

	Handle<DerivedPO> dpo2(*pdbc);
	dpo2->set_x(20);
	dpo2->set_y(0.00030009);
	dpo2->save();
	CHECK_EQUAL(dpo1->id(), 1);
	CHECK_EQUAL(dpo2->id(), 2);

	// Test TransactionNestingException
	SQLStatement troublesome_statement
	(	*pdbc,
		"insert into derived_pos(derived_po_id, x, y) values"
		"(:i, :x, :y)"
	);
	troublesome_statement.bind(":i", numeric_limits<int>::max());
	troublesome_statement.bind(":x", 30);
	troublesome_statement.bind(":y", 39.091);
	troublesome_statement.step_final();
	SQLStatement check_troublesome
	(	*pdbc,
		"select derived_po_id from derived_pos where x = 30"
	);
	check_troublesome.step();
	CHECK_EQUAL
	(	check_troublesome.extract<int>(0),
		numeric_limits<int>::max()
	);
	check_troublesome.step_final();

	Handle<DerivedPO> dpo3(*pdbc);
	dpo3->set_x(100);
	dpo3->set_y(3.2);

	CHECK_THROW(dpo3->save(), TableSizeException);
	
	// But these are still OK
	dpo1->save();
	dpo1->save();
	dpo1->save();
}


TEST_FIXTURE(DerivedPOFixture, test_derived_po_save_and_transactions)
{
	// Test interaction of save() with DatabaseTransaction

	LOG_POSITION;
	Handle<DerivedPO> dpo1(*pdbc);
	dpo1->set_x(4000);
	dpo1->set_y(0.13);
	dpo1->save();

	LOG_POSITION;
	DatabaseTransaction transaction1(*pdbc);


	Handle<DerivedPO> dpo2(*pdbc);
	dpo2->set_x(-17);
	dpo2->set_y(64.29382);
	dpo2->save();
	LOG_POSITION;
	
	Handle<DerivedPO> dpo2b(*pdbc, 2);
	CHECK_EQUAL(dpo2b->x(), -17);
	CHECK_EQUAL(dpo2b->y(), 64.29382);
	dpo2b->save();
	LOG_POSITION;

	CHECK_EQUAL(dpo1->id(), 1);
	CHECK_EQUAL(dpo2->id(), 2);
	CHECK_EQUAL(dpo2b->id(), 2);
	LOG_POSITION;

	Handle<DerivedPO> dpo3(*pdbc);
	LOG_POSITION;
	dpo3->set_x(7834);
	dpo3->set_y(521.520);
	CHECK(!dpo3->has_id());
	dpo3->save();
	CHECK_EQUAL(dpo3->id(), 3);
	LOG_POSITION;

	Handle<DerivedPO> dpo4(*pdbc);
	dpo4->set_y(1324.6);
	dpo4->set_x(321);
	dpo4->save();
	CHECK_EQUAL(dpo4->id(), 4);
	LOG_POSITION;

	transaction1.cancel();
	LOG_POSITION;

	SQLStatement statement(*pdbc, "select * from derived_pos");
	LOG_POSITION;
	int rows = 0;
	while (statement.step()) ++rows;
	CHECK_EQUAL(rows, 1);
	LOG_POSITION;

	// The cache is not aware in itself that the save was cancelled...
	/* THIS WOULD CAUSE UNDEFINED BEHAVIOUR
	Handle<DerivedPO> dpo2c
	(	Handle<DerivedPO>::create_unchecked(*pdbc, 2)
	);
	LOG_POSITION;
	CHECK_EQUAL(dpo2c->id(), 2);
	LOG_POSITION;
	CHECK_EQUAL(dpo2c->x(), -17);
	LOG_POSITION;
	CHECK_EQUAL(dpo2c->y(), 64.29382);
	LOG_POSITION;
	*/

	// ... That's why we should not use unchecked_get_handle unless
	// we're sure we've got a valid id. The "normal" get_handle
	// throws here.
	bool ok = false;
	LOG_POSITION;
	try
	{
		LOG_POSITION;
		Handle<DerivedPO> dpo2c_checked(*pdbc, 2);
		LOG_POSITION;
	}
	catch (BadIdentifier&)
	{
		LOG_POSITION;
		ok = true;
		LOG_POSITION;
	}
	LOG_POSITION;
	CHECK(ok);

	LOG_POSITION;
	// This will save over the top of the old
	// one...
	LOG_POSITION;
	Handle<DerivedPO> dpo5(*pdbc);
	LOG_POSITION;
	dpo5->set_x(12);
	LOG_POSITION;
	dpo5->set_y(19);
	LOG_POSITION;
	dpo5->save();
	LOG_POSITION;

	CHECK_EQUAL(dpo5->id(), 2);
	LOG_POSITION;
	CHECK_EQUAL(dpo5->x(), 12);
	LOG_POSITION;
	CHECK_EQUAL(dpo5->y(), 19);
	LOG_POSITION;
	
	// The old objects still exist in memory
	CHECK_EQUAL(dpo2b->x(), -17);
	LOG_POSITION;
	CHECK_EQUAL(dpo2->y(), 64.29382);
	LOG_POSITION;

	Handle<DerivedPO> dpo2d(*pdbc, 2);
	LOG_POSITION;
	CHECK_EQUAL(dpo2d->x(), 12);
	LOG_POSITION;
	CHECK_EQUAL(dpo2d->y(), 19);
	LOG_POSITION;

	bool ok2 = false;
	LOG_POSITION;
	try
	{
		LOG_POSITION;
		Handle<DerivedPO> dpo7(*pdbc, 7);
	}
	catch (BadIdentifier&)
	{
		LOG_POSITION;
		ok2 = true;
	}
	LOG_POSITION;
	CHECK(ok2);
	LOG_POSITION;
	
	bool ok3 = false;
	try
	{
		Handle<DerivedPO> dpo4b(*pdbc, 4);
	}
	catch (BadIdentifier&)
	{
		ok3 = true;
	}
	CHECK(ok3);

	LOG_POSITION;
	
	/* If dpo4 still has a "residual id", then this results in
	 * undefined behaviour.
	// dpo4->remove();
	LOG_POSITION;
	*/
	// But it still exists in memory with its attributes. That's OK.
	CHECK_EQUAL(dpo4->x(), 321);
	LOG_POSITION;
	CHECK_EQUAL(dpo4->y(), 1324.6);
	LOG_POSITION;
}

TEST_FIXTURE(DerivedPOFixture, test_derived_po_exists_and_remove)
{
	LOG_POSITION;
	Handle<DerivedPO> dpo1(*pdbc);
	LOG_POSITION;
	dpo1->set_x(7);
	LOG_POSITION;
	dpo1->set_y(5.8);
	LOG_POSITION;
	dpo1->save();
	LOG_POSITION;
	SQLStatement selector
	(	*pdbc,
		"select derived_po_id from derived_pos"
	);
	bool check = selector.step();
	CHECK(check);
	bool exists =
		PersistentObject<DerivedPO, DerivedDatabaseConnection>::exists
		(	*pdbc,
			1
		);
	CHECK(exists);
	Handle<DerivedPO> dpo1b = dpo1;
	dpo1->remove();
	selector.reset();
	check = selector.step();
	CHECK(!check);
	exists =
		PersistentObject<DerivedPO, DerivedDatabaseConnection>::exists
		(	*pdbc,
			1
		);
	CHECK(!exists);
	CHECK_THROW(dpo1b->id(), UninitializedOptionalException);
	CHECK_THROW(dpo1->id(), UninitializedOptionalException);

	// Now let's mix with DatabaseTransaction
	Handle<DerivedPO> dpo2(*pdbc);
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
		Handle<DerivedPO> dpo2b(*pdbc, 2);
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
	Handle<DerivedPO> dpo2c(*pdbc, 2);
	CHECK_EQUAL(dpo2c->y(), 50.78);
	CHECK_EQUAL(dpo2c->x(), 10);
}

TEST_FIXTURE(DerivedPOFixture, test_derived_po_id_getter)
{
	Handle<DerivedPO> dpo1(*pdbc);
	CHECK_THROW(dpo1->id(), UninitializedOptionalException);
	dpo1->save();
	CHECK_EQUAL(dpo1->id(), 1);
	Handle<DerivedPO> dpo2(*pdbc);
	CHECK_THROW(dpo2->id(), UninitializedOptionalException);
	CHECK_THROW(dpo2->id(), UninitializedOptionalException);
	dpo2->save();
	CHECK_EQUAL(dpo2->id(), 2);
	dpo2->save();
	CHECK_EQUAL(dpo2->id(), 2);
	dpo2->remove();
	CHECK_THROW(dpo2->id(), UninitializedOptionalException);
}

TEST_FIXTURE(DerivedPOFixture, test_load_indirectly)
{
	// load is protected method but we here we test it indirectly
	// via getting functions we know call it
	Handle<DerivedPO> dpo1(*pdbc);
	int const a = 2097601234;
	double const b = 72973.2987300;
	dpo1->set_x(a);
	dpo1->set_y(b);
	assert (dpo1->x() == a);
	assert (dpo1->y() == b);
	dpo1->save();

	Handle<DerivedPO> dpo2(*pdbc, 1);
	CHECK_EQUAL(dpo2->id(), 1);
	CHECK_EQUAL(dpo2->x(), a);  // load called here
	CHECK_EQUAL(dpo2->y(), b);  // and here
}

TEST_FIXTURE(DerivedPOFixture, test_ghostify)
{
	Handle<DerivedPO> dpo1(*pdbc);
	dpo1->set_x(1290387);
	dpo1->set_y(127);
	dpo1->save();
	Handle<DerivedPO> dpo2(*pdbc);
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

TEST(test_derived_po_self_test)
{
	// Tests certain protected functions of PersistentObject
	CHECK_EQUAL(DerivedPO::self_test(), 0);
}

}  // namespace tests
}  // namespace sqloxx

