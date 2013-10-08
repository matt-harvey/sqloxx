// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_handle_hpp_03158044118042125
#define GUARD_handle_hpp_03158044118042125

#include "general_typedefs.hpp"
#include "identity_map.hpp"
#include "persistence_traits.hpp"
#include "sqloxx_exceptions.hpp"
#include <jewel/assert.hpp>
#include <jewel/exception.hpp>
#include <type_traits>
#include <utility>

namespace sqloxx
{

/**
 * Handle for handling business objects of type T where T is a class
 * derived from PersistentObject<T, Connection> for some type Connection, and is
 * managed via IdentityMap<PersistenceTraits<T>::Base> to ensure only one
 * instance of T exists in memory at any one time, in relation to any given
 * record in the database.
 *
 * T should be associated with a instance of T::Connection,
 * with a member function
 * template identity_map<S>() that is specialized for
 * S = PersistentTraits<T>::Base, that returns an instance of
 * IdentityMap<Base> that is
 * unique to that database connection, for managing instances of Base.
 * (See separate documentation for IdentityMap.) (By default Base will be
 * the same type as T, but it need not be; see documentation
 * for PersistenceTraits.)
 *
 * @todo Testing.
 */
template <typename T>
class Handle
{
public:

	typedef typename T::Connection Connection;
	typedef typename PersistenceTraits<T>::Base Base;

	template <typename L, typename R>
	friend Handle<L> handle_cast(Handle<R> const& rhs);

	static std::string primary_key_name();
	static std::string primary_table_name();
	static std::string exclusive_table_name();

	/**
	 * Construct a null Handle. Cannot be dereferenced.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 *
	 * @todo test
	 */
	Handle();

	/**
	 * Preconditions:\n
	 * the object must have been managed
	 * throughout its life by (a single instance of) IdentityMap,
	 * and must only have ever been
	 * accessed via instances of Handle<Derived>; and\n
	 * The destructor of Derived must be non-throwing.
	 *
	 * Exception safety: <em>nothrow guarantee</em>, provided the
	 * preconditions are met.
	 */
	~Handle();

	/**
	 * Constructs a Handle to a new instance of T, which has \e not yet
	 * been persisted to the database represented by p_connection. Connection
	 * should be derived from sqloxx::DatabaseConnection. The handled object
	 * will be persisted to p_connection if and when it is saved. The
	 * object will be managed by the IdentityMap<T> associated
	 * with p_connection (returned by p_connection.identity_map<T>()).
	 *
	 * @throws sqloxx::OverflowException in the extremely unlikely event that
	 * the in-memory cache already has so many objects loaded that an
	 * additional object could not be cached without causing arithmetic
	 * overflow in the process of assigning it a key.
	 *
	 * @throws std::bad_allocation in the unlikely event of memory allocation
	 * failure during the creating and caching of the instance of T.
	 *
	 * <em>In addition</em>, any exceptions thrown from the T constructor
	 * of the form T(IdentityMap<T>&) may also be thrown
	 * from this Handle constructor.
	 *
	 * Exception safety: depends on the constructor for T. If this constructor
	 * offers the strong guarantee, then the Handle constructor will also
	 * offer the <em>strong guarantee</em> (although in the case of an
	 * exception, the internal state of the IdentityMap may be altered in a
	 * way that temporarily affects performance or size of allocated memory,
	 * but not program logic).
	 */
	explicit Handle(Connection& p_connection);

	/**
	 * @todo Documentation.
	 */
	Handle(Connection& p_connection, Id p_id);

	/**
	 * Calling create_unchecked for an object that is NOT in the database with
	 * the given id, causes UNDEFINED BEHAVIOUR.
	 *
	 * @todo Documentation and testing.
	 */
	static Handle create_unchecked(Connection& p_connection, Id p_id);

	/**
	 * @throws sqloxx::OverflowException in the extremely unlikely
	 * event that the number of Handle instances pointing to the
	 * underlying instance of T is too large to be safely counted
	 * by the type PersistentObject<T, Connection>::HandleCounter.
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	Handle(Handle const& rhs);
	
	/**
	 * @throws sqloxx::OverflowException in the extremely unlikely
	 * event that the number of Handle instances pointing to the
	 * underlying instance of T is too large to be safely counted
	 * by the type PersistentObject<T, Connection>::HandleCounter.
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	Handle& operator=(Handle const& rhs);

	/**
	 * @todo HIGH PRIORITY Testing and documentation.
	 */
	Handle(Handle&& rhs);

	/**
	 * @todo HIGH PRIORITY Testing and documentation.
	 */
	Handle& operator=(Handle&&);

	/**
	 * @returns \e true if this Handle<T> is bound to some instance
	 * of T; otherwise returns \e false.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 *
	 * @todo Testing.
	 */
	explicit operator bool() const;

	/**
	 * @returns the instance of T that is handled by this Handle<T>.
	 *
	 * @throws UnboundHandleException if there is no instance of
	 * T bound to this Handle.
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	T& operator*() const;

	/**
	 * Indirection operator analagous to operator*(), for
	 * accessing members of T via the underlying pointer.
	 *
	 * @throws UnboundHandleException if there is no instance
	 * of T bound to this Handle.
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	T* operator->() const;

	/**
	 * Handles are equal if and only if they are handling the same
	 * underlying object.
	 * 
	 * Exception safety: <em>nothrow guarantee</em>.
	 *
	 * @todo Test.
	 */
	bool operator==(Handle const& rhs) const;
	bool operator!=(Handle const& rhs) const;

private:

	typedef IdentityMap<Base> IdentityMapT;
	typedef typename IdentityMapT::template HandleAttorney<T> AttorneyT;

	explicit Handle(T* p_pointer);

	T* m_pointer;
};

// NON-MEMBER FUNCTIONS

/**
 * R must be such that PersistenceTraits<L>::Base is the same type
 * as PersistenceTraits<R>::Base.
 *
 * L and R must also be such that L::Connection is the same type
 * as R::Connection.
 *
 * Finally, L must be a base class of R, or must be one and the same
 * type as R, or else R must be a base class of L.
 *
 * These preconditions enforced using static_asserts.
 *
 * A dynamic_cast is attempted on the underlying pointer. If it
 * succeeds, then the returned Handle<L> will point to one and the
 * same object of the common base type. If it fails, then the returned
 * pointer will be null.
 *
 * @todo Document exception safety, and test.
 */
template <typename L, typename R>
Handle<L>
handle_cast(Handle<R> const& rhs);


// FUNCTION IMPLEMENTATIONS

template <typename T>
inline
std::string
Handle<T>::primary_key_name()
{
	return T::primary_key_name();
}

template <typename T>
inline
std::string
Handle<T>::primary_table_name()
{
	return T::primary_table_name();
}

template <typename T>
inline
std::string
Handle<T>::exclusive_table_name()
{
	return T::exclusive_table_name();
}

template <typename T>
inline
Handle<T>::Handle(): m_pointer(nullptr)
{
}

template <typename T>
inline
Handle<T>::~Handle()
{
	if (m_pointer) m_pointer->decrement_handle_counter();
}

template <typename T>
Handle<T>::Handle(Connection& p_connection): m_pointer(nullptr)
{
	m_pointer = AttorneyT::get_pointer
	(	p_connection.template identity_map<Base>()
	);
	JEWEL_ASSERT (m_pointer);
	m_pointer->increment_handle_counter();
}

template <typename T>
Handle<T>::Handle(Connection& p_connection, Id p_id): m_pointer(nullptr)
{
	m_pointer = AttorneyT::get_pointer
	(	p_connection.template identity_map<Base>(),
		p_id
	);
	JEWEL_ASSERT (m_pointer);
	m_pointer->increment_handle_counter();
}

template <typename T>
Handle<T>
Handle<T>::create_unchecked(Connection& p_connection, Id p_id)
{
	return Handle<T>
	(	AttorneyT::unchecked_get_pointer
		(	p_connection.template identity_map<Base>(),
			p_id
		)
	);
}

template <typename T>
Handle<T>::Handle(Handle const& rhs): m_pointer(rhs.m_pointer)
{
	if (m_pointer) m_pointer->increment_handle_counter();
}

template <typename T>
Handle<T>&
Handle<T>::operator=(Handle const& rhs)
{
	if (this != &rhs)
	{
		// Strong guarantee.
		if (rhs.m_pointer) rhs.m_pointer->increment_handle_counter();

		// Nothrow guarantee, provided preconditions met.
		if (m_pointer) m_pointer->decrement_handle_counter();

		m_pointer = rhs.m_pointer;
	}
	return *this;
}

template <typename T>
inline
Handle<T>::Handle(Handle&& rhs): m_pointer(std::move(rhs.m_pointer))
{
	rhs.m_pointer = nullptr;
}

template <typename T>
Handle<T>&
Handle<T>::operator=(Handle&& rhs)
{
	if (this != &rhs)
	{
		m_pointer = std::move(rhs.m_pointer);
		rhs.m_pointer = nullptr;
	}
	return *this;
}

template <typename T>
inline
Handle<T>::operator bool() const
{
	return m_pointer != nullptr;
}


template <typename T>
T&
Handle<T>::operator*() const
{
	if (m_pointer)
	{
		return *m_pointer;
	}
	JEWEL_THROW(UnboundHandleException, "Unbound Handle.");
}

template <typename T>
T*
Handle<T>::operator->() const
{
	if (m_pointer)
	{
		return m_pointer;
	}
	JEWEL_THROW(UnboundHandleException, "Unbound Handle.");
}

template <typename T>
inline
bool
Handle<T>::operator==(Handle<T> const& rhs) const
{
	return m_pointer == rhs.m_pointer;
}

template <typename T>
inline
bool
Handle<T>::operator!=(Handle<T> const& rhs) const
{
	return m_pointer != rhs.m_pointer;
}


template <typename T>
inline
Handle<T>::Handle(T* p_pointer): m_pointer(p_pointer)
{
	if (m_pointer) m_pointer->increment_handle_counter();
}

template <typename L, typename R>
Handle<L>
handle_cast(Handle<R> const& rhs)
{
	static_assert
	(	std::is_base_of<L, R>::value ||
		std::is_base_of<R, L>::value ||
		std::is_same<L, R>::value,
		"Invalid instantiation of sqloxx::handle_cast."
	);
	static_assert
	(	std::is_same
		<	typename PersistenceTraits<L>::Base,
			typename PersistenceTraits<R>::Base
		>::value,
		"Invalid instantiation of sqloxx::handle_cast."
	);
	static_assert
	(	std::is_same
		<	typename L::Connection,
			typename R::Connection
		>::value,
		"Invalid instantiation of sqloxx::handle_cast."
	);
	Handle<L> ret;
	if (rhs.m_pointer)
	{
		ret.m_pointer = dynamic_cast<L*>(rhs.m_pointer);
		if (ret.m_pointer) ret.m_pointer->increment_handle_counter();
	}
	return ret;
}



}  // namespace sqloxx

#endif  // GUARD_handle_hpp_03158044118042125
