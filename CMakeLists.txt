cmake_minimum_required (VERSION 2.6)

project (sqloxx)

if (MSYS)
	include_directories(C:/MSYS/1.0/local/include)
	link_directories(C:/MSYS/1.0/local/lib)
else ()
	link_directories(/usr/local/lib)
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
	 tests/reader_tests.cpp)

add_executable (sqloxx_test ${test_files})
add_library (sqloxx ${library_files})
			
set (libraries
	 boost_filesystem
	 boost_system
	 jewel
	 unittest++)

target_link_libraries (sqloxx ${libaries})
target_link_libraries (sqloxx_test ${libraries})

if (MSYS)
	set (lib_installation_dir C:/MSYS/1.0/local/lib)
else ()
	set (lib_installation_dir /usr/local/lib)
endif ()


install(TARGETS sqloxx ARCHIVE DESTINATION ${lib_installation_dir})


if (MSYS)
	set (outer_header_installation_dir C:/MSYS/1.0/local/include)
else ()
	set (outer_header_installation_dir /usr/local/include)
endif ()

set (inner_header_installation_dir "${outer_header_installation_dir}/sqloxx")

install (DIRECTORY sqloxx
         DESTINATION ${outer_header_installation_dir})

install(FILES
		database_connection.hpp
		database_transaction.hpp
		general_typedefs.hpp
		handle.hpp
		identity_map.hpp
		next_auto_key.hpp
		persistent_object.hpp
		reader.hpp
		sql_statement.hpp
		sqloxx_exceptions.hpp
		DESTINATION
		${inner_header_installation_dir})

install(FILES
		detail/sql_statement_impl.hpp
		detail/sqlite_dbconn.hpp
		detail/sqlite3.h
		detail/sqlite3ext.h
		DESTINATION
		"${inner_header_installation_dir}/detail")
		
					


