/*
 * This file is part of the Sqloxx project and is distributed under the
 * terms of the license contained in the file LICENSE.txt distributed
 * with this package.
 * 
 * Author: Matthew Harvey <matthew@matthewharvey.net>
 *
 * Copyright (c) 2012-2013, Matthew Harvey.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

	ExampleA& operator=(ExampleA const&) = delete;
	ExampleA& operator=(ExampleA&&) = delete;

	int x();
	double y();
	void set_x(int p_x);
	void set_y(double p_y);
	// Default destructor is OK.
	
	// To test protected functions of PersistentObject. Returns number
	// of failing checks in test.
	static int self_test();	

	static std::string exclusive_table_name();
	static std::string primary_key_name();

protected:
	ExampleA(ExampleA const& rhs);

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
	ExampleB(ExampleB const& rhs);

private:
	void do_load() override;
	// Uses default version of do_calculate_prospective_key
	void do_save_existing() override;
	void do_save_new() override;
	// Uses default version of do_ghostify
	std::string m_s;
};


// begin forward declarations

class ExampleC;
class ExampleD;

// end forward declarations

}  // namespace tests


// Specialize PersistenceTraits for ExampleC and ExampleD. This needs to
// be done in the sqloxx namespace.
template <> struct PersistenceTraits<tests::ExampleC>
{
	typedef tests::ExampleB Base;
};

template <> struct PersistenceTraits<tests::ExampleD>
{
	typedef tests::ExampleB Base;
};


namespace tests
{

// Dummy class derived from DatabaseConnection, for testing
// purposes

class DerivedDatabaseConnection: public DatabaseConnection
{
public:

	DerivedDatabaseConnection();

	typedef sqloxx::IdentityMap<ExampleA> IdentityMap;

	template <typename T>
	IdentityMap& identity_map();

private:

	IdentityMap m_example_a_map;
};


template <>
inline
IdentityMap<ExampleA>&
DerivedDatabaseConnection::identity_map<ExampleA>()
{
	return m_example_a_map;
}


}  // namespace tests
}  // namespace sqloxx

/// @endcond
// End hiding from Doxygen

#endif  // GUARD_example_hpp_7678596874549332
