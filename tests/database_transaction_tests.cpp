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

#include "database_transaction.hpp"
#include "sql_statement.hpp"
#include "sqloxx_exceptions.hpp"
#include "sqloxx_tests_common.hpp"
#include <UnitTest++/UnitTest++.h>

namespace sqloxx
{
namespace tests
{

TEST_FIXTURE(DatabaseConnectionFixture, test_transaction_nesting_exception_01)
{
	DatabaseTransaction transaction1(*pdbc);
	transaction1.cancel();
	CHECK_THROW(transaction1.cancel(), TransactionNestingException);

	DatabaseTransaction transaction2(*pdbc);
	transaction2.cancel();
	CHECK_THROW(transaction2.commit(), TransactionNestingException);

	DatabaseTransaction transaction3(*pdbc);
	transaction3.commit();
	CHECK_THROW(transaction3.cancel(), TransactionNestingException);

	DatabaseTransaction transaction4(*pdbc);
	transaction4.commit();
	CHECK_THROW(transaction4.commit(), TransactionNestingException);
}


TEST_FIXTURE(DatabaseConnectionFixture, test_transaction_nesting_exception_02)
{
	DatabaseTransaction transaction1(*pdbc);	

	pdbc->execute_sql("create table dummy(col_A text, col_B text)");
	pdbc->execute_sql
	(	"insert into dummy(col_A, col_B) values('Hello', 'Goodbye')"
	);
	pdbc->execute_sql
	(	"insert into dummy(col_A, col_B) values('Yeah!', 'What!')"
	);
	
	DatabaseTransaction transaction2(*pdbc);

	SQLStatement statement
	(	*pdbc,
		"select col_A from dummy where col_B = 'Goodbye'"
	);
	statement.step();
	statement.step_final();

	transaction2.commit();
	CHECK_THROW(transaction2.commit(), TransactionNestingException);
	transaction1.commit();
	CHECK_THROW(transaction1.commit(), TransactionNestingException);
	CHECK_THROW(transaction1.cancel(), TransactionNestingException);
}


TEST(test_transaction_response_to_invalid_connection)
{
	DatabaseConnection invaliddbc;
	CHECK_THROW
	(	DatabaseTransaction transaction(invaliddbc),
		InvalidConnection
	);
}


/*
 * For test of whether DatabaseTransaction
 * actually does enable access to
 * the atomicity of its underlying SQL commands, see
 * atomicity_test.hpp, test.cpp and test.tcl.
 */


TEST_FIXTURE(DatabaseConnectionFixture, test_sqlite_rollback)
{
	// This is not actually aimed at testing anything in sqloxx, but is more
	// aimed at verifying that certain SQLite SQL commands do what we expect.
	pdbc->execute_sql("create table dummy(col_A integer)");
	pdbc->execute_sql("begin transaction");
	pdbc->execute_sql("insert into dummy(col_A) values(3)");
	pdbc->execute_sql("savepoint sp");
	pdbc->execute_sql("insert into dummy(col_A) values(4)");
	pdbc->execute_sql("rollback to savepoint sp");
	pdbc->execute_sql("release sp");
	pdbc->execute_sql("end transaction");
	SQLStatement s1(*pdbc, "select col_A from dummy where col_A = 3");
	s1.step();
	CHECK_EQUAL(s1.extract<int>(0), 3);
	s1.step_final();
	SQLStatement s2(*pdbc, "select col_A from dummy where col_A = 4");
	CHECK_EQUAL(s2.step(), false);
	SQLStatement s3(*pdbc, "select * from dummy");
	s3.step();
	s3.step_final();  // As only one record.
}


TEST_FIXTURE(DatabaseConnectionFixture, transaction_constructor_destructor)
{
	DatabaseConnection invalid_dbc;
	CHECK_THROW(DatabaseTransaction t1(invalid_dbc), InvalidConnection);

	pdbc->execute_sql("create table dummy(Col_A)");
	pdbc->execute_sql("insert into dummy(Col_A) values(12)");
	SQLStatement selector(*pdbc, "select Col_A from dummy");
	for (int i = 0; i != 5; ++i)
	{
		DatabaseTransaction t2(*pdbc);
		pdbc->execute_sql("delete from dummy where Col_A = 12");
		bool const check_inner = selector.step();
		CHECK(!check_inner);
		selector.reset();
		// Destructor of t2 called when scope left, cancelling transaction
	}
	bool const check_outer = selector.step();
	CHECK(check_outer);

	selector.reset();
	for (int i = 0; i != 5; ++i)
	{
		DatabaseTransaction t2_b(*pdbc);
		pdbc->execute_sql("delete from dummy where Col_A = 12");
		bool const check_inner_b = selector.step();
		CHECK(!check_inner_b);
		selector.reset();
		t2_b.commit();
		// Destructor of t2 called when scope left, but as transaction has
		// now been committed, it is not cancelled.
	}
	bool const check_outer_b = selector.step();
	CHECK(!check_outer_b);
}


TEST_FIXTURE(DatabaseConnectionFixture, test_commit_and_cancel_transaction_A)
{
	pdbc->execute_sql("create table dummy(Col_A)");
	DatabaseTransaction transaction1(*pdbc);
	pdbc->execute_sql("insert into dummy(Col_A) values(20)");
	DatabaseTransaction transaction2(*pdbc);
	transaction2.commit();
	SQLStatement selector(*pdbc, "select Col_A from dummy");
	selector.step();
	CHECK_EQUAL(selector.extract<int>(0), 20);
	transaction1.cancel();
	selector.reset();
	bool quick_check = selector.step();
	CHECK(!quick_check);
}

TEST_FIXTURE(DatabaseConnectionFixture, test_cancel_transaction_B)
{
	pdbc->execute_sql("create table dummy(col_A)");

	// Part A

	DatabaseTransaction transaction1(*pdbc);

	pdbc->execute_sql("insert into dummy(col_A) values(3)");

	DatabaseTransaction transaction2(*pdbc);
	pdbc->execute_sql("insert into dummy(col_A) values(4)");
	transaction2.cancel();

	transaction1.commit();

	SQLStatement s1(*pdbc, "select * from dummy");
	CHECK_EQUAL(s1.step(), true);
	CHECK_EQUAL(s1.extract<int>(0), 3);
	CHECK_EQUAL(s1.step(), false);

	// Part B

	DatabaseTransaction transaction3(*pdbc);
	pdbc->execute_sql("insert into dummy(col_A) values(100)");
	transaction3.cancel();

	CHECK_THROW(transaction3.commit(), TransactionNestingException);
	SQLStatement s2(*pdbc, "select * from dummy where col_A = 100");
	CHECK_EQUAL(s2.step(), false);

	// Part C

	DatabaseTransaction transaction4(*pdbc);

	pdbc->execute_sql("insert into dummy(col_A) values(200)");

	DatabaseTransaction transaction5(*pdbc);
	DatabaseTransaction transaction6(*pdbc);
	pdbc->execute_sql("insert into dummy(col_A) values(200)");
	transaction6.commit();
	transaction5.commit();
	transaction4.cancel();

	CHECK_THROW
	(	transaction4.cancel(),
		TransactionNestingException
	);
	SQLStatement s3(*pdbc, "select * from dummy where col_A = 200");
	CHECK_EQUAL(s3.step(), false);
}


}  // namespace tests
}  // namespace sqloxx
