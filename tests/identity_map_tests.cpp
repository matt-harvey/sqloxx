// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "derived_po.hpp"
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

// TODO Uncomment the commented-out tests and adjust them to
// reflect that those functions are now private.


TEST_FIXTURE(DerivedPOFixture, identity_map_after_object_removal)
{
	DerivedDatabaseConnection& dbc = *pdbc;
	Handle<DerivedPO> dpo1(dbc);
	dpo1->set_x(10);
	dpo1->set_y(-1298);
	dpo1->save();
	Handle<DerivedPO> dpo1b(dbc, 1);  // OK
	dpo1->remove();
	bool ok = false;
	try
	{
		Handle<DerivedPO> dpo1c(*pdbc, 1);
	}
	catch (BadIdentifier&)
	{
		ok = true;
	}
	CHECK(ok);
	// But this doesn't throw
	Handle<DerivedPO> dpo1d
	(	Handle<DerivedPO>::create_unchecked(dbc, 1)
	);
}

TEST_FIXTURE(DerivedPOFixture, identity_map_connection)
{
	DerivedDatabaseConnection& dbc = *pdbc;
	IdentityMap<DerivedPO> idm(dbc);
	CHECK_EQUAL(&(idm.connection()), &dbc);
}


}  // namespace tests
}  // namespace sqloxx
