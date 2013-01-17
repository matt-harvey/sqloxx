cmake_minimum_required (VERSION 2.6)

project (sqloxx)

link_directories(/usr/local/lib)

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

add_executable (sqloxx_test ${test_files})
add_library (sqloxx MODULE ${library_files})
			
set (libraries
	 boost_filesystem
	 boost_system
	 jewel
	 unittest++
	 sqlite3)

target_link_libraries (sqloxx ${libaries})
target_link_libraries (sqloxx_test ${libraries})

install(TARGETS sqloxx
		LIBRARY DESTINATION lib)

install (DIRECTORY sqloxx DESTINATION include)
install (DIRECTORY sqloxx/detail DESTINATION include/sqloxx)

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
		include/sqloxx)

install(FILES
		detail/sql_statement_impl.hpp
		detail/sqlite_dbconn.hpp
		DESTINATION
		include/sqloxx/detail)
		
					


