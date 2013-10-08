// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_derived_po_hpp_7678596874549332
#define GUARD_derived_po_hpp_7678596874549332

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


// Dummy class inheriting from PersistentObject, for the purpose
// of testing PersistentObject class.
class DerivedPO: public PersistentObject<DerivedPO, DerivedDatabaseConnection>
{

public:
	typedef sqloxx::Id Id;
	typedef PersistentObject<DerivedPO, DerivedDatabaseConnection>
		DPersistentObject;
	static void setup_tables(DatabaseConnection& dbc);
	DerivedPO(IdentityMap& p_identity_map, IdentityMap::Signature const& p_sig);
	DerivedPO(IdentityMap& p_identity_map, Id p_id, IdentityMap::Signature const& p_sig);
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
	DerivedPO(DerivedPO const& rhs);

private:
	void do_load();
	// Uses default version of do_calculate_prospective_key
	void do_save_existing();
	void do_save_new();
	void do_ghostify();
	int m_x;
	double m_y;
};


// Dummy class derived from DatabaseConnection, to provide
// IdentityMap<DerivedPO>
class DerivedDatabaseConnection: public DatabaseConnection
{
public:

	DerivedDatabaseConnection();

	typedef sqloxx::IdentityMap<DerivedPO> IdentityMap;

	template <typename T>
	IdentityMap& identity_map();

private:

	IdentityMap m_derived_po_map;
};


template <>
inline
IdentityMap<DerivedPO>&
DerivedDatabaseConnection::identity_map<DerivedPO>()
{
	return m_derived_po_map;
}


}  // namespace tests
}  // namespace sqloxx

/// @endcond
// End hiding from Doxygen

#endif  // GUARD_derived_po_hpp_7678596874549332
