// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "derived_po.hpp"
#include "sqloxx_tests_common.hpp"
#include "../database_connection.hpp"
#include "../handle.hpp"
#include "../table_iterator.hpp"
#include "../sqloxx_exceptions.hpp"
#include <UnitTest++/UnitTest++.h>

namespace sqloxx
{
namespace tests
{

// Our base TableIterator class for reading DerivedPO
typedef
	TableIterator< Handle<DerivedPO>, DerivedDatabaseConnection >
	DerivedPOHandleIter;

// A derived TableIterator class
class FiveIter: public DerivedPOHandleIter
{
public:
	FiveIter(DerivedDatabaseConnection& p_database_connection):
		DerivedPOHandleIter
		(	p_database_connection,
			"select derived_po_id from derived_pos where x = 5"
		)
	{
	}
};  // DerivedPOHandleIter


// Setup
void setup_table_iterator_test(DerivedDatabaseConnection& dbc)
{
	Handle<DerivedPO> dpo1(dbc);
	Handle<DerivedPO> dpo2(dbc);
	Handle<DerivedPO> dpo3(dbc);
	Handle<DerivedPO> dpo4(dbc);
	Handle<DerivedPO> dpo5(dbc);

	dpo1->set_x(0);
	dpo1->set_y(14.1);

	dpo2->set_x(5);
	dpo2->set_y(14.2);

	dpo3->set_x(10);
	dpo3->set_y(14.3);

	dpo4->set_x(0);
	dpo4->set_y(14.4);

	dpo5->set_x(5);
	dpo5->set_y(14.5);

	dpo1->save();
	dpo2->save();
	dpo3->save();
	dpo4->save();
	dpo5->save();
}

TEST_FIXTURE
(	DerivedPOFixture,
	test_table_iterator_constructor_and_basic_functioning_1
)
{
	setup_table_iterator_test(*pdbc);

	DerivedPOHandleIter const null_iter;

	size_t count = 0;
	for (DerivedPOHandleIter it(*pdbc); it != null_iter; ++it) ++count;
	CHECK_EQUAL(count, 5);

	int max = 0;
	for (DerivedPOHandleIter it(*pdbc); it != null_iter; ++it)
	{
		max =
		(	((*it)->x() > max)?
			(*it)->x():
			max
		);
	}
	CHECK_EQUAL(max, 10);
}

TEST_FIXTURE
(	DerivedPOFixture,
	test_table_iterator_constructor_and_basic_functioning_2
)
{
	setup_table_iterator_test(*pdbc);
	DerivedPOHandleIter const null_iter;
	DerivedPOHandleIter it
	(	*pdbc,
		"select derived_po_id from derived_pos where y > 14.2"
	);
	size_t count = 0;
	for ( ; it != null_iter; ++it) ++count;
	CHECK_EQUAL(count, 3);
}

TEST_FIXTURE
(	DerivedPOFixture,
	test_table_iterator_constructor_exceptions
)
{
	DerivedDatabaseConnection invalid_dbc;
	CHECK_THROW(DerivedPOHandleIter it(invalid_dbc), InvalidConnection);
	CHECK_THROW(FiveIter it(invalid_dbc), InvalidConnection);

	setup_table_iterator_test(*pdbc);

	CHECK_THROW
	(	DerivedPOHandleIter it
		(	*pdbc,
			"qselect unsyntactical gobbledigook from jbooble"
		),
		SQLiteException
	);
	CHECK_THROW
	(	DerivedPOHandleIter it
		(	*pdbc,
			"select nonexistent_column from derived_pos"
		),
		SQLiteException
	);
	CHECK_THROW
	(	DerivedPOHandleIter it
		(	*pdbc,
			"select derived_po_id from nonexistent_table"
		),
		SQLiteException
	);
	CHECK_THROW
	(	DerivedPOHandleIter it
		(	*pdbc,
			"select derived_po_id from derived_pos; "
			"select derived_po_id from derived_pos where x = 5"
		),
		TooManyStatements
	);
	CHECK_THROW
	(	DerivedPOHandleIter it
		(	*pdbc,
			"select derived_po_id from derived_pos; um"
		),
		TooManyStatements
	);
}
	
		

	
	
	

}  // namespace tests
}  // namespace sqloxx
