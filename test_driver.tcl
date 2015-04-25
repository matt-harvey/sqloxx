#!/usr/bin/env tclsh

###
# Copyright 2013 Matthew Harvey
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###

# Execute this script to run the unit tests.

# This Tcl script is intended to drive the unit tests. We drive them from here
# rather than directly from main, so that we can crash and then revive
# the main program, in the middle of a SQL transaction, and inspect the
# database afterwards, to see if the failed transaction was handled as
# expected. On reviving, the main function will run again, detect the
# existence of the crashed database file, check that the database state
# is as expected given the crash, output the result of this check to
# standard output, and then run the remaining unit tests.
#
# Normally this script should be run as "test.tcl" with no arguments.
# However if you pass additional arguments, these are prefixed within
# the internal execution command for running the test suite.
# Thus running "test.tcl valgrind" will cause the test suite
# to be run through the valgrind program.

set filename testfile9182734123.db

if {[file exists $filename] || [file exists ${filename}-journal]} {
    puts "File named $filename and/or ${filename}-journal already exists. "
    puts "Test aborted as unsafe to proceed."
    exit 1
}

puts "Running atomicity test..."

# This execution crashes, but we recover
catch { exec {*}$argv ./test_engine $filename 2>@ stderr >@ stdout }

# And in this second execution we inspect the database to see that it
# reacted as expected; and then we perform the other unit tests.
catch { exec {*}$argv ./test_engine $filename 2>@ stderr >@ stdout }

# And clean up left over files
catch { file delete $filename }
catch { file delete ${filename}-journal }




