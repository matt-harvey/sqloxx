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

/** \file test.cpp
 *
 * \brief Executes tests.
 *
 * This executable compiled from this file should not be directly executed.
 * Rather, it is designed to be executed from the Tcl script in test.tcl.
 * The C++ executable needs to be caused to crash in order to test the
 * SQL transaction handling via the DatabaseConnection class. This is
 * facilitated by having an "external" script control the execution and
 * recovery process.
 *
 * \author Matthew Harvey
 * \date 29 Sep 2012.
 *
 */


#include "atomicity_test.hpp"
#include "sqloxx_tests_common.hpp"
#include <UnitTest++/UnitTest++.h>
#include <stdexcept>
#include <iostream>

using sqloxx::DatabaseConnection;
using sqloxx::SQLStatement;
using sqloxx::tests::do_atomicity_test;
using sqloxx::tests::do_speed_test;
using std::cout;
using std::endl;
using std::string;
using std::terminate;

int main(int argc, char** argv)
{
	(void)argc;  // silence compiler warning re. unused parameter
	try
	{
		// do_speed_test();
		int failures = 0;
		failures += do_atomicity_test(argv[1]);
		cout << "Now running various unit tests using UnitTest++..."
		     << endl;
		failures += UnitTest::RunAllTests();
		cout << "There were " << failures << " failed tests." << endl;
		return failures;
	}
	// This seems pointless but is necessary to guarantee the stack is
	// fully unwound if an exception is thrown.
	catch (...)
	{
		throw;
	}
}
