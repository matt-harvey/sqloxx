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

#ifndef GUARD_sqloxx_tests_common_hpp_18678273921216965
#define GUARD_sqloxx_tests_common_hpp_18678273921216965

// Hide from Doxygen
/// @cond


/**
 * @file sqloxx_tests_common.hpp
 *
 * Contains code for common use by Sqloxx unit tests (rather
 * than specific to any particular class).
 */

#include "database_connection.hpp"
#include <UnitTest++/UnitTest++.h>
#include <boost/filesystem.hpp>
#include <iostream>


namespace sqloxx
{
namespace tests
{


bool file_exists(boost::filesystem::path const& filepath);

void catch_check_ok(DatabaseConnection& dbc);

void abort_if_exists(boost::filesystem::path const& filepath);

// To compare speed of SQLStatementImpl with SQLStatement, to
// evaluate effectiveness of caching in latter.
void do_speed_test();


// Fixture that creates a DatabaseConnection and database file for
// reuse in tests.
struct DatabaseConnectionFixture
{
	// setup
	DatabaseConnectionFixture();

	// teardown
	~DatabaseConnectionFixture();

	// Database filepath
	boost::filesystem::path db_filepath;

	// The connection to the database
	DatabaseConnection* pdbc;
};


class DerivedDatabaseConnection;  // fwd decl

struct ExampleFixture
{
	// setup
	ExampleFixture();

	// teardown
	~ExampleFixture();

	// Database filepath
	boost::filesystem::path db_filepath;

	// The connection to the database
	DerivedDatabaseConnection* pdbc;
};




}  // namespace tests
}  // namespace sqloxx

/// @endcond
// End hiding from Doxygen

#endif  // GUARD_sqloxx_tests_common_hpp_18678273921216965
