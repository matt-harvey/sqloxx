#ifndef GUARD_persistence_traits_hpp_5478759168404508
#define GUARD_persistence_traits_hpp_5478759168404508

namespace sqloxx
{

/**
 * Houses traits relevant for PersistentObject. Where you have
 * a particular instantiation of PersistentObject<T, Connection>,
 * specialize this template if you want non-default behaviour.
 */
template <typename T>
struct PersistenceTraits
{
	/**
	 * The PrimaryT is the type such that its primary key is
	 * \e ultimately stored in this table. Typically
	 * where we have a Base and a Derived class, both of
	 * which are PersistentObject instantiations, the Base
	 * class will be the PrimaryT of Derived. The primary
	 * key of Derived will be in a column in the "Base table"
	 * in the database. Then Derived may have its own table with
	 * a column that references the primary key column in the
	 * Base table.
	 *
	 * @todo Improve this explanation.
	 */
	typedef T PrimaryT;

};  // class PersistenceTraits

}  // namespace sqloxx

#endif  // GUARD_persistence_traits_hpp_5478759168404508
