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
#include "identity_map.hpp"
#include "sqloxx_exceptions.hpp"
#include "sqloxx_tests_common.hpp"
#include <UnitTest++/UnitTest++.h>

namespace sqloxx
{
namespace tests
{

// Note many of the functions of IdentityMap can only be tested
// indirectly via the database connection, as they are
// private.

TEST_FIXTURE(ExampleFixture, identity_map_after_object_removal)
{
	DerivedDatabaseConnection& dbc = *pdbc;
	Handle<ExampleA> dpo1(dbc);
	dpo1->set_x(10);
	dpo1->set_y(-1298);
	dpo1->save();
	Handle<ExampleA> dpo1b(dbc, 1);  // OK
	dpo1->remove();
	bool ok = false;
	try
	{
		Handle<ExampleA> dpo1c(*pdbc, 1);
	}
	catch (BadIdentifier&)
	{
		ok = true;
	}
	CHECK(ok);
	// But this doesn't throw
	Handle<ExampleA> dpo1d
	(	Handle<ExampleA>::create_unchecked(dbc, 1)
	);
}

TEST_FIXTURE(ExampleFixture, identity_map_connection)
{
	DerivedDatabaseConnection& dbc = *pdbc;
	IdentityMap<ExampleA> idm(dbc);
	CHECK_EQUAL(&(idm.connection()), &dbc);
}


}  // namespace tests
}  // namespace sqloxx
