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

struct DerivedPOFixture
{
	// setup
	DerivedPOFixture();

	// teardown
	~DerivedPOFixture();

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
