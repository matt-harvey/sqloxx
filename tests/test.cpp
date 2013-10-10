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
