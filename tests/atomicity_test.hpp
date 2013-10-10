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
