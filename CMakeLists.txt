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

cmake_minimum_required (VERSION 2.6)

project (sqloxx)

set (version_major 1)
set (version_minor 0)
set (version_patch 0)

find_package (
	Boost 1.53.0
	COMPONENTS filesystem system
)

if (Boost_FOUND)

	add_definitions (
		-DJEWEL_ENABLE_LOGGING
		-DJEWEL_ENABLE_ASSERTION_LOGGING
		-DJEWEL_ENABLE_EXCEPTION_LOGGING
		-DBOOST_FILESYSTEM_VERSION=3
		-DSQLOXX_VERSION_MAJOR=${version_major}
		-DSQLOXX_VERSION_MINOR=${version_minor}
		-DSQLOXX_VERSION_PATCH=${version_patch}
	)

	include_directories (include ${Boost_INCLUDE_DIRS})

	if (WIN32)

		set (
			libraries
			${Boost_LIBRARIES}
			jewel
			winmm
			shell32
			comctl32
			rpcrt4
			wsock32
			odbc32
			opengl32
		)

	else ()

		set (
			libraries
			${Boost_LIBRARIES}
			c
			stdc++
			dl
			jewel
			m
			pthread
		)

	endif ()

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

	add_executable (test ${test_sources})
	add_library (sqloxx ${library_sources})

	target_link_libraries (sqloxx ${libaries})
	target_link_libraries (test UnitTest++ sqloxx ${libraries})

	set (lib_installation_dir ${CMAKE_INSTALL_PREFIX}/lib)

	install(TARGETS sqloxx ARCHIVE DESTINATION ${lib_installation_dir})

	set (header_installation_dir "${CMAKE_INSTALL_PREFIX}/include/sqloxx")

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
			
endif ()
