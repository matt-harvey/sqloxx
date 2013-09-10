#ifndef GUARD_table_iterator_hpp
#define GUARD_table_iterator_hpp

#include "general_typedefs.hpp"
#include "sqloxx_exceptions.hpp"
#include "sql_statement.hpp"
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <jewel/assert.hpp>
#include <jewel/optional.hpp>
#include <string>

namespace sqloxx
{

/**
 * Provides a forward iterator interface for traversing a SQLite database
 * table from which instances of some type T can be extracted by reference
 * to a column in the table.
 *
 * The template parameter \e Connection should be \e
 * sqloxx::DatabaseConnection, or a type derived therefrom.
 *
 * In order for this to
 * work, there must be a static member function of T such that the
 * folling expression will return a valid instance of T, assuming
 * \e connection is an open instance of \e Connection and assuming
 * \e statement is a SQLStatement corresponding to the string passed
 * to the two-parameter constructor for TableIterator<Connection, T>.
 *
 *	<tt>
 *
 *	T::create_unchecked(connection, statement.extract<sqloxx::Id>(0));
 *
 *	</tt>
 * 
 * T might (but need not be) an instantiation of sqloxx::Handle. (The
 * "create_unchecked" function in sqloxx::Handle is so named because
 * it creates a Handle without first checking that the Id passed
 * actually exists in the database. This form is relevant for TableIterator
 * because if we have a valid TableIterator, then it \e must be pointing
 * to an existing row in the database table, so there is no need to check
 * for existence.)
 *
 * The API is similar to \e std::istream_iterator. For example -
 *
 * Suppose \e Conn
 * is a type of database connection derived from \e sqloxx::DatabaseConnection
 * and \e Dog is a type with a \e Dog::create_unchecked function of the
 * form described above, and there is a
 * database table named "dog" with the a dolumn "dog_id" being the primary
 * key. Then we can might copy all the \e Dogs into a vector as follows:
 *
 * <tt>
 *
 * 	Conn dbc;
 * 	dbc.open("animals.db");
 * 	std::vector<Dog> vec;
 * 	std::copy
 * 	(	TableIterator<Dog, Conn>(dbc, "select dog_id from dogs"),
 * 		TableIterator<Dog, Conn>(),
 * 		std::back_inserter(vec)
 * 	);
 *
 * </tt>
 *
 * Note instances of this class template are not copyable.
 *
 * @todo HIGH PRIORITY Testing.
 */
template <typename T, typename Connection>
class TableIterator: private boost::noncopyable
{
public:

	/**
	 * Creates a "null" TableIterator. If we are using a "begin" and "end"
	 * pair of iterators to perform, say, some standard library algorithm,
	 * we can use a "null" TableIterator to serve as the "end". A
	 * TableIterator will become "null" once it has
	 * read all the records in the table.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 */
	TableIterator();

	/**
	 * Creates a TableIterator that can traverse a table in the database
	 * \e p_connection, by stepping through the results obtained from
	 * executing an SQL statement constructed using \e p_statement_text.
	 * Note the default value for \e p_statement_text (which will work
	 * in case \e T is an instantiation of \e sqloxx::Handle). If caller
	 * passes a custom statement to \e p_statement_text, it should be
	 * a SELECT statement, and only the first column of results will
	 * be relevant, where the first column should contain the primary
	 * key for T, of type sqloxx::Id. See class-level documentation of
	 * TableIterator for further requirements on T.
	 *
	 * When first constructed, the TableIterator will be "pointing" to
	 * the first object it reads from the database table; dereferencing
	 * it will yield a constant reference to that object.
	 *
	 * @throws InvalidConnection if p_database_connection is an
	 * invalid database connection (i.e. if p_database_connection.is_valid()
	 * returns false).
	 *
	 * @throws SQLiteException or an exception derived therefrom, if there
	 * is some problem with the SQL statement, that results in a SQLite error
	 * code being returned, either in trying to construct a SQLStatement
	 * from p_statement_text, or in stepping into the first row of the
	 * result set.
	 *
	 * @throws std::bad_alloc in the unlikely event of a memory allocation
	 * failure.
	 * 
	 * @throws TooManyStatements if the first purported SQL statement
	 * in p_selector is syntactically acceptable to SQLite, <em>but</em> there
	 * are characters in p_selector after this statement,
	 * other than ';' and ' '.
	 * This includes the case where there are further syntactically
	 * acceptable SQL statements after the first one.
	 *
	 * Might also throw any exception that might be thrown by the function
	 * <em>T::create_unchecked(Connection& T, sqloxx::Id)</em>, since this
	 * function is invoked to initialize the iterator's internal instance
	 * of T from the first row of the SQL result set.
	 *
	 * Exception safety: <em>strong guarantee</em>, providing that the
	 * function <em>T::create_unchecked(Connection&, sqloxx::Id)</em> also
	 * offers at least the strong guarantee.
	 */
	TableIterator
	(	Connection& p_connection,
		std::string const& p_statement_text =
		(	"select " + T::primary_key_name() +
			" from " + T::exclusive_table_name()
		)
	);

	/**
	 * Destructor is virtual, so an instantiation TableIterator might act as
	 * a polymorphic base class.
	 *
	 * Exception safety: will never throw, assuming the destructor for T will
	 * never throw.
	 */
	virtual ~TableIterator();

	/**
	 * @returns a constant reference to the instance of T that is currently
	 * "pointed to" or "contained" within this TableIterator, i.e. the
	 * instance of T that is stored at the database table row that it
	 * is currently "at".
	 *
	 * @throws InvalidTableIterator if the TableIterator is "null" (see the
	 * default constructor documentation re. when an TableIterator is
	 * "null").
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	T const& operator*() const;

	/**
	 * Use this to access a member of the instance of
	 * T that is currently "pointed to" or "contained" within the
	 * TableIterator.
	 *
	 * @throws InvalidTableIterator if the TableIterator is "null" (see the
	 * default constructor documentation re. when an TableIterator is
	 * "null".
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	T const* operator->() const;

	/**
	 * Advance the TableIterator to the next row in the result set.
	 * If the TableIterator is currently at the final row, then advancing
	 * it will result in the TableIterator becoming "null".
	 *
	 * @returns a reference to the TableIterator itself, \e after
	 * it has been advanced.
	 *
	 * @throws InvalidTableIterator if the TableIterator is already
	 * "null".
	 *
	 * @throws InvalidConnection if the database connection is invalid.
	 *
	 * @throws SQLiteException or some exception deriving therefrom, if an
	 * error occurs while stepping to the next result row, that results in a
	 * SQLite error code.
	 * This function should almost never occur, but it is
	 * possible something will fail as the statement is being executed, in
	 * which case the resulting SQLite error condition will trigger the
	 * corresponding exception class. If this occurs, the TableIterator
	 * should not be used again.
	 *
	 * Might also throw any exception that might be thrown by the function
	 * <em>T::create_unchecked(Connection& T, sqloxx::Id)</em>, since this
	 * function is invoked to construct the iterator's internal instance
	 * of T from the next row of the SQL result set.
	 *
	 * Exception safety: <em>basic guarantee</em>, providing that the
	 * function <em>T::create_unchecked(Connection&, sqloxx::Id)</em> also
	 * offers at least the basic guarantee.
	 */
	TableIterator& operator++();

	/**
	 * @returns \e true if and \e only if the TableIterators on \e both sides
	 * are "null". Thus this can be used to compare an existing TableIterator
	 * with a TableIterator known to be null (e.g. one constructed with the
	 * default constructor), to see if it has read the last object in the
	 * result set.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 */
	bool operator==(TableIterator const& rhs);

	/**
	 * @return \e true if and only if the two sides do not compare equal
	 * using the equality operator. See documentation for the equality
	 * operator.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 */
	bool operator!=(TableIterator const& rhs);

private:
	
	void assert_invariant() const;
	void kill_impl_if_invalid();

	class Impl: boost::noncopyable
	{
	public:
		Impl(Connection&, std::string const&);
		~Impl();
		void advance();

		// Client will NOT own the pointer.
		// Impl will take responsibility for managing memory.
		// Pointer might be null.
		T* object_ptr() const;
	
	private:
		T make_object();
		Connection& m_connection;
		SQLStatement m_sql_statement;
		T* m_object;

	};  // class Impl

	Impl* m_impl;

};  // class TableIterator


// IMPLEMENTATION

template <typename T, typename Connection>
inline
TableIterator<T, Connection>::TableIterator(): m_impl(0)
{
	JEWEL_LOG_TRACE();
	assert_invariant();
}

template <typename T, typename Connection>
TableIterator<T, Connection>::TableIterator
(	Connection& p_connection,
	std::string const& p_statement_text
):
	m_impl(0)
{
	JEWEL_LOG_TRACE();
	m_impl = new Impl(p_connection, p_statement_text);
	kill_impl_if_invalid();
	assert_invariant();
}

template <typename T, typename Connection>
TableIterator<T, Connection>::~TableIterator()
{
	JEWEL_LOG_TRACE();
	delete m_impl;
	m_impl = 0;
	assert_invariant();
}

template <typename T, typename Connection>
T const&
TableIterator<T, Connection>::operator*() const
{
	if (!m_impl)
	{
		JEWEL_THROW
		(	InvalidTableIterator,
			"Attempted to dereference an invalid TableIterator."
		);
	}
	JEWEL_ASSERT (m_impl->object_ptr());
	return *(m_impl->object_ptr());
}

template <typename T, typename Connection>
T const*
TableIterator<T, Connection>::operator->() const
{
	if (!m_impl)
	{
		JEWEL_THROW
		(	InvalidTableIterator,
			"Attempted to dereference an invalid TableIterator."
		);
	}
	JEWEL_ASSERT (m_impl->object_ptr());
	return m_impl->object_ptr();
}

template <typename T, typename Connection>
TableIterator<T, Connection>&
TableIterator<T, Connection>::operator++()
{
	if (!m_impl)
	{
		JEWEL_THROW
		(	InvalidTableIterator,
			"Attempted to increment invalid TableIterator."
		);
	}
	m_impl->advance();
	kill_impl_if_invalid();
	assert_invariant();
	return *this;
}

template <typename T, typename Connection>
inline
bool
TableIterator<T, Connection>::operator==(TableIterator const& rhs)
{
	return !rhs.m_impl && !m_impl;
}

template <typename T, typename Connection>
inline
bool
TableIterator<T, Connection>::operator!=(TableIterator const& rhs)
{
	return !(*this == rhs);
}

template <typename T, typename Connection>
inline
void
TableIterator<T, Connection>::assert_invariant() const
{
	JEWEL_ASSERT ((m_impl == 0) || (m_impl->object_ptr() != 0));
}

template <typename T, typename Connection>
inline
void
TableIterator<T, Connection>::kill_impl_if_invalid()
{
	if (m_impl && !m_impl->object_ptr())
	{
		delete m_impl;
		m_impl = 0;
	}
	return;
}

template <typename T, typename Connection>
inline
TableIterator<T, Connection>::Impl::Impl
(	Connection& p_connection,
	std::string const& p_statement_text
):
	m_connection(p_connection),
	m_sql_statement(p_connection, p_statement_text),
	m_object(0)
{
	JEWEL_LOG_TRACE();
	if (m_sql_statement.step())
	{
		m_object = new T(make_object());
	}
}

template <typename T, typename Connection>
inline
TableIterator<T, Connection>::Impl::~Impl()
{
	JEWEL_LOG_TRACE();
	delete m_object;
	m_object = 0;
}

template <typename T, typename Connection>
void
TableIterator<T, Connection>::Impl::advance()
{
	JEWEL_ASSERT (m_object);
	if (m_sql_statement.step())
	{
		m_object->~T();
		new(m_object) T(make_object());
	}
	else
	{
		delete m_object;
		m_object = 0;
	}
	return;
}

template <typename T, typename Connection>
inline
T*
TableIterator<T, Connection>::Impl::object_ptr() const
{
	return m_object;
}

template <typename T, typename Connection>
inline
T
TableIterator<T, Connection>::Impl::make_object()
{
	return T::create_unchecked
	(	m_connection,
		m_sql_statement.template extract<Id>(0)
	);
}



}  // namespace sqloxx

#endif  // GUARD_table_iterator_hpp
