// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "sqloxx_tests_common.hpp"
#include "derived_po.hpp"
#include "../database_connection.hpp"
#include "../detail/sql_statement_impl.hpp"
#include "../detail/sqlite_dbconn.hpp"
#include "../sql_statement.hpp"
#include <boost/filesystem.hpp>
#include <jewel/on_windows.hpp>
#include <jewel/stopwatch.hpp>


#if JEWEL_ON_WINDOWS
#	include <windows.h>  // for Sleep
#endif

#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

using jewel::Stopwatch;
using std::cout;
using std::cerr;
using std::endl;
using std::remove;
using std::string;
using std::terminate;
using std::vector;
using sqloxx::detail::SQLStatementImpl;
using sqloxx::detail::SQLiteDBConn;

namespace filesystem = boost::filesystem;


namespace
{
	void windows_friendly_remove(string const& fp)
	{
#		if JEWEL_ON_WINDOWS
			int const max_tries = 10000;
			int const delay = 100;
			char const* filename = fp.c_str();
			int i;
			for
			(	i = 0;
				(remove(filename) != 0) && (i != max_tries);
				++i
			)
			{
					Sleep(delay);
			}
			if (i == max_tries)
			{
				assert (filesystem::exists(fp));
				cout << "Test file could not be removed. Terminating tests."
					 << endl;
				terminate();
			}
#		else
			filesystem::remove(fp);
#		endif
		assert (!filesystem::exists(fp));
		return;
	}

}   // end anonymous namespace


namespace sqloxx
{
namespace tests
{


bool file_exists(filesystem::path const& filepath)
{
	return filesystem::exists
	(	filesystem::status(filepath)
	);
}


void abort_if_exists(filesystem::path const& filepath)
{
	if (file_exists(filepath))
	{
		cerr << "File named \"" << filepath.string() << "\" already "
			 << "exists. Test terminated." << endl;
		terminate();
	}
	return;
}

void
do_speed_test()
{
	string const filename("aaksjh237nsal");
	int const loops = 50000;
	
	vector<string> statements;
	statements.push_back
	(	"insert into dummy(colA, colB) values(3, 'hi')"
	);
	statements.push_back
	(	"select colA, colB from dummy where colB = "
		" 'asfkjasdlfkasdfasdf' and colB = '-90982097';"
	);
	statements.push_back
	(	"insert into dummy(colA, colB) values(198712319, 'aasdfhasdkjhash');"
	);
	statements.push_back
	(	"select colA, colB from dummy where colA = "
		" 'noasdsjhasdfkjhasdkfjh' and colB = '-9987293879';"
	);

	vector<string>::size_type const num_statements = statements.size();

	string const table_creation_string
	(	"create table dummy(colA int not null, colB text)"
	);
	

	// With SQLStatement
	DatabaseConnection db;
	db.open(filename);
	db.execute_sql(table_creation_string);

	cout << "Timing with SQLStatement." << endl;
	db.execute_sql("begin");
	Stopwatch sw1;
	for (int i = 0; i != loops; ++i)
	{
		SQLStatement s(db, statements[i % num_statements]);
		// s.step_final();
	}
	sw1.log();
	db.execute_sql("end");
	windows_friendly_remove(filename);


	// With SQLStatementImpl
	SQLiteDBConn sdbc;
	sdbc.open(filename);
	sdbc.execute_sql(table_creation_string);

	cout << "Timing with SQLStatementImpl." << endl;
	sdbc.execute_sql("begin");
	Stopwatch sw0;
	for (int i = 0; i != loops; ++i)
	{
		SQLStatementImpl s(sdbc, statements[i % num_statements]);
		// s.step_final();
	}
	sw0.log();
	sdbc.execute_sql("end");
	windows_friendly_remove(filename);
	
	return;
}

	

DatabaseConnectionFixture::DatabaseConnectionFixture():
	db_filepath("Testfile_01"),
	pdbc(0)
{
	pdbc = new DatabaseConnection;
	abort_if_exists(db_filepath);
	pdbc->open(db_filepath);
	assert (pdbc->is_valid());
}


DatabaseConnectionFixture::~DatabaseConnectionFixture()
{
	assert (pdbc->is_valid());
	delete pdbc;
	windows_friendly_remove(db_filepath.string());
	assert (!file_exists(db_filepath));
}

DerivedPOFixture::DerivedPOFixture():
	db_filepath("Testfile_dpof"),
	pdbc(0)
{
	pdbc = new DerivedDatabaseConnection;
	abort_if_exists(db_filepath);
	pdbc->open(db_filepath);
	assert (pdbc->is_valid());
	DerivedPO::setup_tables(*pdbc);
}

DerivedPOFixture::~DerivedPOFixture()
{
	assert (pdbc->is_valid());
	delete pdbc;
	windows_friendly_remove(db_filepath.string());
	assert (!file_exists(db_filepath));
}

}  // namespace sqloxx
}  // namespace tests
