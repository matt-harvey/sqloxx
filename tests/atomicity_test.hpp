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

#ifndef GUARD_atomicity_test_hpp_2574979389354111
#define GUARD_atomicity_test_hpp_2574979389354111

// Hide from Doxygen
/// @cond

#include "database_connection_fwd.hpp"
#include "sql_statement.hpp"
#include <string>

using std::string;

/**
 * @file atomicity_test_hpp
 *
 * This file declares a function that is designed to create a database
 * file, start a transaction by calling
 * DatabaseConnection::begin_transaction, and then crash
 * before being able to complete the transaction. The intention
 * is that an external script will catch the failure, and relaunch
 * the test, but this time, a different execution path will
 * be taken, which examines the database
 * after the crash, to ensure transaction atomicity was upheld.
 */

namespace sqloxx
{


namespace tests
{


int
do_atomicity_test(std::string const& db_filename);

void
setup_atomicity_test(DatabaseConnection& dbc);

int
inspect_database_for_atomicity(DatabaseConnection& dbc);


}  // namespace tests
}  // namespace sqloxx

/// @endcond
// End hiding from Doxygen

#endif  // GUARD_atomicity_test_hpp_2574979389354111
