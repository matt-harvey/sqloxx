cmake_minimum_required (VERSION 2.6)

project (test)

include_directories (/home/matthew/lib)
link_directories (/home/matthew/lib/jewel)

set (library_files
	 database_connection.cpp
	 database_transaction.cpp
	 sql_statement.cpp
	 detail/sqlite_dbconn.cpp
	 detail/sql_statement_impl.cpp)

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

add_executable (test ${test_files})
add_library (sqloxx
			${library_files})
			
set (libraries
	 boost_filesystem
	 boost_system
	 jewel
	 unittest++
	 sqlite3)

target_link_libraries (test ${libraries})

install(TARGETS
	    sqloxx
		DESTINATION
		/home/matthew/lib/sqloxx)

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
		/home/matthew/lib/sqloxx)
install(DIRECTORY
		detail
		DESTINATION
		/home/matthew/lib/sqloxx)
		
					


