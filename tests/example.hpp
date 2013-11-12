/*
 * Copyright 2013 Matthew Harvey
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GUARD_example_hpp_7678596874549332
#define GUARD_example_hpp_7678596874549332

// Hide from Doxygen
/// @cond

#include "database_connection.hpp"
#include "identity_map.hpp"
#include "persistent_object.hpp"
#include "sqloxx_tests_common.hpp"
#include <string>

namespace sqloxx
{
namespace tests
{


// Dummy classes inheriting from PersistentObject, for the purpose
// of testing.

class ExampleA: public PersistentObject<ExampleA, DerivedDatabaseConnection>
{
public:
	typedef PersistentObject<ExampleA, DerivedDatabaseConnection>
		DPersistentObject;

	static void setup_tables(DatabaseConnection& dbc);

	ExampleA
	(	IdentityMap& p_identity_map,
		IdentityMap::Signature const& p_sig
	);

	ExampleA
	(	IdentityMap& p_identity_map,
		Id p_id,
		IdentityMap::Signature const& p_sig
	);

	// Move constructor deliberately undefined
	
	ExampleA& operator=(ExampleA const&) = delete;
	ExampleA& operator=(ExampleA&&) = delete;

	~ExampleA() = default;

	int x();
	double y();
	void set_x(int p_x);
	void set_y(double p_y);
	
	// To test protected functions of PersistentObject. Returns number
	// of failing checks in test.
	static int self_test();	

	static std::string exclusive_table_name();
	static std::string primary_key_name();

private:
	void do_load() override;
	// Uses default version of do_calculate_prospective_key
	void do_save_existing() override;
	void do_save_new() override;
	// Uses default version of do_ghostify
	int m_x;
	double m_y;
};


class ExampleB: public PersistentObject<ExampleB, DerivedDatabaseConnection>
{
public:
	typedef PersistentObject<ExampleB, DerivedDatabaseConnection>
		DPersistentObject;
	
	static void setup_tables(DatabaseConnection& dbc);

	ExampleB
	(	IdentityMap& p_identity_map,
		IdentityMap::Signature const& p_sig
	);
	
	ExampleB
	(	IdentityMap& p_identity_map,
		Id p_id,
		IdentityMap::Signature const& p_sig
	);

	ExampleB& operator=(ExampleB const&) = delete;
	ExampleB& operator=(ExampleB&&) = delete;

	std::string s();
	void set_s(std::string const& p_s);

	virtual ~ExampleB();
	
	static std::string exclusive_table_name();
	static std::string primary_key_name();

protected:
	void load_core();
	void save_existing_core();
	Id save_new_core();

private:
	std::string m_s;
};


// begin forward declarations

class ExampleC;

// end forward declarations

}  // namespace tests


// Specialize PersistenceTraits for ExampleC and ExampleD. This needs to
// be done in the sqloxx namespace.
template <> struct PersistenceTraits<tests::ExampleC>
{
	typedef tests::ExampleB Base;
};


namespace tests
{

class ExampleC: public ExampleB
{
public:
	
	static void setup_tables(DatabaseConnection& dbc);

	ExampleC
	(	IdentityMap& p_identity_map,
		IdentityMap::Signature const& p_sig
	);
	
	ExampleC
	(	IdentityMap& p_identity_map,
		Id p_id,
		IdentityMap::Signature const& p_sig
	);

	ExampleC& operator=(ExampleC const&) = delete;
	ExampleC& operator=(ExampleC&&) = delete;
	~ExampleC() = default;

	int p();
	int q();
	void set_p(int p_p);
	void set_q(int p_q);

	static std::string exclusive_table_name();

private:
	void do_load() override;
	void do_save_existing() override;
	void do_save_new() override;
	void do_remove() override;
	int m_p;
	int m_q;
};


// Dummy class derived from DatabaseConnection, for testing
// purposes

class DerivedDatabaseConnection: public DatabaseConnection
{
public:

	DerivedDatabaseConnection();

	template <typename T>
	IdentityMap<T>& identity_map();

private:

	IdentityMap<ExampleA> m_example_a_map;
	IdentityMap<ExampleB> m_example_b_map;
};


template <>
inline
IdentityMap<ExampleA>&
DerivedDatabaseConnection::identity_map<ExampleA>()
{
	return m_example_a_map;
}

template <>
inline
IdentityMap<ExampleB>&
DerivedDatabaseConnection::identity_map<ExampleB>()
{
	return m_example_b_map;
}


}  // namespace tests
}  // namespace sqloxx

/// @endcond
// End hiding from Doxygen

#endif  // GUARD_example_hpp_7678596874549332
