cmake_minimum_required (VERSION 2.6)

project (sqloxx)

add_definitions(-DJEWEL_ENABLE_LOGGING
		        -DJEWEL_ENABLE_ASSERTION_LOGGING
				-DJEWEL_ENABLE_EXCEPTION_LOGGING)

if (WIN32)
	include_directories(C:/Dev-Cpp
					C:/include
					C:/Dev-Cpp/include/common)
	link_directories(C:/Dev-Cpp/lib
					C:/Dev-Cpp/MinGW/lib
					C:/lib)
else ()
	link_directories(/usr/local/lib
	                 /usr/lib
					 /usr/local/lib64
					 /usr/lib64)
endif ()

set (library_files
	 database_connection.cpp
	 database_transaction.cpp
	 sql_statement.cpp
	 detail/sqlite_dbconn.cpp
	 detail/sql_statement_impl.cpp
	 detail/sqlite3.c)

set (test_files
	${library_files}
     test.cpp
     tests/database_connection_tests.cpp
	 tests/derived_po.cpp
	 tests/persistent_object_tests.cpp
	 tests/sql_statement_tests.cpp
	 tests/sqloxx_tests_common.cpp
	 tests/atomicity_test.cpp
	 tests/database_transaction_tests.cpp
	 tests/handle_tests.cpp
	 tests/identity_map_tests.cpp
	 tests/next_auto_key_tests.cpp
	 tests/table_iterator_tests.cpp)

add_executable (sqloxx_test ${test_files})
add_library (sqloxx ${library_files})
			
if (WIN32)
	set (libraries
		boost_filesystem-mgw46-mt-1_53
		boost_system-mgw46-mt-1_53
		jewel
		UnitTest++
		 winmm
		 shell32
		 comctl32
		 rpcrt4
		 wsock32
		 odbc32
		 opengl32)
else ()
	set (libraries
		 boost_filesystem
		 c
		 stdc++
		 boost_system
		 dl
		 jewel
		 m
		 pthread
		 UnitTest++)
endif()

target_link_libraries (sqloxx ${libaries})
target_link_libraries (sqloxx_test ${libraries})

if (WIN32)
	set (lib_installation_dir C:/lib)
else ()
	set (lib_installation_dir /usr/local/lib)
endif ()


install(TARGETS sqloxx ARCHIVE DESTINATION ${lib_installation_dir})


if (WIN32)
	set (outer_header_installation_dir C:/include)
else ()
	set (outer_header_installation_dir /usr/local/include)
endif ()

set (inner_header_installation_dir "${outer_header_installation_dir}/sqloxx")

install (DIRECTORY sqloxx
         DESTINATION ${outer_header_installation_dir})

install(FILES
		database_connection.hpp
		database_connection_fwd.hpp
		database_transaction.hpp
		general_typedefs.hpp
		handle.hpp
		handle_fwd.hpp
		identity_map.hpp
		identity_map_fwd.hpp
		next_auto_key.hpp
		persistent_object.hpp
		persistent_object_fwd.hpp
		persistence_traits.hpp
		sql_statement.hpp
		sql_statement_fwd.hpp
		sqloxx_exceptions.hpp
		table_iterator.hpp
		table_iterator_fwd.hpp
		DESTINATION
		${inner_header_installation_dir})

install(FILES
		detail/sql_statement_impl.hpp
		detail/sqlite_dbconn.hpp
		detail/sqlite3.h
		detail/sqlite3ext.h
		DESTINATION
		"${inner_header_installation_dir}/detail")
		
					


