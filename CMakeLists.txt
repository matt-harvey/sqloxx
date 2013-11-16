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

# Preliminaries

cmake_minimum_required (VERSION 2.8)

# Specify project and version

project (sqloxx)
set (version_major 1)
set (version_minor 0)
set (version_patch 0)

# Custom configuration options

option (
	BUILD_SHARED_LIBS
	"Build shared (rather than static) library (ON/OFF)?"
	OFF
)
option (
	ENABLE_ASSERTION_LOGGING
	"Enable logging of assertion failures (ON/OFF)?"
	ON
)
option (
	ENABLE_EXCEPTION_LOGGING
	"Enable logging of exceptions (ON/OFF)?"
	ON
)

# Definitions passed to the compiler

add_definitions (
	-DBOOST_FILESYSTEM_VERSION=3
	-DSQLOXX_VERSION_MAJOR=${version_major}
	-DSQLOXX_VERSION_MINOR=${version_minor}
	-DSQLOXX_VERSION_PATCH=${version_patch}
)
add_definitions (-DJEWEL_ENABLE_LOGGING)
if (ENABLE_ASSERTION_LOGGING)
	add_definitions (-DJEWEL_ENABLE_ASSERTION_LOGGING)
endif ()
if (ENABLE_EXCEPTION_LOGGING)
	add_definitions (-DJEWEL_ENABLE_EXCEPTION_LOGGING)
endif ()
if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif ()

# Dependencies

find_package (
	Boost 1.53.0
	COMPONENTS filesystem system
	REQUIRED
)
find_library (JEWEL_LIBRARY jewel REQUIRED)
find_library (UNIT_TEST_LIBRARY UnitTest++ REQUIRED)
find_package (Tclsh 8.4 REQUIRED)
if (WIN32)
	set (
		extra_libraries
		winmm
		shell32
		comctl32
		rpcrt4
		wsock32
		odbc32
		opengl32
	)
elseif (UNIX)
	set (
		extra_libraries
		pthread
		dl
	)
endif ()
if (JEWEL_LIBRARY-NOTFOUND)
	message ("Could not find Jewel library.")
endif ()
if (UNIT_TEST_LIBRARY-NOTFOUND)
	message ("Could not find UnitTest++ - cannot build tests.")
endif ()
if (NOT TCLSH_FOUND)
	message ("Could not find Tclsh - cannot run tests.")
endif ()

# Build instructions

if (
	Boost_FOUND AND
	NOT JEWEL_LIBRARY-NOTFOUND AND
	NOT UNIT_TEST_LIBRARY-NOTFOUND AND
	TCLSH_FOUND
)

	include_directories (
		include
		${Boost_INCLUDE_DIRS}
		${JEWEL_INCLUDES}
		${UNIT_TEST_INCLUDES}
	)

	set (
		libraries
		${Boost_LIBRARIES}
		${JEWEL_LIBRARY}
		${extra_libraries}
	)

	# Building the library

	set (
		library_sources
		src/database_connection.cpp
		src/database_transaction.cpp
		src/info.cpp
		src/sql_statement.cpp
		src/sqlite_dbconn.cpp
		src/sql_statement_impl.cpp
		src/sqlite3.c
	)
	set (library_name sqloxx)
	add_library (${library_name} ${library_sources})
	target_link_libraries (${library_name} ${libraries})

	# Building the tests

	set (
		test_sources
		tests/test.cpp
		tests/database_connection_tests.cpp
		tests/example.cpp
		tests/persistent_object_tests.cpp
		tests/sql_statement_tests.cpp
		tests/sqloxx_tests_common.cpp
		tests/atomicity_test.cpp
		tests/database_transaction_tests.cpp
		tests/handle_tests.cpp
		tests/identity_map_tests.cpp
		tests/next_auto_key_tests.cpp
		tests/table_iterator_tests.cpp
	)
	add_executable (test_engine ${test_sources})
	target_link_libraries (test_engine ${UNIT_TEST_LIBRARY} ${library_name} ${libraries})
	add_custom_target (
		test
		ALL
		COMMAND ${TCL_TCLSH} test_driver.tcl
		DEPENDS ${library_name} test_engine
	)

	# Installation instructions

	set (lib_installation_dir "${CMAKE_INSTALL_PREFIX}/lib")
	set (header_installation_dir "${CMAKE_INSTALL_PREFIX}/include/${library_name}")
	install (
		TARGETS ${library_name}
		ARCHIVE DESTINATION ${lib_installation_dir}
		LIBRARY DESTINATION ${lib_installation_dir}
	)
	install (
		FILES
			include/database_connection.hpp
			include/database_connection_fwd.hpp
			include/database_transaction.hpp
			include/handle.hpp
			include/handle_counter.hpp
			include/handle_fwd.hpp
			include/id.hpp
			include/identity_map.hpp
			include/identity_map_fwd.hpp
			include/info.hpp
			include/next_auto_key.hpp
			include/persistent_object.hpp
			include/persistent_object_fwd.hpp
			include/persistence_traits.hpp
			include/sql_statement.hpp
			include/sql_statement_fwd.hpp
			include/sqloxx_exceptions.hpp
			include/table_iterator.hpp
			include/table_iterator_fwd.hpp
		DESTINATION
			${header_installation_dir}
	)
	install (
		FILES
			include/detail/sql_statement_impl.hpp
			include/detail/sqlite_dbconn.hpp
			include/detail/sqlite3.h
			include/detail/sqlite3ext.h
		DESTINATION
			"${header_installation_dir}/detail"
	)
	
	# Building the tarball source package

	if (UNIX)
		set (
			package_name
			"${library_name}-${version_major}.${version_minor}.${version_patch}-working"
		)
		set (tarball_name "${package_name}.tar.gz")
		set (
			packaged_items
			include
			src
			tests
			CMakeLists.txt
			Doxyfile
			LICENSE
			NOTICE
			README
			overview.dox
			test_driver.tcl
		)
		add_custom_target (
			package
			COMMAND
				mkdir ${package_name} &&
				cp -r ${packaged_items} ${package_name} &&
				tar -sczf ${tarball_name} ${package_name} &&
				rm -rf ${package_name}
			DEPENDS ${library_name} test
		)
		set_directory_properties (
			PROPERTIES
				ADDITIONAL_MAKE_CLEAN_FILES "${tarball_name}"
		)
	endif ()

	# Building the documentation

	add_custom_target (
		docs
		COMMAND doxygen Doxyfile
	)

endif ()

