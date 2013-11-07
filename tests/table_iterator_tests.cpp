/*
 * Copyright 2012-2013 Matthew Harvey
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

#include "database_connection.hpp"
#include "example.hpp"
#include "handle.hpp"
#include "table_iterator.hpp"
#include "sqloxx_exceptions.hpp"
#include "sqloxx_tests_common.hpp"
#include <UnitTest++/UnitTest++.h>

namespace sqloxx
{
namespace tests
{

// Our base TableIterator class for reading ExampleA
typedef
	TableIterator< Handle<ExampleA> >
	ExampleAHandleIter;

// A derived TableIterator class
class FiveIter: public ExampleAHandleIter
{
public:
	FiveIter(DerivedDatabaseConnection& p_database_connection):
		ExampleAHandleIter
		(	p_database_connection,
			"select example_a_id from example_as where x = 5"
		)
	{
	}
};  // ExampleAHandleIter


// Setup
void setup_table_iterator_test(DerivedDatabaseConnection& dbc)
{
	Handle<ExampleA> dpo1(dbc);
	Handle<ExampleA> dpo2(dbc);
	Handle<ExampleA> dpo3(dbc);
	Handle<ExampleA> dpo4(dbc);
	Handle<ExampleA> dpo5(dbc);

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
(	ExampleFixture,
	test_table_iterator_constructor_and_basic_functioning_1
)
{
	setup_table_iterator_test(*pdbc);

	ExampleAHandleIter const null_iter;

	size_t count = 0;
	for (ExampleAHandleIter it(*pdbc); it != null_iter; ++it) ++count;
	CHECK_EQUAL(count, static_cast<size_t>(5));

	int max = 0;
	for (ExampleAHandleIter it(*pdbc); it != null_iter; ++it)
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
(	ExampleFixture,
	test_table_iterator_constructor_and_basic_functioning_2
)
{
	setup_table_iterator_test(*pdbc);
	ExampleAHandleIter const null_iter;
	ExampleAHandleIter it
	(	*pdbc,
		"select example_a_id from example_as where y > 14.2"
	);
	size_t count = 0;
	for ( ; it != null_iter; ++it) ++count;
	CHECK_EQUAL(count, static_cast<size_t>(3));
}

TEST_FIXTURE
(	ExampleFixture,
	test_table_iterator_constructor_exceptions
)
{
	DerivedDatabaseConnection invalid_dbc;
	CHECK_THROW(ExampleAHandleIter it(invalid_dbc), InvalidConnection);
	CHECK_THROW(FiveIter it(invalid_dbc), InvalidConnection);

	setup_table_iterator_test(*pdbc);

	CHECK_THROW
	(	ExampleAHandleIter it
		(	*pdbc,
			"qselect unsyntactical gobbledigook from jbooble"
		),
		SQLiteException
	);
	CHECK_THROW
	(	ExampleAHandleIter it
		(	*pdbc,
			"select nonexistent_column from example_as"
		),
		SQLiteException
	);
	CHECK_THROW
	(	ExampleAHandleIter it
		(	*pdbc,
			"select example_a_id from nonexistent_table"
		),
		SQLiteException
	);
	CHECK_THROW
	(	ExampleAHandleIter it
		(	*pdbc,
			"select example_a_id from example_as; "
			"select example_a_id from example_as where x = 5"
		),
		TooManyStatements
	);
	CHECK_THROW
	(	ExampleAHandleIter it
		(	*pdbc,
			"select example_a_id from example_as; um"
		),
		TooManyStatements
	);
}

TEST_FIXTURE
(	ExampleFixture,
	test_table_iterator_copy_constructor
)
{
	setup_table_iterator_test(*pdbc);
	ExampleAHandleIter it(*pdbc);
	++it;
	CHECK_EQUAL((*it)->id(), 2);
	ExampleAHandleIter it2(it);
	CHECK_EQUAL((*it2)->id(), 2);
	++it2;
	CHECK_EQUAL((*it)->id(), 2);
	++it;
	CHECK_EQUAL((*it)->id(), 4);

	ExampleAHandleIter null_iter;
	ExampleAHandleIter it3(null_iter);
	CHECK(it3 == null_iter);
}

TEST_FIXTURE
(	ExampleFixture,
	test_table_iterator_empty_result_set
)
{
	setup_table_iterator_test(*pdbc);
	CHECK
	(	ExampleAHandleIter
		(	*pdbc,
			"select example_a_id from example_as where x = 76898"
		) ==
		(ExampleAHandleIter())
	);
	ExampleAHandleIter it
	(	*pdbc,
		"select example_a_id from example_as where 1 = 2"
	);
	ExampleAHandleIter null_iter;
	CHECK(it == null_iter);
	++it;
	CHECK(it == null_iter);
	CHECK(null_iter == it);
	CHECK(it++ == null_iter);
}

TEST_FIXTURE
(	ExampleFixture,
	test_table_iterator_completely_empty_table
)
{
	CHECK(ExampleAHandleIter(*pdbc) == (ExampleAHandleIter()));
	ExampleAHandleIter it(*pdbc);
	for (size_t i = 0; i != 189; ++i)
	{
		++it;
		CHECK(it == (ExampleAHandleIter()));
	}
	it++;
	it++;
	CHECK(it == (ExampleAHandleIter()));
}

TEST_FIXTURE
(	ExampleFixture,
	test_table_iterator_increment_and_deref
)
{
	setup_table_iterator_test(*pdbc);

	ExampleAHandleIter const null_it;

	ExampleAHandleIter it1
	(	*pdbc,
		"select example_a_id from example_as order by example_a_id"
	);
	int i = 1;
	for ( ; it1 != null_it; ++it1, ++i)
	{
		CHECK_EQUAL((*it1)->id(), i);
	}

	for (ExampleAHandleIter it(*pdbc) ; it != null_it; ++it)
	{
		Handle<ExampleA> dpo2 = *it;
		int const id = dpo2->id();
		switch (id)
		{
		case 1:
			CHECK_EQUAL(dpo2->x(), 0);
			CHECK_EQUAL(dpo2->y(), 14.1);
			break;
		case 2:
			CHECK_EQUAL(dpo2->x(), 5);
			CHECK_EQUAL(dpo2->y(), 14.2);
			break;
		case 3:
			CHECK_EQUAL(dpo2->x(), 10);
			CHECK_EQUAL(dpo2->y(), 14.3);
			break;
		case 4:
			CHECK_EQUAL(dpo2->x(), 0);
			CHECK_EQUAL(dpo2->y(), 14.4);
			break;
		case 5:
			CHECK_EQUAL(dpo2->x(), 5);
			CHECK_EQUAL(dpo2->y(), 14.5);
			break;
		default:
			CHECK(false);  // Execution should never reach here.
			break;
		}
	}
}

TEST_FIXTURE
(	ExampleFixture,
	test_table_iterator_cycling_through_results_set
)
{
	setup_table_iterator_test(*pdbc);
	ExampleAHandleIter it(*pdbc);
	ExampleAHandleIter const null_iter;
	CHECK_EQUAL((*it)->id(), 1);
	++it;
	CHECK_EQUAL((*it)->id(), 2);
	while (it != null_iter) ++it;
	CHECK(it == ExampleAHandleIter());
	++it;
	CHECK(it != null_iter);
	CHECK_EQUAL((*it)->id(), 1);
	++it;
	CHECK_EQUAL((*it)->id(), 2);
}

TEST_FIXTURE
(	ExampleFixture,
	test_table_iterator_postfix_increment
)
{
	ExampleAHandleIter null_iter;
	null_iter++;  // Does nothing

	setup_table_iterator_test(*pdbc);
	ExampleAHandleIter it(*pdbc);
	CHECK_EQUAL((*it)->id(), 1);
	it++;	
	CHECK_EQUAL((*it)->id(), 2);
	CHECK_EQUAL((*it++)->id(), 2);
	CHECK_EQUAL((*it)->id(), 3);

	ExampleAHandleIter jt(*pdbc);
	jt++;
	CHECK_EQUAL((*jt)->id(), 2);
	ExampleAHandleIter jt2(jt++);
	CHECK_EQUAL((*jt2)->id(), 2);
	CHECK_EQUAL((*jt2)->id(), 2);
	++jt2;

	// because jt2 still references the same underlying
	// SQLStatement as jt.
	CHECK_EQUAL((*jt2)->id(), 4);
}

TEST_FIXTURE
(	ExampleFixture,
	test_table_iterator_equality_and_inequality
)
{
	setup_table_iterator_test(*pdbc);

	ExampleAHandleIter it1(*pdbc);
	ExampleAHandleIter it2(*pdbc);
	ExampleAHandleIter const null_iter1;
	ExampleAHandleIter const null_iter2;

	CHECK(it1 != it2);
	CHECK(null_iter1 == null_iter2);

	while (it1 != null_iter1)
	{
		CHECK(it1 != it2);
		CHECK(it2 != null_iter2);
		CHECK(it1 != null_iter2);
		CHECK(it2 != null_iter1);
		++it1;
		++it2;
	}

	CHECK(it1 == it2);
	CHECK(it1 == null_iter1);
	CHECK(it2 == null_iter2);
	CHECK(it1 == null_iter2);
	CHECK(it2 == null_iter1);
	CHECK(null_iter1 == null_iter2);
}
	
	

}  // namespace tests
}  // namespace sqloxx
