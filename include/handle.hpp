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

#ifndef GUARD_handle_hpp_03158044118042125
#define GUARD_handle_hpp_03158044118042125

#include "handle_counter.hpp"
#include "id.hpp"
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
 * Handle for handling business objects of type \b T where \b T is a class
 * derived from <b>PersistentObject<T, Connection></b> for some type
 * \b Connection, and is managed via \b IdentityMap<PersistenceTraits<T>::Base>
 * to ensure that only one
 * instance of \b T exists in memory at any one time, in relation to any given
 * record in the database.
 *
 * \b T should be associated with a instance of <b>T::Connection</b>,
 * and with a member function
 * template \b T::Connection::identity_map<S>() that is specialized for \b S
 * where \b S is <b>identity_map<PersistentTraits<T>::Base></b>, and that
 * returns an instance of \b IdentityMap<Base> that is
 * unique to that database connection, for managing instances of \b Base.
 * (See separate documentation for IdentityMap.) (By default \b Base will be
 * the same type as \b T, but it need not be; see documentation
 * for \b PersistenceTraits.)
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
     * Construct a null Handle, i.e. a Handle to which no instance
     * of \b T is bound.
     *
     * <b>Exception safety</b>: <em>nothrow guarantee</em>.
     */
    Handle();

    /**
     * Constructs a Handle to a new instance of \b T, which has \e not yet
     * been persisted to the database represented by \e p_connection.
     * The handled object will be persisted to \e p_connection if and when it is
     * saved. The object will be managed by the \b IdentityMap<T> associated
     * with \e p_connection (returned by
     * <em>p_connection.identity_map<T>()</em>).
     *
     * @throws sqloxx::OverflowException in the extremely unlikely event that
     * the in-memory cache already has so many objects loaded that an
     * additional object could not be cached without causing arithmetic
     * overflow in the process of assigning it a key.
     *
     * @throws std::bad_allocation in the unlikely event of memory allocation
     * failure during the creating and caching of the instance of \b T.
     *
     * <em>In addition</em>, any exceptions thrown from the \b T constructor
     * of the form <em>T(IdentityMap<T>&)</em> may also be thrown
     * from this Handle constructor.
     *
     * <b>Exception safety</b>: depends on the constructor for \b T. If this
     * constructor
     * offers the strong guarantee, then the Handle constructor will also
     * offer the <em>strong guarantee</em> (although in case of an
     * exception, the internal state of the IdentityMap may be altered in a
     * way that temporarily affects performance or size of allocated memory,
     * but not program logic).
     */
    explicit Handle(Connection& p_connection);

    /**
     * Constructs a Handle to an instance of \b T corresponding to one that has
     * already been persisted to the database represented by \e p_connection,
     * with a primary key of \e p_id. The object will be managed by the
     * \b IdentityMap<T> associated with \e p_connection (returned by
     * <em>p_connection.identity_map<T>()</em>.
     *
     * @throws sqloxx::OverflowException in the extremely unlikely event that
     * the in-memory cache already has so many objects loaded that an
     * additional object could not be cached without causing arithmetic
     * overflow in the process of assigning it a key.
     *
     * @throws std::bad_allocation in the unlikely event of memory allocation
     * failure during the creating and caching of the instance of \b T.
     *
     * @throws sqloxx::BadIdentifier if there is no record in the database
     * of type \b T that has \e p_id as its primary key.
     *
     * @throws sqloxx::InvalidConnection in case the database connection is
     * invalid.
     *
     * <em>In addition</em>, any exceptions thrown from the \b T constructor
     * of the form <em>T(IdentityMap<T>&)</em> may also be thrown
     * from this Handle constructor.
     *
     * <b>Exception safety</b>: depends on the constructor for \b T. If this
     * constructoroffers the strong guarantee, then the Handle constructor will
     * also offer the <em>strong guarantee</em> (although in case of an
     * exception, the internal state of the IdentityMap may be altered in a
     * way that temporarily affects performance or size of allocated memory,
     * but not program logic). For this guarantee to hold, it is also required
     * that the destructor for \b T not throw.
     */
    Handle(Connection& p_connection, Id p_id);

    /**
     * <b>Preconditions</b>:\n
     * the object must have been managed
     * throughout its life by (a single instance of) IdentityMap,
     * and must only have ever been
     * accessed via instances of Handle<Derived>; and\n
     * The destructor of Derived must be non-throwing.
     *
     * <b>Exception safety</b>: <em>nothrow guarantee</em>, provided the
     * preconditions are met.
     */
    ~Handle();

    /**
     * @returns a Handle to an instance of \b T corresponding to one that has
     * already been persisted to the database represented by \e p_connection
     * with a primary key of \e p_id, assuming such an instance of b T exists.
     * This function should not be called unless it is known that there exists
     * in the database an object of type \b T with \b p_id as its primary key.
     *
     * <em>Calling create_unchecked for an object that is NOT in the database
     * with the given Id, causes <b>undefined behaviour</b>.</em>
     *
     * This function may be significantly faster than calling the constructor
     * for Handle directly, as it does not check whether the requested primary
     * key exists.
     *
     * @throws std::bad_alloc if there is a memory allocation failure in the
     * process of loading and caching the object in the
     * relevant sqloxx::IdentityMap (in case it is not already
     * cached).
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
     * <em>In addition</em>, any exceptions thrown from the \b T constructor
     * may also be thrown by this function.
     *
     * <b>Exception safety</b> depends on the constructor of \b T of the form
     * <em>T(IdentityMapT&, Id, IdentityMap::Signature const&)</em>.
     * Provided this constructor offers at least the
     * <em>strong guarantee</em>, then create_unchecked() offers the
     * <em>strong guarantee</em> (although there may be some internal cache
     * state that is not rolled back but which does not affect client code).
     * For this guarantee to hold, it is also required that the destructor
     * of \b T not throw. Remember also that if no object of type \b T exists in
     * the database with \e p_id as its primary key, then behaviour is
     * undefined.
     */
    static Handle create_unchecked(Connection& p_connection, Id p_id);

    /**
     * @throws sqloxx::OverflowException in the extremely unlikely
     * event that the number of Handle instances pointing to the
     * underlying instance of \b T is too large to be safely counted
     * by the type <em>PersistentObject<T, Connection>::HandleCounter</em>.
     *
     * <b>Exception safety</b>: <em>strong guarantee</em>.
     */
    Handle(Handle const& rhs);
    
    /**
     * @throws sqloxx::OverflowException in the extremely unlikely
     * event that the number of Handle instances pointing to the
     * underlying instance of \b T is too large to be safely counted
     * by the type <b>PersistentObject<T, Connection>::HandleCounter</b>.
     *
     * <b>Exception safety</b>: <em>strong guarantee</em>.
     */
    Handle& operator=(Handle const& rhs);

    /** Move constructor.
     *
     * <b>Exception safety</b>: <em>nothrow guarantee</em>.
     */
    Handle(Handle&& rhs) noexcept;

    /** Move assignment.
     *
     * <b>Exception safety</b>: <em>nothrow guarantee</em>.
     */
    Handle& operator=(Handle&&) noexcept;

    /**
     * @returns \e true if this Handle<T> is bound to some instance
     * of T; otherwise returns \e false.
     *
     * <b>Exception safety</b>: <em>nothrow guarantee</em>.
     */
    explicit operator bool() const;

    /**
     * @returns the instance of T that is handled by this Handle<T>.
     *
     * @throws UnboundHandleException if there is no instance of
     * \b T bound to this Handle.
     *
     * <b>Exception safety</b>: <em>strong guarantee</em>.
     */
    T& operator*() const;

    /**
     * Indirection operator analagous to <b>operator*()</b>, for
     * accessing members of \b T via the underlying pointer.
     *
     * @throws UnboundHandleException if there is no instance
     * of \b T bound to this Handle.
     *
     * <b>Exception safety</b>: <em>strong guarantee</em>.
     */
    T* operator->() const;

    /**
     * Handles are equal if and only if they are handling the same
     * underlying object.
     * 
     * <b>Exception safety</b>: <em>nothrow guarantee</em>.
     */
    bool operator==(Handle const& rhs) const;

    /**
     * Handles are unequal if an only if they are handling distinct
     * underlying objects.
     *
     * <b>Exception safety</b>: <em>nothrow guarantee</em>.
     */
    bool operator!=(Handle const& rhs) const;

private:

    typedef IdentityMap<Base> IdentityMapT;
    typedef typename IdentityMapT::template HandleAttorney<T> AttorneyT;

    explicit Handle(T* p_pointer);

    T* m_pointer;
};

// NON-MEMBER FUNCTIONS

/**
 * A dynamic_cast is attempted on the underlying pointer. If it
 * succeeds, then the returned \b Handle<L> will point to one and the
 * same object as rhs. If it fails, then the returned
 * \b Handle<L> will be null.
 *
 * In order successfully to instantiate this function template:
 *
 * \b R must be such that \b PersistenceTraits<L>::Base is the same type
 * as \b PersistenceTraits<R>::Base;\n
 * \b L and \b R must also be such that \b L::Connection is the same type
 * as \b R::Connection; and\n
 * \b L must be a base class of \b R, or must be one and the same
 * type as \b R, or else \b R must be a base class of \b L.\n
 *
 * These conditions are enforced statically: compilation will fail
 * if they do not hold.
 *
 * @throws sqloxx::OverflowException in the extremely unlikely
 * event that the number of Handle instances pointing to the
 * underlying instance of \b T is too large to be safely counted
 * by the type \b PersistentObject<T, Connection>::HandleCounter.
 *
 * <b>Exception safety</b>: <em>strong guarantee</em>.
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
Handle<T>::Handle(Connection& p_connection): m_pointer(nullptr)
{
    m_pointer = AttorneyT::get_pointer
    (   p_connection.template identity_map<Base>()
    );
    JEWEL_ASSERT (m_pointer);
    m_pointer->increment_handle_counter();
}

template <typename T>
Handle<T>::Handle(Connection& p_connection, Id p_id): m_pointer(nullptr)
{
    m_pointer = AttorneyT::get_pointer
    (   p_connection.template identity_map<Base>(),
        p_id
    );
    JEWEL_ASSERT (m_pointer);
    m_pointer->increment_handle_counter();
}

template <typename T>
inline
Handle<T>::~Handle()
{
    if (m_pointer) m_pointer->decrement_handle_counter();
}

template <typename T>
Handle<T>
Handle<T>::create_unchecked(Connection& p_connection, Id p_id)
{
    return Handle<T>
    (   AttorneyT::unchecked_get_pointer
        (   p_connection.template identity_map<Base>(),
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
Handle<T>::Handle(Handle&& rhs) noexcept: m_pointer(std::move(rhs.m_pointer))
{
    rhs.m_pointer = nullptr;
}

template <typename T>
Handle<T>&
Handle<T>::operator=(Handle&& rhs) noexcept
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
    (   std::is_base_of<L, R>::value ||
        std::is_base_of<R, L>::value ||
        std::is_same<L, R>::value,
        "Invalid instantiation of sqloxx::handle_cast."
    );
    static_assert
    (   std::is_same
        <    typename PersistenceTraits<L>::Base,
            typename PersistenceTraits<R>::Base
        >::value,
        "Invalid instantiation of sqloxx::handle_cast."
    );
    static_assert
    (   std::is_same
        <    typename L::Connection,
            typename R::Connection
        >::value,
        "Invalid instantiation of sqloxx::handle_cast."
    );
    Handle<L> ret;  // nothrow
    if (rhs.m_pointer)  // nothrow
    {
        ret.m_pointer = dynamic_cast<L*>(rhs.m_pointer);  // nothrow
        if (ret.m_pointer)
        {
            // might throw sqloxx::OverflowException - strong guarantee
            ret.m_pointer->increment_handle_counter();
        }
    }
    return ret;
}

}  // namespace sqloxx

#endif  // GUARD_handle_hpp_03158044118042125
