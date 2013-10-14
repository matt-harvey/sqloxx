cmake_minimum_required (VERSION 2.6)

project (sqloxx)

add_definitions(-DJEWEL_ENABLE_LOGGING
		        -DJEWEL_ENABLE_ASSERTION_LOGGING
				-DJEWEL_ENABLE_EXCEPTION_LOGGING
				-DBOOST_FILESYSTEM_VERSION=3)

if (WIN32)
	include_directories(include
	                C:/Dev-Cpp
					C:/include
					C:/Dev-Cpp/include/common
					include)
	link_directories(C:/Dev-Cpp/lib
					C:/Dev-Cpp/MinGW/lib
					C:/lib)
	set (libraries
		boost_filesystem-mgw46-mt-1_53
		boost_system-mgw46-mt-1_53
		jewel
		 winmm
		 shell32
		 comctl32
		 rpcrt4
		 wsock32
		 odbc32
		 opengl32)
else ()
	include_directories (include)
	link_directories(/usr/local/lib
	                 /usr/lib
					 /usr/local/lib64
					 /usr/lib64)
	set (libraries
		 boost_filesystem
		 c
		 stdc++
		 boost_system
		 dl
		 jewel
		 m
		 pthread)
endif ()

set (library_sources
	 src/database_connection.cpp
	 src/database_transaction.cpp
	 src/sql_statement.cpp
	 src/sqlite_dbconn.cpp
	 src/sql_statement_impl.cpp
	 src/sqlite3.c)

set (test_sources
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
	 tests/table_iterator_tests.cpp)

add_executable (test ${test_sources})
add_library (sqloxx ${library_sources})

target_link_libraries (sqloxx ${libaries})
target_link_libraries (test UnitTest++ sqloxx ${libraries})

set (lib_installation_dir ${CMAKE_INSTALL_PREFIX}/lib)

install(TARGETS sqloxx ARCHIVE DESTINATION ${lib_installation_dir})

set (header_installation_dir "${CMAKE_INSTALL_PREFIX}/include/sqloxx")

install(FILES
		include/database_connection.hpp
		include/database_connection_fwd.hpp
		include/database_transaction.hpp
		include/general_typedefs.hpp
		include/handle.hpp
		include/handle_fwd.hpp
		include/identity_map.hpp
		include/identity_map_fwd.hpp
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
		${header_installation_dir})

install(FILES
		include/detail/sql_statement_impl.hpp
		include/detail/sqlite_dbconn.hpp
		include/detail/sqlite3.h
		include/detail/sqlite3ext.h
		DESTINATION
		"${header_installation_dir}/detail")
		
					


