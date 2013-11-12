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
