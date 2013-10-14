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


#include "derived_po.hpp"
#include "handle.hpp"
#include "sqloxx_tests_common.hpp"
#include <jewel/optional.hpp>
#include <UnitTest++/UnitTest++.h>

using jewel::UninitializedOptionalException;

namespace sqloxx
{
namespace tests
{

TEST_FIXTURE(DerivedPOFixture, handle_constructors)
{
	JEWEL_LOG_TRACE();
	Handle<DerivedPO> const dpo1(*pdbc);
	dpo1->set_x(10);
	CHECK_EQUAL(dpo1->x(), 10);
	dpo1->set_y(50000.9812);
	CHECK_EQUAL(dpo1->y(), 50000.9812);
	dpo1->save();

	Handle<DerivedPO> const dpo1b(*pdbc, 1);
	CHECK_EQUAL(dpo1b->x(), 10);
	CHECK_EQUAL(dpo1b->y(), 50000.9812);

	Handle<DerivedPO> dpo2(*pdbc);
	dpo2->set_x(503);
	dpo2->set_y(-1.3);
	dpo2->save();

	Handle<DerivedPO> const dpo2b(*pdbc, 2);
	CHECK_EQUAL(dpo2b->x(), 503);
	CHECK_EQUAL(dpo2b->y(), -1.3);

	Handle<DerivedPO> dpo2c(*pdbc, 2);
	CHECK_EQUAL(dpo2c->y(), -1.3);
	CHECK_EQUAL(dpo2c->x(), 503);

	CHECK_THROW
	(	Handle<DerivedPO> const dpo3(*pdbc, 3),
		sqloxx::BadIdentifier
	);

	CHECK_THROW
	(	Handle<DerivedPO> dpo0(*pdbc, 0),
		sqloxx::BadIdentifier
	);
	JEWEL_LOG_TRACE();
}

TEST_FIXTURE(DerivedPOFixture, handle_create_unchecked)
{
	JEWEL_LOG_TRACE();

	Handle<DerivedPO> const dpo1(*pdbc);
	dpo1->set_x(10);
	CHECK_EQUAL(dpo1->x(), 10);
	dpo1->set_y(50000.9812);
	CHECK_EQUAL(dpo1->y(), 50000.9812);
	dpo1->save();

	Handle<DerivedPO> const dpo1b =
		Handle<DerivedPO>::create_unchecked(*pdbc, 1);
	CHECK_EQUAL(dpo1b->x(), 10);
	CHECK_EQUAL(dpo1b->y(), 50000.9812);

	Handle<DerivedPO> dpo2(*pdbc);
	dpo2->set_x(503);
	dpo2->set_y(-1.3);
	dpo2->save();

	CHECK_EQUAL(Handle<DerivedPO>::create_unchecked(*pdbc, 2)->x(), 503);
	Handle<DerivedPO> const dpo2b
	(	Handle<DerivedPO>::create_unchecked(*pdbc, 2)
	);
	CHECK_EQUAL(dpo2b->y(), -1.3);

	JEWEL_LOG_TRACE();
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
	CHECK(dpo1);
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
	CHECK_EQUAL(static_cast<bool>(dpo1), true);
	CHECK(static_cast<bool>(dpo1));

	// However handle is not valid if completely uninitialized.
	Handle<DerivedPO> dpo2;
	CHECK_EQUAL(static_cast<bool>(dpo2), false);
	dpo1 = dpo2;
	CHECK_EQUAL(static_cast<bool>(dpo2), false);
	CHECK(!dpo2);
	
}

TEST_FIXTURE(DerivedPOFixture, handle_equality_and_inequality)
{
	Handle<DerivedPO> dpo1(*pdbc);
	Handle<DerivedPO> dpo2(dpo1);
	CHECK(dpo1 == dpo2);
	CHECK(dpo2 == dpo1);

	Handle<DerivedPO> const dpo3(*pdbc);
	dpo3->set_x(109);
	dpo3->set_y(.5);
	dpo3->save();
	CHECK(dpo3 != dpo1);
	CHECK(dpo2 != dpo3);

	Handle<DerivedPO> dpo4(*pdbc, 1);
	CHECK(dpo4 == dpo3);
	CHECK(dpo3 == dpo4);

	Handle<DerivedPO> const dpo5(*pdbc);
	dpo5->save();

	Handle<DerivedPO> const dpo6(*pdbc, 2);
	CHECK(dpo6 == dpo5);
	CHECK(dpo5 == dpo6);
	CHECK(dpo6 != dpo4);
	CHECK(dpo3 != dpo6);
	CHECK(dpo4 != dpo6);
}


}  // namespace tests
}  // namespace sqloxx
