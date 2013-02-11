#ifndef GUARD_handle_hpp
#define GUARD_handle_hpp

#include "general_typedefs.hpp"
#include "identity_map.hpp"
#include "sqloxx_exceptions.hpp"

namespace sqloxx
{



/**
 * Handle for handling business objects of type T where T is a class
 * derived from PersistentObject and is
 * managed via IdentityMap<T> to ensure only one instance of T exists in
 * memory at any one time, in relation to any given record in a given
 * database.
 *
 * T should be associated with a instance of Connection, where Connection
 * is a subclass of sqloxx::DatabaseConnection, with a member function
 * identity_map<T>() that returns an instance of IdentityMap<T, Connection>
 * unique to that database connection, for managing instances of T. (See
 * separate documentation for IdentityMap.)
 *
 * @todo Testing.
 */
template <typename T>
class Handle
{
public:

	static std::string primary_key_name();
	static std::string primary_table_name();

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
	 * Preconditions:\n
	 * Connection should be a subclass of sqloxx::DatabaseConnection and
	 * should define identity_map<T>().
	 *
	 * Constructs a Handle to a new instance of T, which has \e not yet
	 * been persisted to the database represented by p_connection. Connection
	 * should be derived from sqloxx::DatabaseConnection. The handled object
	 * will be persisted to p_connection if and when it is saved. The
	 * object will be managed by the IdentityMap<T, Connection> associated
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
	 * of the form T(IdentityMap<T, Connection>&) may also be thrown
	 * from this Handle constructor.
	 *
	 * Exception safety: depends on the constructor for T. If this constructor
	 * offers the strong guarantee, then the Handle constructor will also
	 * offer the <em>strong guarantee</em> (although in the case of an
	 * exception, the internal state of the IdentityMap may be altered in a
	 * way that temporarily affects performance or size of allocated memory,
	 * but not program logic).
	 */
	template <typename Connection>
	explicit Handle(Connection& p_connection);

	/**
	 * @todo Documentation.
	 */
	template <typename Connection>
	Handle(Connection& p_connection, Id p_id);

	/**
	 * @todo Documentation and testing.
	 */
	template <typename Connection>
	Handle
	static create_unchecked(Connection& p_connection, Id p_id);

	/**
	 * @throws sqloxx::OverflowException in the extremely unlikely
	 * event that the number of Handle instances pointing to the
	 * underlying instance of T is too large to be safely counted
	 * by the type PersistentObject<T, Connection>::HandleCounter.
	 *
	 * Note it is necessary for both const& and non-const& forms
	 * to be defined here, otherwise the compiler confuses it
	 * with the templated single-parameter constructor. (OK,
	 * possibly the const& form is never getting called. But
	 * we want to make certain the compiler doesn't create
	 * an undesirable version for us...)
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	Handle(Handle const& rhs);
	Handle(Handle& rhs);

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
	 * @returns \e true if this Handle<T> is bound to some instance
	 * of T; otherwise returns \e false.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 *
	 * @todo Testing.
	 */
	operator bool() const;

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
	
	Handle(T* p_pointer);

	T* m_pointer;
};


template <typename T>
std::string
Handle<T>::primary_key_name()
{
	return T::primary_key_name();
}

template <typename T>
std::string
Handle<T>::primary_table_name()
{
	return T::primary_table_name();
}

template <typename T>
Handle<T>::~Handle()
{
	m_pointer->notify_handle_destruction();
}


template <typename T>
template <typename Connection>
Handle<T>::Handle(Connection& p_connection):
	m_pointer(0)
{
	m_pointer = IdentityMap<T, Connection>::HandleAttorney::get_pointer
	(	p_connection.template identity_map<T>()
	);
	m_pointer->notify_handle_construction();
}

template <typename T>
template <typename Connection>
Handle<T>::Handle(Connection& p_connection, Id p_id):
	m_pointer(0)
{
	m_pointer = IdentityMap<T, Connection>::HandleAttorney::get_pointer
	(	p_connection.template identity_map<T>(),
		p_id
	);
	m_pointer->notify_handle_construction();
}


template <typename T>
template <typename Connection>
Handle<T>
Handle<T>::create_unchecked(Connection& p_connection, Id p_id)
{
	return Handle<T>
	(	IdentityMap<T, Connection>::HandleAttorney::unchecked_get_pointer
		(	p_connection.template identity_map<T>(),
			p_id
		)
	);
}


template <typename T>
Handle<T>::Handle(Handle const& rhs):
	m_pointer(rhs.m_pointer)
{
	m_pointer->notify_handle_copy_construction();
}


template <typename T>
Handle<T>::Handle(Handle& rhs):
	m_pointer(rhs.m_pointer)
{
	m_pointer->notify_handle_copy_construction();
}


template <typename T>
Handle<T>&
Handle<T>::operator=(Handle const& rhs)
{
	// Strong guarantee, provided rhs has a valid pointer...
	rhs.m_pointer->notify_rhs_assignment_operation();

	// Nothrow guarantee, provided preconditions met, and
	// provided rhs has a valid pointer.
	m_pointer->notify_lhs_assignment_operation();

	m_pointer = rhs.m_pointer;  // nothrow
	return *this;  // throw, provided we have a valid pointer
}


template <typename T>
Handle<T>::operator bool() const
{
	return static_cast<bool>(m_pointer);  // nothrow
}


template <typename T>
T&
Handle<T>::operator*() const
{
	if (static_cast<bool>(m_pointer))  // nothrow
	{
		return *m_pointer;  // nothrow
	}
	throw (UnboundHandleException("Unbound Handle."));
}


template <typename T>
T*
Handle<T>::operator->() const
{
	if (static_cast<bool>(m_pointer))  // nothrow
	{
		return m_pointer;  // nothrow
	}
	throw (UnboundHandleException("Unbound Handle."));
}

template <typename T>
bool
Handle<T>::operator==(Handle<T> const& rhs) const
{
	return m_pointer == rhs.m_pointer;
}

template <typename T>
bool
Handle<T>::operator!=(Handle<T> const& rhs) const
{
	return m_pointer != rhs.m_pointer;
}


template <typename T>
Handle<T>::Handle(T* p_pointer):
	m_pointer(p_pointer)
{
	m_pointer->notify_handle_construction();
}

		

}  // namespace sqloxx

#endif  // GUARD_handle_hpp
