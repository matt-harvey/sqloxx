// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_identity_map_hpp_41089441925794556
#define GUARD_identity_map_hpp_41089441925794556

#include "general_typedefs.hpp"
#include "handle.hpp"
#include "persistence_traits.hpp"
#include "persistent_object_fwd.hpp"
#include "sqloxx_exceptions.hpp"
#include <boost/numeric/conversion/cast.hpp>
#include <jewel/assert.hpp>
#include <jewel/exception.hpp>
#include <jewel/log.hpp>
#include <jewel/signature.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace sqloxx
{

/**
 * Provides an in-memory cache for objects of type T, where such
 * objects are persisted to a database via a database connection of
 * type Connection. T and Connection are passed as template parameters
 * to the class template. It is expected that T is a subclass of
 * sqloxx::PersistentObject<T, Connection>, and Connection is a
 * subclass of sqloxx::DatabaseConnection.
 *
 * T should define
 * constructors of the form:\n
 * T(IdentityMap&, IdentityMap::Signature const&); and\n
 * T(IdentityMap&, Id, IdentityMap::Signature const&)\n
 * These should then pass their parameters to the corresponding
 * constructors of PersistentObject<T, Connection>; except that the
 * final "Signature" parameter is not passed on.
 * The purpose of the Signature parameter is to prevent
 * the T constructor from being called by any class other than
 * IdentityMap.
 *
 * Each instance of IdentityMap has a particular Connection associated
 * with it. The IdentityMap caches objects loaded from the database,
 * and provides the Handle class
 * with pointers to these objects. By using IdentityMap to cache objects,
 * application code can be sure that each single record of type T
 * that is stored in the database, has at most a single in-memory
 * object of type T associated with that record, loaded in memory
 * at any one time. IdentityMap thus implements the "Identity Map"
 * pattern detailed in Martin Fowler's book, "Patterns of Enterprise
 * Application Architecture". By having at most a single in-memory
 * object per in-database record, we guard against the possibility
 * of the same object being edited inconsistently in different
 * locations. By keeping objects in a cache, we speed execution of
 * read and write operations, by avoiding a trip to the disk when an
 * object has already been loaded.
 *
 * IdentityMap is intended to work in conjunction with sqloxx::Handle
 * and sqloxx::PersistentObject<T, Connection>. See also the documentation
 */
template <typename T, typename Connection>
class IdentityMap
{
public:

	typedef sqloxx::Id CacheKey;

	typedef jewel::Signature<IdentityMap> Signature;

	/**
	 * Construct an IdentityMap associated with the database
	 * connection Connection. Connection should be a subclass
	 * of sqloxx::DatabaseConnection.

	 * @throws std::bad_alloc in the case of memory allocation
	 * failure. (This is very unlikely.)
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	IdentityMap(Connection& p_connection);

	IdentityMap(IdentityMap const&) = delete;
	IdentityMap(IdentityMap&&) = delete;
	IdentityMap& operator=(IdentityMap const&) = delete;
	IdentityMap& operator=(IdentityMap&&) = delete;

	/**
	 * Destructor. The underlying cache is automatically emptied
	 * of all objects (i.e. instances of T) on destruction of
	 * the IdentityMap, by the calling the destructor of each
	 * object in the cache. The cache is then itself destructed.
	 * However the database connection (Connection instance) referenced
	 * by the IdentityMap is \e not destructed merely by virtue
	 * of the destruction of the IdentityMap.
	 *
	 * Exception safety: the <em>nothrow guarantee</em> is provided,
	 * providing the destructor of T does not throw.
	 *
	 * @todo Testing.
	 */
	~IdentityMap() = default;

	/**
	 * Turn on caching. When caching is on, objects loaded from the
	 * database are cached indefinitely in the IdentityMap. When
	 * caching is off, each object is only cached as long as there
	 * as at least one Handle referring to it. If enable_caching() is
	 * called when caching is already on, it has no effect.
	 *
	 * Caching is off by default.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 */
	void enable_caching();

	/**
	 * Turn caching off. When caching is off, each object is only
	 * cached as long as there is at least one Handle referring to
	 * it. If disable_caching() is called when caching is already
	 * off, it has no effect. If caching is on when disable_caching()
	 * is called, then the function will remove from the cache any
	 * object that currently has no Handle instances
	 * pointing to it.
	 *
	 * Caching is off by default.
	 * 
	 * Exception safety: <em>nothrow guarantee</em>, providing the
	 * destructor of T does not throw.
	 */
	void disable_caching();

	/**
	 * @returns a reference to the database connection associated with
	 * this IdentityMap.
	 * 
	 * Exception safety: <em>nothrow guarantee</em>.
	 *
	 * @todo Is this function necessary?
	 */
	Connection& connection();

	/**
	 * Control access to the provide_pointer functions, deliberately
	 * limiting this access to the Handle class.
	 */
	template <typename DynamicT>
	class HandleAttorney
	{
	public:
		friend class Handle<T>;
		friend class Handle<DynamicT>;
	private:
		static DynamicT* get_pointer(IdentityMap& p_identity_map)
		{
			return p_identity_map.provide_pointer<DynamicT>();
		}
		static DynamicT* get_pointer(IdentityMap& p_identity_map, Id p_id)
		{
			return p_identity_map.provide_pointer<DynamicT>(p_id);
		}
		static DynamicT* unchecked_get_pointer
		(	IdentityMap& p_identity_map, Id p_id
		)
		{
			return p_identity_map.unchecked_provide_pointer<DynamicT>(p_id);
		}
	};

	template <typename DynamicT> friend class HandleAttorney;

	/**
	 * Control access to the various functions of the class
	 * IdentityMap<T, Connection>, deliberately
	 * limiting this access to the class PersistentObject<T, Connection>.
	 */
	class PersistentObjectAttorney
	{
	public:
		friend class PersistentObject<T, Connection>;
	private:
		static void register_id
		(	IdentityMap& p_identity_map,
			CacheKey p_cache_key,
			Id p_id
		)
		{
			p_identity_map.register_id(p_cache_key, p_id);
			return;
		}
		static void deregister_id
		(	IdentityMap& p_identity_map,
			Id p_id
		)
		{
			p_identity_map.deregister_id(p_id);
			return;
		}
		static void notify_nil_handles
		(	IdentityMap& p_identity_map,
			CacheKey p_cache_key
		)
		{
			p_identity_map.notify_nil_handles(p_cache_key);
			return;
		}
		static void uncache_object
		(	IdentityMap& p_identity_map,	
			CacheKey p_cache_key
		)
		{
			p_identity_map.uncache_object(p_cache_key);
			return;
		}
		static void partially_uncache_object
		(	IdentityMap& p_identity_map,
			CacheKey p_cache_key
		)
		{
			p_identity_map.partially_uncache_object(p_cache_key);
			return;
		}
	};

	friend class PersistentObjectAttorney;

private:

	/**
	 * Provide pointer to object of type static type T, and dynamic type
	 * DynamicT, representing a newly created
	 * object that has not yet been persisted to the database.
	 * Typically DynamicT will be the same type as T, but it need
	 * not be - it might be a class derived from T.
	 *
	 * DynamicT must also be such that PersistenceTraits<T>::PrimaryT is the
	 * same as T. DynamicT must also be, or be derived, from T.
	 * If derived from T, then T must
	 * be a polymorphic base class. If these conditions fail, compilation
	 * will fail.
	 *
	 * @returns a T* pointing to a newly constructed instance of DynamicT,
	 * that is cached in this instance of IdentityMap<T, Connection>.
	 *
	 * @throws sqloxx::OverflowException in the extremely unlikely
	 * event that the in-memory cache already has so many objects loaded that
	 * an additional object could not be cached without causing
	 * arithmetic overflow in the process of assigning it a key.
	 *
	 * @throws std::bad_alloc in the unlikely event of memory allocation
	 * failure during the creating and caching of the instance of T.
	 *
	 * <em>In addition</em>, any exceptions thrown from the DynamicT
	 * constructor may also
	 * be thrown from provide_pointer().
	 *
	 * Exception safety depends on the constructor of DerivedT of the form
	 * DynamicT(IdentityMap&, IdentityMap::Signature const&).
	 * Provided this constructor offers at
	 * least the <em>strong guarantee</em>, then provide_pointer() offers the
	 * <em>strong guarantee</em> (although there may be some internal cache
	 * state that is not rolled back but which does not affect client code).
	 */
	template <typename DynamicT>
	DynamicT* provide_pointer();

	/**
	 * Provide pointer to object of static type T, and dynamic type DynamicT,
	 * representing an object
	 * already stored in the database, with primary key (id) p_id.
	 *
	 * DynamicT must also be such that PersistenceTraits<T>::PrimaryT is the
	 * same as T. DynamicT must also be, or be derived, from T.
	 * If derived from T, then T must
	 * be a polymorphic base class. If these conditions fail, compilation
	 * will fail.
	 *
	 * @returns a pointer<T> pointing to an instance of DynamicT corresponding
	 * to a record of the corresponding type already persisted in the
	 * database, with p_id as its primary key.
	 *
	 * @throws sqloxx::BadIdentifier if there is not record in the
	 * database of type T that has p_id as its primary key. Note the
	 * validity of p_id is always checked in the physical database
	 * by this function, regardless of whether object yet has
	 * yet be cached in the IdentityMap. (It is possibly in certain
	 * situations for objects to be left in the cache with ids when they
	 * no longer exist in the database. This doesn't do any harm as long
	 * as we don't subsequently use Handles with these invalid
	 * ids.) For a faster, unchecked
	 * version of this function, see unchecked_provide_pointer(Id p_id).
	 *
	 * @throws std::bad_alloc if the object is not already loaded in the
	 * cache, and there is a memory allocation failure in the process of
	 * loading and caching the object.
	 *
	 * @throws sqloxx::OverflowException in the extremely unlikely
	 * event that the in-memory cache already has so many objects loaded that
	 * an additional object could not be cached without causing
	 * arithmetic overflow in the process of assigning it a key.
	 *
	 * @throws InvalidConnection in case the database connection is invalid.
	 *
	 * @throws SQLiteException, or a derivative thereof, in the extremely
	 * unlikely event of an error during execution thrown up by the underlying
	 * SQLite API.
	 *
	 * <em>In addition</em>, any exceptions thrown from the T constructor
	 * may also be thrown from provide_pointer().
	 *
	 * Exception safety depends on the constructor of DynamicT of the form
	 * DynamicT(IdentityMap&, Id, IdentityMap::Signature const&).
	 * Provided this constructor offers at least the
	 * <em>strong guarantee</em>, then provide_pointer() offers the
	 * <em>strong guarantee</em> (although there may be some internal cache
	 * state that is not rolled back but which does not affect client code).
	 * For this guarantee to hold, it is also required that the destructor
	 * of T not throw.
	 *
	 * @todo Revise tests to reflect checked nature. Test
	 * unchecked_provide_pointer separately as well.
	 *
	 * @todo HIGH PRIORITY Does this check for existence in the exclusive_table
	 * of \e DynamicT? It should.
	 */
	template <typename DynamicT>
	DynamicT* provide_pointer(Id p_id);

	/**
	 * Behaviour is exactly the same as provide_pointer(Id p_id), with the
	 * sole difference that (a) the unchecked version is faster, and
	 * (b) if a record of type DynamicT, with p_id as its primary key,
	 * does not exist in the database, then, rather than an exception
	 * being thrown, behaviour is undefined. This function should \e never be
	 * called unless you are \e sure p_id is an existing primary key.
	 *
	 * DynamicT must also be such that PersistenceTraits<T>::PrimaryT is the
	 * same as T. DynamicT must also be, or be derived, from T.
	 * If derived from T, then T must
	 * be a polymorphic base class. If these conditions fail, compilation
	 * will fail.
	 */
	template <typename DynamicT>
	DynamicT* unchecked_provide_pointer(Id p_id);

	/**
	 * Register id of newly saved instance of T. This function is
	 * intended only to be called from PersistentObject<T, Connection>.
	 * This tells the cache the id of the object so that in future, it
	 * can be looked up by its id as well as by its cache key.
	 *
	 * Precondition: there must be an object cached in the IdentityMap
	 * with this p_cache_key;\and
	 * The destructor of T must not throw.
	 *
	 * @param p_cache_key the cache key of the newly saved object
	 *
	 * @param p_id the id of the newly saved object, which corresponds
	 * to its primary key in the database
	 *
	 * @throws std::bad_alloc in the very unlikely event of memory allocation
	 * failure while registering the object's id in the cache.
	 *
	 * Exception safety: <em>strong guarantee</em>, providing the precondition
	 * is met.
	 */
	void register_id(CacheKey p_cache_key, Id p_id);

	/**
	 * Notify IdentityMap that an instance of T that previously had
	 * an id p_id, no longer has any id. This function is
	 * intended only to be called from PersistentObject<T, Connection>.
	 *
	 * @param p_id the id of the instance of T, corresponding to its
	 * primary key in the database.
	 *
	 * Preconditions:\n
	 * It must be known of the instance of T in
	 * question, that it is cached in the IdentityMap under p_id
	 * (not as its cache key, but as its id); and\n
	 * The destructor of T must be non-throwing.
	 *
	 * Exception safety: <em>nothrow guarantee</em>, providing the
	 * preconditions are met.
	 */
	void deregister_id(Id p_id);

	/**
	 * This should only be called from PersistentObject<T, Connection>.
	 *
	 * Notify the IdentityMap that there are no handles left that are
	 * pointing to the object with p_cache_key.
	 * 
	 * Preconditions: (a) there must be an object cached in this
	 * IdentityMap with this cache_key; and (b) the destructor of T must
	 * never throw.
	 * 
	 * Exception safety: <em>nothrow guarantee</em>, provided the
	 * preconditions are met.
	 */
	void notify_nil_handles(CacheKey p_cache_key);

	/**
	 * This should only be called by PersistentObject<T, Connection>.
	 *
	 * Preconditions:\n
	 * The destructor of T must be nothrow; and\n
	 * There is an object cached under p_cache_key in the cache_key_map.
	 *
	 * Exception safety: <em>nothrow guarantee</em>, provided
	 * preconditions are met.
	 */
	void uncache_object(CacheKey p_cache_key);

	/**
	 * Mark an object as no longer corresponding to one in the database.
	 * The object will be retained in the cache, but will not be identified
	 * with any in-database object.
	 *
	 * @todo Testing and documentation of exceptions and exception-safety.
	 */
	void partially_uncache_object(CacheKey p_cache_key);

	// Find the next available cache key
	// WARNING Move the implementation out of the class body.
	/**
	 * @throws sqloxx::OverflowException if the cache has reached
	 * its maximum size (extremely unlikely).
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	CacheKey provide_cache_key();

	typedef typename std::shared_ptr<T> Record;
	typedef std::unordered_map<Id, Record> IdMap;
	typedef std::map<CacheKey, Record> CacheKeyMap;

	// Data members

	// Provides index to all cached objects, including those not as yet
	// saved to the database.
	CacheKeyMap m_cache_key_map;

	// Provides index to object that have been saved to the database,
	// indexed by their primary key.
	IdMap m_id_map;

	// To database connection with which this IdentityMap is associated.
	Connection& m_connection;

	// The last key to have been assigned as an index into m_cache_key_map -
	// or 0 if none have been assigned.
	CacheKey m_last_cache_key = 0;

	// Indicates whether the IdentityMap is currently holding objects
	// indefinitely in the cache (m_caching == true), or whether
	// it is clearing each object out when there are no longer
	// handles pointing to it (m_caching == false).
	bool m_is_caching = false;
};


template <typename T, typename Connection>
inline
IdentityMap<T, Connection>::IdentityMap(Connection& p_connection):
	m_connection(p_connection)
{
	JEWEL_ASSERT (m_id_map.empty());
	JEWEL_ASSERT (m_cache_key_map.empty());
	JEWEL_ASSERT (m_last_cache_key == 0);
	JEWEL_ASSERT (m_is_caching == false);
}

template <typename T, typename Connection>
template <typename DynamicT>
DynamicT*
IdentityMap<T, Connection>::provide_pointer()
{
	static_assert
	(	std::is_same
		<	T,
			typename PersistenceTraits<T>::PrimaryT
		>::value,
		"Invalid instantiation of provide_pointer template."
	);

	static_assert
	(	std::is_same<T, DynamicT>::value ||
		(std::is_polymorphic<T>::value && std::is_base_of<T, DynamicT>::value),
		"Invalid instantiation of provide_pointer template."
	);

	// Comments here are to help ascertain exception-safety.
	Record obj_ptr(new DynamicT(*this, Signature()));  // T-dependent exception safety
	CacheKey const cache_key = provide_cache_key(); // strong guarantee

	// In the next statement:
	// constructing the pair of CacheKeyMap::value_type is nothrow; and
	// calling insert either (a) succeeds, or (b) fails completely and
	// throws std::bad_alloc. If it throws, then obj_ptr
	// will be deleted on exit (as it's a shared_ptr) - which amounts to
	// rollback of provide_pointer<DynamicT>().
	m_cache_key_map.insert
	(	typename CacheKeyMap::value_type(cache_key, obj_ptr)
	);
	// We could have done the following, but the above is more efficient and
	// less "magical".
	// m_cache_key_map[cache_key] = obj_ptr; 

	// Nothrow
	PersistentObject<T, Connection>::
		KeyAttorney::set_cache_key(*obj_ptr, cache_key);

	// In the below, get() is nothrow.
	DynamicT* const ret = dynamic_cast<DynamicT*>(obj_ptr.get());
	JEWEL_ASSERT (ret);
	return ret;
}

template <typename T, typename Connection>
template <typename DynamicT>
DynamicT*
IdentityMap<T, Connection>::provide_pointer(Id p_id)
{
	static_assert
	(	std::is_same
		<	T,
			typename PersistenceTraits<T>::PrimaryT
		>::value,
		"Invalid instantiation of provide_pointer template."
	);

	static_assert
	(	std::is_same<T, DynamicT>::value ||
		(std::is_polymorphic<T>::value && std::is_base_of<T, DynamicT>::value),
		"Invalid instantiation of provide_pointer template."
	);

	if (!PersistentObject<T, Connection>::exists(m_connection, p_id))
	{
		JEWEL_THROW
		(	BadIdentifier,
			"The database does not contain a record of the "
			"requested type with the requested id."
		);
	}
	return unchecked_provide_pointer<DynamicT>(p_id);
}

template <typename T, typename Connection>
template <typename DynamicT>
DynamicT*
IdentityMap<T, Connection>::unchecked_provide_pointer(Id p_id)
{
	static_assert
	(	std::is_same
		<	T,
			typename PersistenceTraits<T>::PrimaryT
		>::value,
		"Invalid instantiation of unchecked_provide_pointer template."
	);

	static_assert
	(	std::is_same<T, DynamicT>::value ||
		(std::is_polymorphic<T>::value && std::is_base_of<T, DynamicT>::value),
		"Invalid instantiation of unchecked_provide_pointer template."
	);

	typename IdMap::iterator it = m_id_map.find(p_id);
	if (it == m_id_map.end())
	{
		// Then we need to create this object.

		// Exception safety here depends on T.
		Record obj_ptr(new DynamicT(*this, p_id, Signature()));

		// atomic, possible sqloxx::OverflowException
		CacheKey const cache_key = provide_cache_key();

		// atomic, possible std::bad_alloc
		m_id_map.insert(typename IdMap::value_type(p_id, obj_ptr));
		try
		{
			m_cache_key_map.insert
			(	typename IdMap::value_type(cache_key, obj_ptr)
			);
		}
		catch (std::bad_alloc&)
		{
			m_id_map.erase(p_id);
			throw;
		}

		// Nothrow
		PersistentObject<T, Connection>::
			KeyAttorney::set_cache_key(*obj_ptr, cache_key);

		// We know this won't throw sqloxx::OverflowError, as it's a
		// newly loaded object.
		DynamicT* const ret = dynamic_cast<DynamicT*>(obj_ptr.get());
		JEWEL_ASSERT (ret);
		return ret;
	}
	JEWEL_ASSERT (it != m_id_map.end());
	if
	(	PersistentObject<T, Connection>::HandleMonitorAttorney::
			has_high_handle_count(*(it->second))
	)
	{
		JEWEL_THROW
		(	sqloxx::OverflowException,
			"Handle count for has reached dangerous level. "
		);
	}
	DynamicT* const ret = dynamic_cast<DynamicT*>(it->second.get());
	JEWEL_ASSERT (ret);
	return ret;
}

template <typename T, typename Connection>
void
IdentityMap<T, Connection>::register_id(CacheKey p_cache_key, Id p_id)
{
	typename CacheKeyMap::const_iterator const finder =
		m_cache_key_map.find(p_cache_key);
	JEWEL_ASSERT (finder != m_cache_key_map.end());  // Precondition
	typedef typename std::pair<typename IdMap::const_iterator, bool>
		InsertionResult;
	typedef typename IdMap::value_type Elem;
	InsertionResult res = m_id_map.insert(Elem(p_id, finder->second));
	if (!res.second)
	{
		// There was already an object with this id. This could occur
		// from a previous save that was cancelled at database
		// transaction level after already cached in the database.
		// We don't want the new object to overwrite the old one in the
		// CacheKeyMap, because other objects could still be referring to it.
		// However, we do want to remove the old object from the IdMap, and
		// we also want to clear the old object's id, to avoid possible
		// confusion between the two objects in client code.
		T& old_obj = *(res.first->second);
		partially_uncache_object
		(	PersistentObject<T, Connection>::KeyAttorney::cache_key(old_obj)
		);
		PersistentObject<T, Connection>::KeyAttorney::clear_id(old_obj);
		res = m_id_map.insert(Elem(p_id, finder->second));
		JEWEL_ASSERT (res.second);
	}
	return;
}

template <typename T, typename Connection>
void
IdentityMap<T, Connection>::deregister_id(Id p_id)
{
	// Precondition
	JEWEL_ASSERT (m_id_map.find(p_id) != m_id_map.end());
	
	m_id_map.erase(p_id);
	return;
}

template <typename T, typename Connection>
void
IdentityMap<T, Connection>::uncache_object(CacheKey p_cache_key)
{
	// Precondition
	JEWEL_ASSERT (m_cache_key_map.find(p_cache_key) != m_cache_key_map.end());
	partially_uncache_object(p_cache_key);  // Erase from m_id_map
	m_cache_key_map.erase(p_cache_key);     // Erase from m_cache_key_map
	return;
}

template <typename T, typename Connection>
void
IdentityMap<T, Connection>::partially_uncache_object(CacheKey p_cache_key)
{
	// Precondition
	JEWEL_ASSERT (m_cache_key_map.find(p_cache_key) != m_cache_key_map.end());
	Record const record = m_cache_key_map.find(p_cache_key)->second;
	if (record->has_id())
	{
		JEWEL_ASSERT (m_id_map.find(record->id()) != m_id_map.end());
		m_id_map.erase(record->id());
	}
	return;
}

template <typename T, typename Connection>
void
IdentityMap<T, Connection>::notify_nil_handles(CacheKey p_cache_key)
{
	typename CacheKeyMap::const_iterator it =
		m_cache_key_map.find(p_cache_key);
	JEWEL_ASSERT (it != m_cache_key_map.end()); // Assert precondition
	if ( !it->second->has_id()  ||  !m_is_caching )
	{
		uncache_object(p_cache_key);
	}
	return;
}

template <typename T, typename Connection>
void
IdentityMap<T, Connection>::enable_caching()
{
	m_is_caching = true;
}

template <typename T, typename Connection>
void
IdentityMap<T, Connection>::disable_caching()
{
	if (m_is_caching)
	{
		for (auto& cache_entry: m_cache_key_map)
		{
			if
			(	PersistentObject<T, Connection>::HandleMonitorAttorney::
					is_orphaned(*(cache_entry.second))
			)
			{
				uncache_object(cache_entry.first);  // TODO Should this be partially_uncache_object?
			}
		}
		m_is_caching = false;
	}
	return;
}

template <typename T, typename Connection>
inline
Connection&
IdentityMap<T, Connection>::connection()
{
	return m_connection;
}

template <typename T, typename Connection>
typename IdentityMap<T, Connection>::CacheKey
IdentityMap<T, Connection>::provide_cache_key()
{
	static CacheKey const maximum = std::numeric_limits<CacheKey>::max();
	typename CacheKeyMap::size_type const sz = m_cache_key_map.size();
	if (sz == 0)
	{
		return m_last_cache_key = 1;  // Intentional assignment
	}
	if (sz == boost::numeric_cast<typename CacheKeyMap::size_type>(maximum))
	{
		// There are no more available positive numbers to serve
		// as cache keys. This is extremely unlikely ever to occur.
		// We could possibly avoid throwing here by instead calling
		// disable_caching(), which would trigger an emptying of the cache
		// of any orphaned objects. But the emptying might take a long, long
		// time. So we just throw.
		// Avoid complication by not even considering negative numbers.
		JEWEL_THROW
		(	OverflowException,
			"No more cache keys are available for identifying objects "
			"in the IdentityMap."
		);
	}
	CacheKey ret = m_last_cache_key;
	typedef typename CacheKeyMap::const_iterator Iterator;
	Iterator it = m_cache_key_map.find(ret);
	Iterator const endpoint = m_cache_key_map.end();
	if (it == endpoint)
	{
		return m_last_cache_key;
	}

	// Look for the first available unused key to assign to next_cache_key()
	// ready for the next call to provide_cache_key(). This relies on
	// CacheKeyMap being, or behaving like, std::map, in that it keeps its
	// elements ordered by key.
	JEWEL_ASSERT (m_cache_key_map.size() > 0);
	while (ret == it->first)
	{
		if (ret == maximum) ret = 1;
		else ++ret;
		if (++it == endpoint) it = m_cache_key_map.begin();
	}
	JEWEL_ASSERT (m_cache_key_map.find(ret) == m_cache_key_map.end());
	return m_last_cache_key = ret;  // Intentional assignment
}

}  // namespace sqloxx

#endif  // GUARD_identity_map_hpp_41089441925794556
