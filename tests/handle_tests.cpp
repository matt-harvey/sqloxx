#include "derived_po.hpp"
#include "sqloxx_tests_common.hpp"
#include "../handle.hpp"
#include <jewel/optional.hpp>
#include <UnitTest++/UnitTest++.h>

using jewel::UninitializedOptionalException;

namespace sqloxx
{
namespace tests
{

TEST_FIXTURE(DerivedPOFixture, handle_constructors)
{
	Handle<DerivedPO> dpo1(*pdbc);
	dpo1->set_x(10);
	CHECK_EQUAL(dpo1->x(), 10);
	dpo1->set_y(50000.9812);
	CHECK_EQUAL(dpo1->y(), 50000.9812);
	dpo1->save();

	Handle<DerivedPO> dpo1b(*pdbc, 1);
	CHECK_EQUAL(dpo1b->x(), 10);
	CHECK_EQUAL(dpo1b->y(), 50000.9812);
}


TEST_FIXTURE(DerivedPOFixture, handle_copy_constructor_and_indirection)
{
	Handle<DerivedPO> dpo1(*pdbc);
	dpo1->set_x(-9);
	Handle<DerivedPO> dpo2(dpo1);
	dpo2->set_y(102928);
	CHECK_EQUAL(dpo2->x(), -9);
	dpo2->save();
	CHECK_EQUAL(dpo1->id(), 1);
	CHECK_EQUAL(dpo2->id(), dpo1->id());
	CHECK_EQUAL(dpo1->y(), 102928);
	Handle<DerivedPO> dpo3(dpo1);
	CHECK_EQUAL(dpo3->id(), 1);
	CHECK_EQUAL(dpo3->y(), 102928);
	CHECK_EQUAL(dpo3->x(), dpo1->x());
}

TEST_FIXTURE(DerivedPOFixture, handle_assignment_and_indirection)
{
	Handle<DerivedPO> dpo1(*pdbc);
	Handle<DerivedPO> dpo2(*pdbc);
	dpo2->set_x(100);
	dpo2->set_y(0.0112);
	dpo2->save();
	dpo1->set_x(897);
	dpo1->set_y(30978);
	dpo2 = dpo1;
	CHECK_EQUAL(dpo2->x(), dpo1->x());
	CHECK_EQUAL(dpo2->y(), 30978);
	dpo1->save();
	CHECK_EQUAL(dpo2->id(), 2);
	Handle<DerivedPO> dpo3(*pdbc, 1);
	CHECK_EQUAL(dpo3->id(), 1);
	dpo3->set_x(-188342392);
	dpo1 = dpo3;
	CHECK_EQUAL(dpo1->x(), -188342392);
	dpo1->set_y(50);
	CHECK_EQUAL(dpo1->y(), 50);
	dpo1->save();
	CHECK_EQUAL(dpo3->id(), 1);
}

TEST_FIXTURE(DerivedPOFixture, handle_dereferencing)
{
	Handle<DerivedPO> dpo1(*pdbc);
	dpo1->set_x(10);
	dpo1->set_y(1278.90172);
	dpo1->save();
	DerivedPO& dpo1_dereferenced(*dpo1);
	CHECK_EQUAL(dpo1_dereferenced.y(), dpo1->y());
	CHECK_EQUAL((*dpo1).y(), dpo1->y());
	CHECK_EQUAL(dpo1_dereferenced.id(), dpo1->id());
	CHECK_EQUAL(dpo1_dereferenced.x(), 10);
	dpo1_dereferenced.set_y(.504);
	CHECK_EQUAL(dpo1->y(), 0.504);
	Handle<DerivedPO> dpo2(*pdbc);
	DerivedPO& dpo2_dereferenced = *dpo2;
	dpo2_dereferenced.set_x(8000);
	dpo2_dereferenced.set_y(140);
	CHECK_EQUAL((*dpo2).x(), dpo2_dereferenced.x());
	CHECK_EQUAL(dpo2->y(), (*dpo2).y());
	CHECK_EQUAL(dpo2->y(), 140);
}

TEST_FIXTURE(DerivedPOFixture, handle_conversion_to_bool)
{
	Handle<DerivedPO> dpo1(*pdbc);
	CHECK(dpo1);
	CHECK_EQUAL(static_cast<bool>(dpo1), true);
	dpo1->set_y(139000000);
	dpo1->set_x(7);
	dpo1->save();
	CHECK(dpo1);

	// Handle is still valid after underlying object has
	// been removed from the database.
	dpo1->remove();
	CHECK(dpo1);
	CHECK_EQUAL(dpo1->x(), 7);
	CHECK_THROW(dpo1->id(), UninitializedOptionalException);

	// todo Think of a way to actually make it convert to false.
	// If this is literally impossible, then is there any point
	// retaining the conversion in the API? And can we do away
	// with checking for m_pointer == 0, in operator*() and
	// operator->() implementations for Handle?
}

TEST_FIXTURE(DerivedPOFixture, handle_equality_and_inequality)
{
	Handle<DerivedPO> dpo1(*pdbc);
	Handle<DerivedPO> dpo2(dpo1);
	CHECK(dpo1 == dpo2);
	CHECK_EQUAL(dpo2, dpo1);

	Handle<DerivedPO> dpo3(*pdbc);
	dpo3->set_x(109);
	dpo3->set_y(.5);
	dpo3->save();
	CHECK(dpo3 != dpo1);
	CHECK(dpo2 != dpo3);

	Handle<DerivedPO> dpo4(*pdbc, 1);
	CHECK(dpo4 == dpo3);
	CHECK_EQUAL(dpo3, dpo4);
}


}  // namespace tests
}  // namespace sqloxx
