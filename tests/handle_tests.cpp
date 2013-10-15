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


#include "example.hpp"
#include "handle.hpp"
#include "sqloxx_tests_common.hpp"
#include <jewel/optional.hpp>
#include <UnitTest++/UnitTest++.h>
#include <utility>

using jewel::UninitializedOptionalException;
using std::move;

namespace sqloxx
{
namespace tests
{

TEST_FIXTURE(ExampleFixture, handle_constructors)
{
	JEWEL_LOG_TRACE();

	Handle<ExampleA> const dpo0;
	CHECK(!dpo0);
	CHECK_THROW(*dpo0, UnboundHandleException);
	CHECK_THROW(dpo0->x(), UnboundHandleException);

	Handle<ExampleA> const dpo1(*pdbc);
	dpo1->set_x(10);
	CHECK_EQUAL(dpo1->x(), 10);
	dpo1->set_y(50000.9812);
	CHECK_EQUAL(dpo1->y(), 50000.9812);
	dpo1->save();

	Handle<ExampleA> const dpo1b(*pdbc, 1);
	CHECK_EQUAL(dpo1b->x(), 10);
	CHECK_EQUAL(dpo1b->y(), 50000.9812);

	Handle<ExampleA> dpo2(*pdbc);
	dpo2->set_x(503);
	dpo2->set_y(-1.3);
	dpo2->save();

	Handle<ExampleA> const dpo2b(*pdbc, 2);
	CHECK_EQUAL(dpo2b->x(), 503);
	CHECK_EQUAL(dpo2b->y(), -1.3);

	Handle<ExampleA> dpo2c(*pdbc, 2);
	CHECK_EQUAL(dpo2c->y(), -1.3);
	CHECK_EQUAL(dpo2c->x(), 503);

	CHECK_THROW
	(	Handle<ExampleA> const dpo3(*pdbc, 3),
		sqloxx::BadIdentifier
	);

	CHECK_THROW
	(	Handle<ExampleA> dpo0(*pdbc, 0),
		sqloxx::BadIdentifier
	);
	JEWEL_LOG_TRACE();
}

TEST_FIXTURE(ExampleFixture, handle_create_unchecked)
{
	JEWEL_LOG_TRACE();

	Handle<ExampleA> const dpo1(*pdbc);
	dpo1->set_x(10);
	CHECK_EQUAL(dpo1->x(), 10);
	dpo1->set_y(50000.9812);
	CHECK_EQUAL(dpo1->y(), 50000.9812);
	dpo1->save();

	Handle<ExampleA> const dpo1b =
		Handle<ExampleA>::create_unchecked(*pdbc, 1);
	CHECK_EQUAL(dpo1b->x(), 10);
	CHECK_EQUAL(dpo1b->y(), 50000.9812);

	Handle<ExampleA> dpo2(*pdbc);
	dpo2->set_x(503);
	dpo2->set_y(-1.3);
	dpo2->save();

	CHECK_EQUAL(Handle<ExampleA>::create_unchecked(*pdbc, 2)->x(), 503);
	Handle<ExampleA> const dpo2b
	(	Handle<ExampleA>::create_unchecked(*pdbc, 2)
	);
	CHECK_EQUAL(dpo2b->y(), -1.3);

	JEWEL_LOG_TRACE();
}

TEST_FIXTURE(ExampleFixture, handle_copy_constructor_and_indirection)
{
	Handle<ExampleA> dpo1(*pdbc);
	dpo1->set_x(-9);
	Handle<ExampleA> dpo2(dpo1);
	dpo2->set_y(102928);
	CHECK_EQUAL(dpo2->x(), -9);
	dpo2->save();
	CHECK_EQUAL(dpo1->id(), 1);
	CHECK_EQUAL(dpo2->id(), dpo1->id());
	CHECK_EQUAL(dpo1->y(), 102928);
	Handle<ExampleA> dpo3(dpo1);
	CHECK_EQUAL(dpo3->id(), 1);
	CHECK_EQUAL(dpo3->y(), 102928);
	CHECK_EQUAL(dpo3->x(), dpo1->x());
}

TEST_FIXTURE(ExampleFixture, handle_move_constructor_and_indirection)
{
	Handle<ExampleA> dpo1(*pdbc);
	dpo1->set_x(-9);
	dpo1->set_y(102928);
	dpo1->save();
	Handle<ExampleA> dpo2(move(dpo1));

	// dpo1 is now "taboo".
	CHECK_EQUAL(dpo2->id(), 1);
	CHECK_EQUAL(dpo2->y(), 102928);
	CHECK_EQUAL(dpo2->x(), -9);
}

TEST_FIXTURE(ExampleFixture, handle_assignment_and_indirection)
{
	Handle<ExampleA> dpo1(*pdbc);
	Handle<ExampleA> dpo2(*pdbc);
	dpo2->set_x(100);
	dpo2->set_y(0.0112);
	dpo2->save();
	dpo1->set_x(897);
	dpo1->set_y(30978);
	dpo2 = dpo1;  // copy assignment
	CHECK_EQUAL(dpo2->x(), dpo1->x());
	CHECK_EQUAL(dpo2->y(), 30978);
	dpo1->save();
	CHECK_EQUAL(dpo2->id(), 2);
	Handle<ExampleA> dpo3(*pdbc, 1);
	CHECK_EQUAL(dpo3->id(), 1);
	dpo3->set_x(-188342392);
	dpo1 = dpo3;  // copy assignment
	CHECK_EQUAL(dpo1->x(), -188342392);
	dpo1->set_y(50);
	CHECK_EQUAL(dpo1->y(), 50);
	dpo1->save();
	CHECK_EQUAL(dpo3->id(), 1);

	Handle<ExampleA> dpo4(*pdbc);
	dpo4 = Handle<ExampleA>(*pdbc,  2);  // move assignment
	CHECK_EQUAL(dpo4->x(), 897);
	CHECK_EQUAL(dpo4->y(), 30978);
	dpo4 = Handle<ExampleA>();  // move assignment
	CHECK(!dpo4);
	dpo4 = Handle<ExampleA>(*pdbc);  // move assignment
	CHECK(dpo4);
}

TEST_FIXTURE(ExampleFixture, handle_dereferencing)
{
	Handle<ExampleA> dpo1(*pdbc);
	dpo1->set_x(10);
	dpo1->set_y(1278.90172);
	dpo1->save();
	ExampleA& dpo1_dereferenced(*dpo1);
	CHECK_EQUAL(dpo1_dereferenced.y(), dpo1->y());
	CHECK_EQUAL((*dpo1).y(), dpo1->y());
	CHECK_EQUAL(dpo1_dereferenced.id(), dpo1->id());
	CHECK_EQUAL(dpo1_dereferenced.x(), 10);
	dpo1_dereferenced.set_y(.504);
	CHECK_EQUAL(dpo1->y(), 0.504);
	Handle<ExampleA> dpo2(*pdbc);
	ExampleA& dpo2_dereferenced = *dpo2;
	dpo2_dereferenced.set_x(8000);
	dpo2_dereferenced.set_y(140);
	CHECK_EQUAL((*dpo2).x(), dpo2_dereferenced.x());
	CHECK_EQUAL(dpo2->y(), (*dpo2).y());
	CHECK_EQUAL(dpo2->y(), 140);

	Handle<ExampleA> dpo4;
	CHECK_THROW(dpo4->y(), UnboundHandleException);
	CHECK_THROW(*dpo4, UnboundHandleException);
}

TEST_FIXTURE(ExampleFixture, handle_conversion_to_bool)
{
	Handle<ExampleA> dpo1(*pdbc);
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
	Handle<ExampleA> dpo2;
	CHECK_EQUAL(static_cast<bool>(dpo2), false);
	dpo1 = dpo2;
	CHECK_EQUAL(static_cast<bool>(dpo2), false);
	CHECK(!dpo2);
	
}

TEST_FIXTURE(ExampleFixture, handle_equality_and_inequality)
{
	Handle<ExampleA> dpo1(*pdbc);
	Handle<ExampleA> dpo2(dpo1);
	CHECK(dpo1 == dpo2);
	CHECK(dpo2 == dpo1);

	Handle<ExampleA> const dpo3(*pdbc);
	dpo3->set_x(109);
	dpo3->set_y(.5);
	dpo3->save();
	CHECK(dpo3 != dpo1);
	CHECK(dpo2 != dpo3);

	Handle<ExampleA> dpo4(*pdbc, 1);
	CHECK(dpo4 == dpo3);
	CHECK(dpo3 == dpo4);

	Handle<ExampleA> const dpo5(*pdbc);
	dpo5->save();

	Handle<ExampleA> const dpo6(*pdbc, 2);
	CHECK(dpo6 == dpo5);
	CHECK(dpo5 == dpo6);
	CHECK(dpo6 != dpo4);
	CHECK(dpo3 != dpo6);
	CHECK(dpo4 != dpo6);
}

TEST_FIXTURE(ExampleFixture, handle_cast_test)
{
	Handle<ExampleC> dpoc1(*pdbc);
	dpoc1->set_s("hello");
	dpoc1->set_p(3);
	dpoc1->set_q(50);
	dpoc1->save();

	Handle<ExampleC> dpoc2(*pdbc);
	dpoc2->set_s("goodbye");
	dpoc2->set_p(-309);
	dpoc2->set_q(501);
	dpoc2->save();

	Handle<ExampleB> dpob1;
	CHECK_THROW(*dpob1, UnboundHandleException);
	dpob1 = handle_cast<ExampleB>(dpoc2);
	CHECK(dpob1);
	CHECK_EQUAL(dpob1->id(), 2);
	CHECK_EQUAL(dpob1->s(), "goodbye");

	Handle<ExampleB> dpob2 = handle_cast<ExampleB>(dpob1);
	CHECK(dpob2);
	CHECK_EQUAL(dpob2->id(), 2);
	CHECK_EQUAL(dpob2->s(), "goodbye");
	dpob2->set_s("yeah");
	CHECK_EQUAL(dpoc2->s(), "yeah");
	CHECK_EQUAL(dpob1->s(), "yeah");
	dpoc2->set_s("mmm");
	CHECK_EQUAL(dpob2->s(), "mmm");

	Handle<ExampleC> dpoc3;
	CHECK(!dpoc3);
	dpoc3 = handle_cast<ExampleC>(dpob2);
	CHECK(dpoc3);
	CHECK_EQUAL(dpoc3->q(), 501);
	CHECK_EQUAL(dpoc3->id(), 2);
	CHECK_EQUAL(dpoc3->p(), -309);
	CHECK_EQUAL(dpoc3->s(), "mmm");

	Handle<ExampleB> dpoc4;
	dpoc3 = handle_cast<ExampleC>(dpoc4);
	CHECK_THROW(dpoc3->s(), UnboundHandleException);
	CHECK_THROW(dpoc3->p(), UnboundHandleException);
	CHECK(!dpoc3);
}


}  // namespace tests
}  // namespace sqloxx
