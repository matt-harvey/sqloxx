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

#include "database_connection.hpp"
#include "next_auto_key.hpp"
#include "sqloxx_tests_common.hpp"
#include <UnitTest++/UnitTest++.h>
#include <limits>

using std::numeric_limits;

namespace sqloxx
{
namespace tests
{


TEST(test_next_auto_key_invalid_connection)
{
	DatabaseConnection db0;
	// Have to do this as CHECK_THROW gets confused by multiple template args
	bool ok = false;
	try
	{
		next_auto_key
		<	DatabaseConnection,
			int
		>	(db0, "dummy_table");
	}
	catch (InvalidConnection&)
	{
		ok = true;
	}
	CHECK(ok);
}
	

TEST_FIXTURE(DatabaseConnectionFixture, test_next_auto_key_normal)
{
	// Note CHECK_EQUAL and CHECK get confused by multiple template args
	bool ok =
		(next_auto_key<DatabaseConnection, int>(*pdbc, "dummy_table") == 1);
	CHECK(ok);
	pdbc->execute_sql
	(	"create table dummy_table(column_A text)"
	);
	ok = (next_auto_key<DatabaseConnection, int>(*pdbc, "dummy_table") == 1);
	CHECK(ok);
	ok = (next_auto_key<DatabaseConnection, int>(*pdbc, "dummy_table") == 1);
	CHECK(ok);
	pdbc->execute_sql
	(	"create table test_table"
		"("
			"column_A integer not null unique, "
			"column_B integer primary key autoincrement, "
			"column_C text not null"
		")"
	);
	ok = (next_auto_key<DatabaseConnection, int>(*pdbc, "test_table") == 1);
	CHECK(ok);
	// This behaviour is strange but expected - see API docs.
	ok = (next_auto_key<DatabaseConnection, int>(*pdbc, "dummy_table") == 1);
	CHECK(ok);
	pdbc->execute_sql
	(	"insert into test_table(column_A, column_C) "
		"values(3, 'Hello')"
	);
	pdbc->execute_sql
	(	"insert into test_table(column_A, column_C) "
		"values(4, 'Red')"
	);
	pdbc->execute_sql
	(	"insert into test_table(column_A, column_C) "
		"values(10, 'Gold')"
	);
	ok = (next_auto_key<DatabaseConnection, int>(*pdbc, "test_table") == 4);
	CHECK(ok);
	ok = (next_auto_key<DatabaseConnection, int>(*pdbc, "dummy_table") == 1);
	CHECK(ok);
	
	// Test behaviour with gaps in numbering
	pdbc->execute_sql("delete from test_table where column_B = 2");
	ok = (next_auto_key<DatabaseConnection, int>(*pdbc, "test_table") == 4);
	
	// Key is not predicted to be reused once deleted
	pdbc->execute_sql("delete from test_table where column_B = 3");
	ok = (next_auto_key<DatabaseConnection, int>(*pdbc, "test_table") == 4);
	CHECK(ok);
	int const predicted_key =
		next_auto_key<DatabaseConnection, int>(*pdbc, "test_table");

	// Check key is not actually reused once deleted
	pdbc->execute_sql
	(	"insert into test_table(column_A, column_C) "
		"values(110, 'Red')"
	);
	SQLStatement statement2
	(	*pdbc,
		"select column_B from test_table where column_A = 110"
	);
	statement2.step();
	ok = (statement2.extract<int>(0) == predicted_key);
	CHECK(ok);
	statement2.step_final();

	// Test behaviour in protecting against overflow
	SQLStatement statement
	(	*pdbc,
		"insert into test_table(column_A, column_B, column_C) "
		"values(:A, :B, :C)"
	);
	statement.bind(":A", 30);
	statement.bind(":B", numeric_limits<int>::max());
	statement.bind(":C", "Hello");
	statement.step_final();

	// Have to do this as CHECK_THROW gets confused by multiple template args
	try
	{
		next_auto_key<DatabaseConnection, int>(*pdbc, "test_table");
	}
	catch (TableSizeException&)
	{
		ok = true;
	}
	CHECK(ok);
	pdbc->execute_sql("drop table dummy_table");
	pdbc->execute_sql("drop table test_table");
}



}  // namespace tests
}  // namespace sqloxx
