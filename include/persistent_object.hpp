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

#ifndef GUARD_persistent_object_hpp_3601795073413195
#define GUARD_persistent_object_hpp_3601795073413195

#include "database_transaction.hpp"
#include "handle.hpp"
#include "handle_counter.hpp"
#include "id.hpp"
#include "identity_map.hpp"
#include "next_auto_key.hpp"
#include "persistence_traits.hpp"
#include "sql_statement.hpp"
#include "sqloxx_exceptions.hpp"
#include <boost/optional.hpp>
#include <jewel/assert.hpp>
#include <jewel/exception.hpp>
#include <jewel/log.hpp>
#include <jewel/optional.hpp>
#include <exception>
#include <string>
#include <type_traits>

namespace sqloxx
{

/**
 * Class template for creating objects persisted to a database. In client
 * code, this should be inherited by a derived class that defines certain
 * functions as detailed below. For an example of such a derived
 * client class, see the classes sqloxx::tests::ExampleA,
 * sqloxx::tests::ExampleB and sqloxx::tests::ExampleB, in
 * example.hpp and example.cpp.
 *
 * An instance of (a class deriving from an instantiation of)
 * PersistentObject represents a "business object" for which the
 * data will be stored in a relational database (currently only
 * SQLite is supported) under a single primary key, that shall be
 * an auto-incrementing integer primary key.
 *
 * <b>IMPORTANT</b>: Only handle a PersistentObject instances via Handle
 * instances.
 *
 * <b>Identity Map Pattern</b>
 *
 * PersistentObject is intended
 * to be used in conjunction with the Identity Map pattern (as
 * detailed in Martin Fowler's book "Patterns of Enterprise
 * Architecture"). To enable this, sqloxx::PersistentObject is intended to
 * work in conjunction with sqloxx::IdentityMap and sqloxx::Handle.
 *
 * Instances of DerivedT should only ever be handled via a Handle.
 * Handles can be copied around, dereferenced, and otherwise treated
 * similarly to a shared_ptr. The PersistentObject part of the DerivedT
 * instance, together
 * with Handle and IdentityMap, will work behind the scenes to ensure
 * that, for each record in the database, at most one object is loaded
 * into memory (i.e. cached in the IdentityMap). This prevents problems
 * with objects being edited across multiple instances.
 *	
 * See sqloxx::IdentityMap and sqloxx::Handle for further documentation
 * here.
 *
 * <b>Lazy Loading</b>
 *
 * PersistentObject provides for lazy loading behaviour,
 * using the "ghost" pattern as described on p. 202 of Martin Fowler's
 * "Patterns of Enterprise Application Architecture". The PersistentObject
 * base class provides the bookkeeping associated with this pattern,
 * keeping track of the loading status of each in-memory object
 * ("loaded", "loading" or "ghost").
 *
 * in the DerivedT class, the intention is that some or all data members
 * declared in that class, can be "lazy". This means that they are not
 * initialized in the derived object's constructor, but are rather only
 * initialized at a later time via a call to load(), which in turn calls
 * the virtual method \b do_load() (which needs to be defined in the
 * derived class).
 *
 * In the derived class, implementations of getters
 * for attributes
 * other than those that are loaded immediately on construction, should
 * have \e load() as their first statement. (This means that getters in
 * DerivedT cannot
 * be const.) (See documentation for load().)
 *
 * In addition, implementations of \e all setters in the
 * derived class should have load() as their first statement.
 * Failure to adhere to these requirements will result in
 * in undefined behaviour.
 *
 * It is advisable to store lazy attributes in a boost::optional<T>, which
 * will result in loud, rather than silent, failure, in the event of an
 * attempt to access such an attribute before it has been initialized.
 *
 * DerivedT classes are free to initialize all attributes on construction of
 * an instance. This avoids the complications associated
 * with lazy loading, while giving up the potential efficiencies
 * that lazy loading can provide.
 * 
 *
 * <b>Virtual functions</b>
 *
 * The following functions need to be provided with definitions
 * provided in the DerivedT class:
 *
 * <em>static std::string \b exclusive_table_name();</em>\n
 * Should return, without side effects, the name of table in which
 * instances of the derived class
 * are persisted in the database. If instances fields are persisted in
 * multiple tables (which is often the case where class hierarchies
 * are involved), the exclusive_table_name() should return the name
 * of the table in which <em>all and only</em> primary keys of \e T
 * occur (this generally corresponds to the derived class table, not
 * the base class table).
 *
 * <em>static std::string \b primary_key_name();</em>\n
 * Should return, without side effects, the name of the primary
 * key column for DerivedT. This primary key column must appear in
 * the table named by \e exclusive_table_name(). The primary key
 * must be a single-column integer primary key that is autoincrementing
 * (using the SQLite "autoincrement" key word).
 * If PersistentTraits<...> has been specialized for DerivedT, then it
 * is the class PersistenceTraits<T>::Base for which primary_key_name()
 * must be defined, rather than DerivedT per se. The primary key name must
 * be the same both in the table name by DerivedT::exclusive_table_name() and
 * Base::exclusive_table_name().
 *
 * <em>virtual void \b do_load() = 0;</em>\n
 * See documentation of \e load() function.
 *
 * <em>virtual void \b do_save_existing() = 0;</em>\n
 * See documentation for \e save_existing() function.
 *
 * <em>virtual void \b do_save_new() = 0;</em>\n
 * See documentation for \e save_new() function.
 *
 * In addition the following functions \e may be provided with a definition
 * in the DerivedT class, although the PersistentObject base class provides
 * a default implementation which is suitable in many cases:
 *
 * <em>virtual void \b do_remove()</em>;\n
 * See documentation for \e remove() function.
 *
 * <em>virtual void \b do_ghostify()</em>;\n
 * See documentation for \e ghostify() function.
 *
 * <b>Template parameters</b>
 *
 * @param DerivedT The derived class. DerivedT should inherit publicly
 * from PersistentObject<DerivedT, ConnectionT> per the Curiously Recurring
 * Template Pattern (CRTP).
 *
 * @param ConnectionT The type of the database connection through which
 * instances of DerivedT will be persisted to the database. ConnectionT
 * should be a class derived from sqloxx::DatabaseConnection.
 *
 * TODO LOW PRIORITY Have a single location for documenting use of Sqloxx
 * holistically, perhaps with an extended example (but see "tests/example.hpp"
 * and "tests/example.cpp" already done).
 * 
 * <b>Pitfalls</b>
 *
 * Care needs to be taken when different PersistentObject instances
 * hold references or Handles to each other. For more on this issue, see the
 * documentation for IdentityMap::~IdentityMap.
 */
template <typename DerivedT, typename ConnectionT>
class PersistentObject
{
public:

	typedef ConnectionT Connection;
	typedef typename sqloxx::PersistenceTraits<DerivedT>::Base Base;
	typedef sqloxx::IdentityMap<Base> IdentityMap;

	template <typename T> friend class Handle;

	template <typename L, typename R>
	friend Handle<L> handle_cast(Handle<R> const& rhs);

	/**
	 * @returns a reference to the database connection with which
	 * this instance of PersistentObject is associated. This is where the
	 * object will be loaded from or saved to, as the case may be.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>.
	 */
	ConnectionT& database_connection() const;

	// note copy constructor is protected

	PersistentObject(PersistentObject&&) = delete;
	PersistentObject& operator=(PersistentObject const&) = delete;
	PersistentObject& operator=(PersistentObject&&) = delete;

	/**
	 * Destructor.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>.
	 */
	virtual ~PersistentObject();

	/**
	 * @returns PersistenceTraits<T>::Base::exclusive_table_name().
	 *
	 * Behaviour regarding exceptions and exception safety is determined
	 * by the behaviour of
	 * PersistenceTraits<T>::Base::exclusive_table_name(), which
	 * is provided by the client.
	 */
	static std::string primary_table_name();
	
	/**
	 * @returns \e true if and only if a record with \e p_id as its
	 * primary key exists in
	 * the database to which \e p_database_connection is connected,
	 * in the "exclusive table" for T (i.e. the table with the
	 * name returned by exclusive_table_name()).
	 * Note the database is always checked, not the
	 * IdentityMap.
	 *
	 * @throws InvalidConnection if the database connecton is
	 * invalid.
	 *
	 * @throws std::bad_alloc in case of memory allocation failure
	 * (very unlikely)
	 *
	 * @throws SQLiteException (or derivative thereof) in case of a
	 * SQLite error during
	 * execution. (This is extremely unlikely, but cannot be entirely
	 * ruled out.)
	 *
	 * <b>Exception safety</b>: <em>strong guarantee</em>.
	 */
	static bool exists(ConnectionT& p_database_connection, Id p_id);

	/**
	 * @returns \e true if and only if there are no objects of type DerivedT
	 * saved in the database.
	 * Note the database is always checked, not the IdentityMap.
	 *
	 * @throws InvalidConnection if the database connecton is
	 * invalid.
	 *
	 * @throws std::bad_alloc in case of memory allocation failure
	 * (very unlikely)
	 *
	 * @throws SQLiteException (or derivative thereof) in case of a
	 * SQLite error during
	 * execution. (This is extremely unlikely, but cannot be entirely
	 * ruled out.)
	 *
	 * <b>Exception safety</b>: <em>strong guarantee</em>.
	 */
	static bool none_saved(ConnectionT& p_database_connection);

	/**
	 * <b>Preconditions</b>:\n
	 * The destructor of DerivedT must be non-throwing;\n
	 * We have handled this object only via a Handle, with
	 * the Handle having been copied or assigned from another
	 * Handle, or obtained by a call to one of Handle's
	 * constructors or factory functions;\n
	 * If the object has an id, the id corresponds to the primary
	 * key of an object of this type that exists in the database;\n
	 * \b do_save_existing() and \b do_save_new() must be defined in such
	 * a way that they do not change the logical state of the object being
	 * saved, or of any other objects, but have side-effects only
	 * in respect of the database;\n
	 * \b do_save_existing() and \b do_save_new() must be been defined so that
	 * if they fail, they throw std::exception or an exception derived
	 * therefrom;\n
	 * Every setter and getter method defined in the DerivedT class must have
	 * a call to load() as its first statement (see below for
	 * explanation);\n
	 * \b DerivedT::do_ghostify() must be defined so as to be non-throwing;\n
	 * \b DerivedT::do_load() preconditions must be met (see documentation
	 * for load());
	 * 
	 * The result of calling save() depends on whether the in-memory
	 * object has an id.

	 * (1) <b>Object has id</b>
	 *
	 * If the object has an id - i.e. if it corresponds to an object already
	 * existent in the database - then
	 * calling save() will result in the object in the database being updated
	 * with
	 * the state of the current in-memory object.
	 * This is done by a call
	 * to virtual function \b do_save_existing(), which much be defined
	 * in the class DerivedT. The base method save() takes care of wrapping
	 * the call to \b do_save_existing() as a single SQL transaction by calling
	 * begin_transaction() and end_transaction() on the database connection.
	 * 
	 * <em>Important:</em> Before calling \b do_save_existing(), and before
	 * beginning the
	 * SQL transaction, the base save() method first ensures that the object
	 * is in a fully loaded state (as we don't want to save a partial
	 * object to the database). This is done via a call to load().
	 * If the object is <em>in a ghost state and has an id</em> at this point,
	 * then the entire
	 * state of the in-memory object will be overwritten by the state of
	 * the in-database object, and \e then the in-memory object will be
	 * saved - with the net result being that any changes to the in-memory
	 * object are lost, and the in-database object remains unchanged!
	 * If, on the other, hand, the object is in a fully loaded state at
	 * the point save() is called, then the call to load() has no effect.
	 * (The call to load() also has no effect if the object doesn't yet have
	 * an id; but that's not relevant here as we're considering the case
	 * where the object has an id.)
	 * The upshot of all this is that, in order to make sure that
	 * changes to the in-memory object remain in the in-memory object and
	 * are subsequently written to the database
	 * when save() is called, you should always call load() as
	 * the \e first statement in the implementation of any setter method in
	 * the DerivedT class.
	 * 
	 * (2) <b>Object does not have id</b>
	 *
	 * If the object does not have an id - i.e. if it does not correspond or
	 * purport to correspond to an object already saved to the database - then
	 * calling save() will result in the in-memory object being saved to the
	 * database as an additional item, rather than overwriting existing data.
	 * In other words, a new record will be created in the database.
	 * This is done
	 * via a call to virtual function \b do_save_new(), which must be defined in
	 * the class DerivedT. The base save() function takes care of wrapping
	 * this call as a SQL transaction. The base function also takes care of:
	 * assigning an id to the newly saved object in the database; recording
	 * this id in the in-memory object; and notifying the IdentityMap
	 * (i.e. the "cache") for this object, that it has been saved and assigned
	 * its id.
	 *
	 * After saving the object as above, whether via (1) or (2), the in-memory
	 * object is marked internally as being in a fully loaded, i.e.
	 * "complete" state.
	 *
	 * In defining \b do_save_new(), the class DerivedT should ensure that a call
	 * to \b do_save_new() results in a \e complete object of its type being
	 * inserted into the database. The semantics of save() here only make
	 * sense if this is the case. The Sqloxx framework does not provide for
	 * the saving of objects "a bit at a time".
	 *
	 * @throws TableSizeException if the object does not have an id, but
	 * the greatest primary key value already in the primary table for the
	 * type DerivedT is
	 * the maximum possible value for the type Id, so that another row
	 * could not be inserted without overflow.\n
	 * 
	 * @throws std::bad_alloc in the unlikely event of memory allocation
	 * failure during execution.\n
	 *
	 * @throws TransactionNestingException if the maximum transaction
	 * nesting level of the DatabaseConnection has been reached (extremely
	 * unlikely).\n
	 *
	 * @throws InvalidConnection if the DatabaseConnection is
	 * invalid.\n
	 *
	 * @throws UnresolvedTransactionException if there is failure in
	 * the process of committing the database transaction, or if there is
	 * some other failure, followed by a failure in the process of
	 * \e formally cancelling the database transaction. If this is
	 * thrown (which is extremely unlikely), it is recommended that the
	 * application be gracefully terminated. The database transaction
	 * \e will be fully rolled back, but further transaction during the
	 * same application session may jeopardize that situation.
	 *
	 * May also throw other derivatives of DatabaseException if there is
	 * a failure finding the next primary key value for the object in case
	 * it doesn't already have an id. This should not occur except in the
	 * case of a corrupt database, or a memory allocation error (very
	 * unlikely).
	 *
	 * May also throw exceptions from \b do_save_new() and/or \b do_save_exising(),
	 * depending on how those functions are defined in the derived class.
	 *
	 * <b>Exception safety</b>: <em>basic guarantee</em>. Possible outcomes
	 * from calling save() are as follows -\n
	 *  (a) Complete success;\n
	 *  (b) Failure with exception thrown and no effect on program
	 *  state;\n
	 *  (c) If the object has an id, save() may fail and throw an
	 *  exception but with the object left in a
	 * ghost state.\n
	 * In either (b) or (c), it is possible that failure may occur
	 * with \b UnresolvedTransactionException being thrown. If this occurs,
	 * it is recommended that the application session be gracefully
	 * terminated.
	 * Note that (b) and (c) are functionally equivalent to one another
	 * as far as client
	 * code is concerned, providing the preconditions are met.
	 */
	void save();

	/**
	 * Delete an object of type DerivedT from the database. The
	 * function will also inform the IdentityMap in which this
	 * object is cached, and the cache will update itself accordingly.
	 * After calling remove(), the object will no longer have an id;
	 * however its other attributes will be unaltered (assuming the
	 * DerivedT class doesn't define \b do_remove() in such a way as to
	 * alter them - see below).
	 *
	 * <b>Preconditions</b>:\n
	 * If the default implementation of \b do_remove() is not redefined
	 * by the class DerivedT, then the preconditions of \b do_remove()
	 * must be satisfied (see separate documentation for \b do_remove());\n
	 * If \b do_remove() is redefined by DerivedT, then it should offer the
	 * <em>strong guarantee</em>, i.e. be atomic, in respect of the state of
	 * the in-memory objects (but note, the base remove() method takes
	 * care of wrapping the implementation as a SQL transaction, so
	 * in general, \b do_remove() doesn't need to worry about atomicity
	 * in regards to the database);\n
	 * \b DerivedT::do_ghostify() should be defined so as to adhere to the
	 * preconditions detailed in the documentation for ghostify();\n and
	 * Getters and setters in DerivedT should always call load() as their
	 * first statement.
	 *
	 * @throws std::bad_alloc in the unlikely event of mememory allocation
	 * failure during execution.\n
	 *
	 * @throws InvalidConnection if the database connection is invalid.\n
	 *
	 * @throws TransactionNestingException if the maximum transaction
	 * nesting level of the DatabaseConnection has been reached (extremely
	 * unlikely).\n
	 *
	 * @throws UnresolvedTransactionException if there is failure in
	 * the process of committing the database transaction, or if there is
	 * some other failure, followed by a failure in the process of
	 * \e formally cancelling the database transaction. If this is
	 * thrown (which is extremely unlikely), it is recommended that the
	 * application be gracefully terminated. The database transaction
	 * \e will be fully rolled back, but further transaction during the
	 * same application session may jeopardize that situation.
	 *
	 * <b>Exception safety</b>: <em>basic guarantee</em>. If an exception other
	 * than \b UnresolvedTransactionException is thrown, then the
	 * application state will be effectively rolled back, and although the
	 * object may be left in a ghost state, this should require no
	 * special handling by the client code provided the preconditions are
	 * met. If \b UnresolvedTransactionException is thrown, then, provided
	 * the application exits the current session without executing any
	 * further database transactions, the application and database state
	 * will be in a state of having been effectively rolled back, when the
	 * next session commences.
	 */
	void remove();

	/**
	 * @returns the id of the object, if it has one.
	 *
	 * @throws \b jewel::UninitializedOptionalException if the object doesn't
	 * have an id.
	 * 
	 * <b>Exception safety</b>: <em>strong guarantee</em>.
	 */
	Id id() const;

	/**
	 * @returns \e true if and only if this instance of PersistentObject has
	 * a valid id; otherwise returns \e false. Note a valid id is merely one
	 * that has been initialised. It need not actually exist in the database.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>.
	 */
	bool has_id() const;

	/**
	 * Reverts the object to a "ghost state". This is a state in
	 * which only certain member variables (typically, only the id)
	 * are initialized. This is done by calling the private virtual
	 * function \b do_ghostify(). This may be redefined by class DerivedT,
	 * but by default, \b do_ghostify() has an empty body.
	 * Then, the base ghostify() method marks the object as being in a
	 * "ghost" state.
	 *
	 * \b DerivedT::do_ghostify() should be defined in such a way that,
	 * when executed, the object is put into such a state that, the
	 * next time load() is called, the object can be fully reloaded to
	 * a valid loaded state without any issues of duplication or etc.
	 * For example, if one of the member variables of DerivedT is a
	 * vector, and if loading the object involves pushing elements
	 * onto the vector, then \b do_ghostify() should ensure that the
	 * vector is emptied, so that after load() is called next, the
	 * object contains only one lot of elements.
	 *
	 * <em>\b do_ghostify() should be defined
	 * such as to provide the nothrow guarantee. This makes it much
	 * easier for certain other functions to maintain exception-safety.
	 * </em>
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>, provided the
	 * \b DerivedT::do_ghostify() method is non-throwing.
	 */
	void ghostify();

	/// @cond
	/**
	 * Provides access to get m_cache_key, set m_cache_key,
	 * and clear m_id, only to
	 * to IdentityMap<Base>.
	 */
	class KeyAttorney
	{
	public:
		friend class sqloxx::IdentityMap<Base>;
	private:
		static void set_cache_key(DerivedT& p_obj, Id p_cache_key)
		{
			p_obj.set_cache_key(p_cache_key);
			return;
		}
		static Id cache_key(DerivedT& p_obj)
		{
			return jewel::value(p_obj.m_cache_key);
		}
		static void clear_id(DerivedT& p_obj)
		{
			p_obj.clear_id();
			return;
		}
	};
	
	friend class KeyAttorney;

	/**
	 * Controls access to functions that monitor the number of
	 * Handle instances pointing to a given instance of
	 * PersistentObject<T, ConnectionT>, deliberately restricting
	 * this access to IdentityMap<Base>
	 */
	class HandleMonitorAttorney
	{
	public:
		friend class sqloxx::IdentityMap<Base>;
	private:
		static bool is_orphaned(DerivedT const& p_obj)
		{
			return p_obj.is_orphaned();
		}
		static bool has_high_handle_count(DerivedT const& p_obj)
		{
			return p_obj.has_high_handle_count();
		}
	};

	friend class HandleMonitorAttorney;

	/// @endcond

protected:

	/**
	 * This should only be called by DerivedT.
	 *
	 * Create a PersistentObject that corresponds (or purports to correspond)
	 * to one that already exists in the database.
	 *
	 * @param p_identity_map IdentityMap with
	 * which the PersistentObject is associated.
	 *
	 * @param p_id the id of the object as it exists in the database. This
	 * presumably will be, or correspond directly to, the primary key.
	 *
	 * <b>Preconditions</b>:\n
	 * Note that even if there is no corresponding object in the database for
	 * the given value \e p_id, this constructor will still proceed without
	 * complaint. The constructor does not actually perform any checks on the
	 * validity either of the DatabaseConnection or of \e p_id. The caller should
	 * be sure, before calling this function, that there exists in the
	 * database a row representing an instance of the DerivedT type, with \e p_id
	 * as it primary key. If no such row exists, then UNDEFINED BEHAVIOUR will
	 * result, including the possibility of silent or delayed corruption of
	 * data.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>.
	 */
	PersistentObject(IdentityMap& p_identity_map, Id p_id);

	/** 
	 * This should only be called by DerivedT.
	 *
	 * Create a PersistentObject that does \e not correspond to
	 * one that already exists in the database.
	 *
	 * @param p_identity_map IdentityMap with which the
	 * PersistentObject is to be associated.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>.
	 */
	explicit PersistentObject(IdentityMap& p_identity_map);

	/**
	 * Calls the derived class's implementation
	 * of \b do_load(), if and only if the object is not already
	 * loaded. If the object is already loaded, it does nothing.
	 * Also if the object does not have an id,
	 * then this function does nothing, since there would be nothing
	 * to load.
	 *
	 * <b>Preconditions</b>:\n
	 * In defining \b do_load(), the DerivedT class should throw an instance
	 * of \b std::exception (which may be an instance of any exception class
	 * derived therefrom) in the event that the load fails;\n
	 * \b do_load() should not perform any write operations on the database;\n
	 * \b do_load() should be such that if it fails, ghostify() can safely be
	 * called on the object;\n
	 * \b do_load() should be such that if it fails, any external state
	 * (outside of the object itself) will then be just
	 * as it was prior to the unsuccessful call to load();\n
	 * The DerivedT class should define \b do_ghostify() according to the
	 * preconditions specified in the documentaton of ghostify(); and\n
	 * The destructor of DerivedT must be non-throwing.
	 *
	 * Note the implementation is wrapped as a transaction
	 * by calls to DatabaseConnection::begin_transaction() and \b
	 * DatabaseConnection::end_transaction().
	 * This is taken care of by the base load() method.
	 *
	 * The following exceptions may be thrown regardless of how
	 * \b do_load() is defined:
	 *
	 * @throws TransactionNestingException in the event that the maximum
	 * level of transaction nesting for the database connection has been
	 * reached. (This is extremely unlikely.) If this occurs \e before
	 * \b do_load() is entered, the object will be as it was before the
	 * function was called.\n
	 * 
	 * @throws InvalidConnection in the event that the database connection is
	 * invalid at the point the \e load function is entered. If this occurs,
	 * the object will be as it was before this function was called.\n
	 *
	 * @throws std::bad_alloc in the event of memory allocation failure
	 * during execution.\n
	 *
	 * @throws UnresolvedTransactionException if there is failure in
	 * the process of committing the database transaction, or if there is
	 * some other failure, followed by a failure in the process of
	 * \e formally cancelling the database transaction. If this is
	 * thrown (which is extremely unlikely), it is recommended that the
	 * application be gracefully terminated. The database transaction
	 * \e will be fully rolled back, but attempting further transactions
	 * during the same application session may jeopardize that situation.
	 *
	 * <b>Exception safety</b>: <em>basic guarantee</em>, provided the
	 * preconditions are met. Either there will be complete success,
	 * or the object will be left in a ghost state, functionally
	 * equivalent, as far as client code is concerned, to the state
	 * it was in prior to load() being called. The possibility of
	 * \b UnresolvedTransactionException means the strong guarantee cannot
	 * be provided, however (see above).
	 */
	void load();

	/**
	 * Copy constructor is deliberately protected. Copy construction does
	 * not make much semantic sense, as each instance of PersistentObject is
	 * supposed to represent a \e unique object in the database, with a
	 * unique id. However we provide it to derived classes, who may wish
	 * to use it in, for example, copy-and-swap operations.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em> (though derived classes'
	 * copy constructors might, of course, throw).
	 */
	PersistentObject(PersistentObject const&) = default;

	/**
	 * Swap function. This swaps the base (sqloxx::PersistentObject) part of the
	 * object only: it does not call down into any virtual functions.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>
	 */
	void swap(PersistentObject& rhs);

	/**
	 * @returns the id that would be assigned to this instance of
	 * PersistentObject when saved to the database. This uses SQLite's
	 * built-in auto-incrementing primary key.
	 *
	 * @throws sqloxx::LogicError in the event this instance already has
	 * an id.\n
	 *
	 * @throws sqloxx::TableSizeException if the greatest primary key value
	 * already in the table (i.e. the table into which this instance of
	 * PersistentObject would be persisted) is the maximum value for the
	 * type \e Id, so that another row could not be inserted without overflow.\n
	 *
	 * @throws std::bad_alloc is the unlikely event that memory allocation
	 * fails during execution.\n
	 *
	 * @throws sqloxx::DatabaseException, or a derivative therefrom, may
	 * be thrown if there is some other
	 * error finding the next primary key value. This should not occur except
	 * in the case of a corrupt database, or a memory allocation error
	 * (extremely unlikely), or the database connection being invalid
	 * (including because not yet connected to a database file).
	 * The particular child class of DatabaseException thrown will depend
	 * on the type of error, e.g. InvalidConnection will be thrown
	 * in the event of an invalid database connection.
	 *
	 * <b>Exception safety</b>: the default implementation offers the
	 * <em>strong guarantee</em>.
	 */
	Id prospective_key() const;

	/**
	 * This function is called by remove(). For that function, and the
	 * role of \b do_remove() within that function, see the separate
	 * documentation for remove().
	 *
	 * The following relates the default implementation of \b do_remove()
	 * provided by PersistentObject<DerivedT, ConnectionT>.
	 *
	 * The default implementation of this function will simply delete
	 * the row with the primary key returned by id(), in the table
	 * named by PersistenceTraits<DerivedT>::Base::exclusive_table_name().
	 *
	 * @throws InvalidConnection if the database connection is invalid.
	 *
	 * @throws std::bad_alloc in the unlikely event of a memory
	 * allocation failure in execution.
	 *
	 * <b>Exception safety</b>: <em>strong guarantee</em>.
	 */
	virtual void do_remove();

private:

	virtual void do_load() = 0;
	virtual void do_save_existing() = 0;
	virtual void do_save_new() = 0;

	/**
	 * Unless overridden, this simply has an empty body.
	 */
	virtual void do_ghostify();

	/**
	 * @returns \e true if and only if there are no Handle
	 * instances pointing to this object.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>.
	 */
	bool is_orphaned() const;
	
	/**
	 * @returns \e true if and only if we are dangerously close to reaching
	 * the maximum value of HandleCounter.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>.
	 */
	bool has_high_handle_count() const;

	/**
	 * Called by Handle to trigger increment of reference count.
	 * constructed, but ordinarily constructed).
	 * 
	 * @throws sqloxx::OverflowException if the maximum value
	 * for type HandleCounter has been reached, such that additional Handle
	 * cannot be safely counted. On the default type for HandleCounter,
	 * this should be extremely unlikely.
	 *
	 * <b>Exception safety</b>: <em>strong guarantee</em>
	 */
	void increment_handle_counter();
	
	/**
	 * Called by Handle via to decrement reference count.
	 *
	 * <b>Preconditions</b>:\n
	 * This function should only be called from Handle.
	 * This instance of DerivedT must have been handled throughout
	 * its life only via instances of Handle, that have been obtained
	 * from a single instance of IdentityMap<Base>
	 * via calls to the IdentityMap API, or else have been copied from other
	 * instances of Handle; and\n
	 * The destructor of derived must be non-throwing.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em> is offered, providing
	 * the preconditions are met.
	 */
	void decrement_handle_counter();

	/**
	 * Called by IdentityMap<Base> via KeyAttorney to
	 * provide a "cache
	 * key" to the object. The cache key is used by IdentityMap to
	 * identify the object in its internal cache. Every object created by
	 * the IdentityMap will have a cache key, even if it doesn't have an id.
	 *
	 * <b>Exception safety</b>: <em>nothrow guarantee</em>.
	 */
	void set_cache_key(Id p_cache_key);

	/**
	 * Clears m_id
	 */
	void clear_id();

	enum LoadingStatus
	{
		ghost = 0,
		loading,
		loaded
	};
	
	// Data members

	// non-owning pointer to IdentityMap in which object is cached.
	IdentityMap* m_identity_map;

	// Represent primary key in database. If the object does not correspond to
	// and does not purport to correspond to any record in the database, then
	// m_id in unitialized.
	boost::optional<Id> m_id;
	
	// Represents the identifier, in the IdentityMap for
	// m_database_connection, of an instance of DerivedT. The
	// IdentityMap can look up a PersistentObject either via its id
	// (which corresponds to its primary key in the database), or via
	// its cache_key. PersistentObject instances that are newly created and
	// have not yet been saved to the database will not have an id (i.e. m_id
	// will be in an uninitialized state), however these may still be managed
	// by the IdentityMap, and so still need a means for the IdentityMap to
	// identify them in their internal cache. Hence the need for a cache_key
	// distinct from the id.
	boost::optional<Id> m_cache_key;

	LoadingStatus m_loading_status;
	HandleCounter m_handle_counter;
};




template <typename DerivedT, typename ConnectionT>
PersistentObject<DerivedT, ConnectionT>::PersistentObject
(	IdentityMap& p_identity_map,
	Id p_id
):
	m_identity_map(&p_identity_map),
	m_id(p_id),
	m_loading_status(ghost),
	m_handle_counter(0)
{
}

template <typename DerivedT, typename ConnectionT>
inline
PersistentObject<DerivedT, ConnectionT>::PersistentObject
(	IdentityMap& p_identity_map	
):
	m_identity_map(&p_identity_map),
	m_loading_status(ghost),
	m_handle_counter(0)
	// Note m_cache_key is left unitialized. It is the responsibility
	// of IdentityMap to call set_cache_key after construction,
	// before providing a Handle to a newly created DerivedT instance.
{
}

template <typename DerivedT, typename ConnectionT>
inline
PersistentObject<DerivedT, ConnectionT>::~PersistentObject()
{
}

template <typename DerivedT, typename ConnectionT>
inline
std::string
PersistentObject<DerivedT, ConnectionT>::primary_table_name()
{
	return Base::exclusive_table_name();
}

template <typename DerivedT, typename ConnectionT>
bool
PersistentObject<DerivedT, ConnectionT>::exists
(	ConnectionT& p_database_connection,
	Id p_id
)
{
	// Could throw std::bad_alloc
	static std::string const text =
		"select * from " +
		DerivedT::exclusive_table_name() +
		" where " +
		Base::primary_key_name() +
		" = :p";
	// Could throw InvalidConnection or SQLiteException
	SQLStatement statement(p_database_connection, text);
	// Could throw InvalidConnection or SQLiteException
	statement.bind(":p", p_id);
	// Could throw InvalidConnection or SQLiteException
	return statement.step();
}

template <typename DerivedT, typename ConnectionT>
bool
PersistentObject<DerivedT, ConnectionT>::none_saved
(	ConnectionT& p_database_connection
)
{
	// Could throw std::bad_alloc
	static std::string const text =
		"select * from " +
		DerivedT::exclusive_table_name();
	// Could throw InvalidConnection or SQLiteException
	SQLStatement statement(p_database_connection, text);
	// Could throw InvalidConnection or SQLiteException
	return !statement.step();
}

template <typename DerivedT, typename ConnectionT>
void
PersistentObject<DerivedT, ConnectionT>::load()
{
	while (m_loading_status == loading)
	{
		// Wait
	}
	if (m_loading_status == ghost && has_id())
	{
		DatabaseTransaction transaction(database_connection());
		m_loading_status = loading;
		try
		{
			do_load();	
			transaction.commit();
		}
		catch (std::exception&)
		{
			ghostify();
			transaction.cancel();
			throw;
		}
		m_loading_status = loaded;
	}
	return;
}

template <typename DerivedT, typename ConnectionT>
void
PersistentObject<DerivedT, ConnectionT>::save()
{
	JEWEL_ASSERT (m_cache_key);  // precondition
	if (has_id())  // nothrow
	{
		// basic guarantee, under preconditions of do_load (see load())
		load(); 

		// strong guarantee
		DatabaseTransaction transaction(database_connection());
		try
		{
			do_save_existing();  // Safety depends on DerivedT
			transaction.commit();  // Strong guarantee
		}
		catch (std::exception&)
		{
			ghostify();  // nothrow (assuming preconditions met)
			transaction.cancel();
			throw;
		}
	}
	else
	{
		Id const allocated_id = prospective_key();  // strong guarantee
		DatabaseTransaction transaction(database_connection());// strong guar.
		try
		{
			do_save_new();  // Safety depends on DerivedT

			// strong guarantee
			IdentityMap::PersistentObjectAttorney::register_id
			(	*m_identity_map,
				*m_cache_key,
				allocated_id
			);
			try
			{
				transaction.commit(); // strong guarantee
			}
			catch (std::exception&)
			{
				// nothrow (assuming preconditions met)
				IdentityMap::PersistentObjectAttorney::deregister_id
				(	*m_identity_map,
					allocated_id
				);
				throw;
			}
		}
		catch (std::exception&)
		{
			jewel::clear(m_id);  // nothrow
			transaction.cancel();
			throw;
		}
		m_id = allocated_id; // nothrow
	}
	m_loading_status = loaded;  // nothrow
	return;
}

template <typename DerivedT, typename ConnectionT>
void
PersistentObject<DerivedT, ConnectionT>::remove()
{
	if (has_id())
	{
		DatabaseTransaction transaction(database_connection());// strong guar.
		try
		{
			do_remove(); // safety depends on derived. by default strong guar.
			transaction.commit();  // strong guarantee
		}
		catch (std::exception&)
		{
			ghostify();  // nothrow, providing preconditions met
			transaction.cancel();
			throw;
		}
		// nothrow (conditional)
		IdentityMap::PersistentObjectAttorney::partially_uncache_object
		(	*m_identity_map,
			*m_cache_key
		);

		jewel::clear(m_id);  // nothrow
	}
	return;
}

template <typename DerivedT, typename ConnectionT>
inline
Id
PersistentObject<DerivedT, ConnectionT>::id() const
{
	return jewel::value(m_id);
}

template <typename DerivedT, typename ConnectionT>
inline
void
PersistentObject<DerivedT, ConnectionT>::set_cache_key(Id p_cache_key)
{
	m_cache_key = p_cache_key;
	return;
}

template
<typename DerivedT, typename ConnectionT>
void
PersistentObject<DerivedT, ConnectionT>::clear_id()
{
	jewel::clear(m_id);
	return;
}


template <typename DerivedT, typename ConnectionT>
void
PersistentObject<DerivedT, ConnectionT>::increment_handle_counter()
{
	if (m_handle_counter == std::numeric_limits<HandleCounter>::max())
	{
		JEWEL_THROW
		(	OverflowException,
			"Handle counter for PersistentObject instance has reached "
			"maximum value and cannot be safely incremented."
		);
	}
	++m_handle_counter;
	return;
}

template <typename DerivedT, typename ConnectionT>
void
PersistentObject<DerivedT, ConnectionT>::decrement_handle_counter()
{
	switch (m_handle_counter)
	{
	case 1:
		--m_handle_counter;
		// Will not throw, provided the destructor of DerivedT
		// is non-throwing, and the object is saved in the cache
		// under m_cache_key.
		if (m_cache_key)
		{
			IdentityMap::PersistentObjectAttorney::notify_nil_handles
			(	*m_identity_map,
				*m_cache_key
			);
		}
		break;
	case 0:
		// Do nothing
		break;
	default:
		JEWEL_ASSERT (m_handle_counter > 1);
		--m_handle_counter;
	}
	return;
}

template <typename DerivedT, typename ConnectionT>
inline
ConnectionT&
PersistentObject<DerivedT, ConnectionT>::database_connection() const
{
	JEWEL_ASSERT (m_identity_map);
	return m_identity_map->connection();
}

template <typename DerivedT, typename ConnectionT>
Id
PersistentObject<DerivedT, ConnectionT>::prospective_key() const
{
	if (has_id())
	{
		JEWEL_THROW
		(	LogicError,
			"Object already has id so prospective_key does not apply."
		);
	}
	return next_auto_key<ConnectionT, Id>
	(	database_connection(),
		primary_table_name()
	);
}

template <typename DerivedT, typename ConnectionT>
void
PersistentObject<DerivedT, ConnectionT>::do_remove()
{
	// primary_table_name() might throw std::bad_alloc (strong guar.).
	// primary_key_name() might throw might throw InvalidConnection or
	// std::bad_alloc.
	std::string const statement_text =
		"delete from " + primary_table_name() + " where " +
		Base::primary_key_name() + " = :p";
	
	// Might throw InvalidConnection or std::bad_alloc
	SQLStatement statement(database_connection(), statement_text);
	statement.bind(":p", id());  // Might throw InvalidConnection
	// throwing above this point will have no effect

	statement.step_final();  // Might throw InvalidConnection
	return;
}

template <typename DerivedT, typename ConnectionT>
inline
void
PersistentObject<DerivedT, ConnectionT>::do_ghostify()
{
	// do nothing
	return;
}

template <typename DerivedT, typename ConnectionT>
inline
bool
PersistentObject<DerivedT, ConnectionT>::has_id() const
{
	// Relies on the fact that m_id is a boost::optional<Id>, and
	// will convert to true if and only if it has been initialized.
	return m_id;
}

template <typename DerivedT, typename ConnectionT>
inline
bool
PersistentObject<DerivedT, ConnectionT>::is_orphaned() const
{
	return m_handle_counter == 0;
}

template <typename DerivedT, typename ConnectionT>
inline
bool
PersistentObject<DerivedT, ConnectionT>::has_high_handle_count() const
{
	static HandleCounter const safe_limit =
		std::numeric_limits<HandleCounter>::max() - 2;
	return m_handle_counter >= safe_limit;
}

template <typename DerivedT, typename ConnectionT>
void
PersistentObject<DerivedT, ConnectionT>::
ghostify()
{
	do_ghostify();
	m_loading_status = ghost;
	return;
}
		
template
<typename DerivedT, typename ConnectionT>
void
PersistentObject<DerivedT, ConnectionT>::swap
(	PersistentObject& rhs
)
{
	IdentityMap* const temp_id_map = rhs.m_identity_map;
	boost::optional<Id> const temp_id = rhs.m_id;
	boost::optional<Id> const temp_cache_key = rhs.m_cache_key;
	LoadingStatus const temp_loading_status = rhs.m_loading_status;
	HandleCounter const temp_handle_counter = rhs.m_handle_counter;

	rhs.m_identity_map = m_identity_map;
	rhs.m_id = m_id;
	rhs.m_cache_key = m_cache_key;
	rhs.m_loading_status = m_loading_status;
	rhs.m_handle_counter = m_handle_counter;

	m_identity_map = temp_id_map;
	m_id = temp_id;
	m_cache_key = temp_cache_key;
	m_loading_status = temp_loading_status;
	m_handle_counter = temp_handle_counter;

	return;
}


}  // namespace sqloxx


#endif  // GUARD_persistent_object_hpp_3601795073413195






