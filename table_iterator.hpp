#ifndef GUARD_table_iterator_hpp_9025044682913015
#define GUARD_table_iterator_hpp_9025044682913015

#include "general_typedefs.hpp"
#include "sqloxx_exceptions.hpp"
#include "sql_statement.hpp"
#include <boost/optional.hpp>
#include <jewel/assert.hpp>
#include <jewel/optional.hpp>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

namespace sqloxx
{

/**
 * Provides an input iterator interface for traversing a SQLite database
 * table from which instances of some type T can be extracted by reference
 * to a column in the table.
 *
 * The template parameter T should be an instantiation of sqloxx::Handle<R>
 * for some R.
 *
 * The API is similar to \e std::istream_iterator. For example, suppose
 * \e Connection is some class derived from sqloxx::DatabaseConnection.
 *
 * <tt>
 *
 *  using sqloxx::Handle;
 * 	Connection dbc;
 * 	dbc.open("animals.db");
 * 	std::vector<Handle<Dog> > vec;
 * 	std::copy
 * 	(	TableIterator<Handle<Dog> >(dbc, "select dog_id from dogs"),
 * 		(TableIterator<Handle<Dog> >()),
 * 		std::back_inserter(vec)
 * 	);
 *
 * </tt>
 */
template <typename T>
class TableIterator:
	public std::iterator
	<	std::input_iterator_tag,
		T,
		ptrdiff_t,
		const T*,
		const T&
	>
{
public:

	typedef typename T::Connection Connection;

	/**
	 * Creates a "null" TableIterator. If we are using a "begin" and "end"
	 * pair of iterators to perform, say, some standard library algorithm,
	 * we can use a "null" TableIterator to serve as the "end". A
	 * TableIterator will become "null" once it has
	 * read all the records in the table.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 */
	TableIterator() = default;

	/**
	 * Creates a TableIterator that can traverse a table in the database
	 * \e p_connection, by stepping through the results obtained from
	 * executing an SQL statement constructed using \e p_statement_text.
	 * Note the default value for \e p_statement_text. If caller
	 * passes a custom statement to \e p_statement_text, it should be
	 * a SELECT statement, and only the first column of results will
	 * be relevant, where the first column should contain the primary
	 * key for T, of type sqloxx::Id. See class-level documentation of
	 * TableIterator for further requirements on T.
	 *
	 * When first constructed, the TableIterator will be "pointing" to
	 * the first object it reads from the database table; dereferencing
	 * it will yield a constant reference to that object. However, if
	 * there are no result rows to read, then the TableIterator
	 * will immediately be null (and deferencing it will yield undefined
	 * behaviour).
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
	 * Copy constructor.
	 *
	 * The newly created TableIterator will have its own instance
	 * of T copied from the one held in \e rhs, or else will be
	 * "null" (i.e. hold no instance of T) if rhs is null.
	 *
	 * If \e rhs is referencing a SQLStatement
	 * (i.e. was not created with the default constructor),
	 * then the newly created TableIterator will be referencing the
	 * same underlying SQLStatement. This means that if either
	 * TableIterator is incremented, then this will advance the
	 * implicit \e next position of \e both TableIterators in the result
	 * set. The lhs will continue to hold the same instance of T
	 * as when it was constructed, but when incremented next, it will
	 * go to whichever is the \e next position for the underlying
	 * SQLStatement. This seems odd but is exactly analogous to the
	 * behaviour of \e std::istream_iterator.
	 *
	 * Exceptions are the same as for the two-parameter constructor.
	 *
	 * Exception safety: <em>strong guarantee</em>, providing that the
	 * copy constructor for T offers at least the strong guarantee.
	 */
	TableIterator(TableIterator const&) = default;

	/**
	 * @todo HIGH PRIORITY Test, and document esp. re. exception-safety.
	 */
	TableIterator(TableIterator&&) = default;

	TableIterator& operator=(TableIterator const&) = delete;
	TableIterator& operator=(TableIterator&&) = delete;

	/**
	 * Note destructor is virtual, TableIterator should not be used as a
	 * polymorphic base class.
	 *
	 * Exception safety: will never throw, assuming the destructor for T will
	 * never throw.
	 */
	~TableIterator() = default;

	/**
	 * @returns a constant reference to the instance of T that is currently
	 * "pointed to" or "contained" within this TableIterator, i.e. the
	 * instance of T that is stored at the database table row that it
	 * is currently "at".
	 *
	 * Dereferencing a null iterator will result in undefined
	 * behaviour.
	 *
	 * Precondition: TableIterator must not be null. (A TableIterator
	 * is null if and only if it compares equal with a TableIterator
	 * constructed with the default constructor.)
	 *
	 * Exception safety: <em>nothrow guarantee</em>, providing precondition
	 * is met.
	 */
	T const& operator*() const;

	/**
	 * Use this to access a member of the instance of
	 * T that is currently "pointed to" or "contained" within the
	 * TableIterator.
	 *
	 * Dereferencing a null iterator will result in undefined
	 * behaviour.
	 *
	 * Precondition: TableIterator must not be null. (A TableIterator
	 * is null if and only if it compares equal with a TableIterator
	 * constructed with the default constructor.)
	 *
	 * Exception safety: <em>nothrow guarantee</em>, providing precondition
	 * is met.
	 */
	T const* operator->() const;

	/**
	 * Advance the TableIterator to the next row in the result set, and
	 * return a reference to the TableIterator at its new location.
	 *
	 * If the TableIterator is currently at the final row, then advancing
	 * it will result in the TableIterator becoming "null". Incrementing
	 * it \e again will reset it to the beginning of the result set
	 * again, providing there is at least one result row, in which
	 * case it will become non-null again.
	 *
	 * @throws SQLiteException or some exception deriving therefrom, if an
	 * error occurs while stepping to the next result row, that results in a
	 * SQLite error code.
	 * This function should almost never occur, but it is
	 * possible something will fail as the statement is being executed, in
	 * which case the resulting SQLite error condition will trigger the
	 * corresponding exception class.
	 *
	 * Might also throw any exception that might be thrown by the function
	 * <em>T::create_unchecked(Connection& T, sqloxx::Id)</em>, since this
	 * function is invoked to construct the iterator's internal instance
	 * of T from the next row of the SQL result set.
	 *
	 * If an exception is thrown, the TableIterator
	 * should not be used again, and neither should any other
	 * TableIterator referencing the same underlying SQLStatement.
	 * (Via application of the copy constructor, multiple
	 * TableIterators may reference the same SQLStatement.)
	 *
	 * Exception safety: <em>basic guarantee</em>, providing that the
	 * function <em>T::create_unchecked(Connection&, sqloxx::Id)</em> also
	 * offers at least the basic guarantee.
	 */
	TableIterator& operator++();

	/**
	 * Advance the TableIterator to the next row in the result set, but
	 * return a copy of the TableIterator at its previous location.
	 *
	 * The returned copy will contain the object pointed to by this
	 * TableIterator before it was advanced; however, the returned
	 * TableIterator will still be referencing the \e same underlying
	 * SQLStatement as the original iterator. Thus both iterators will
	 * share the same underlying result set, and the same
	 * \e position in that result set (which has now been advanced
	 * by one), which will be the starting position next time
	 * \e either iterator is called. This sounds odd but is exactly analogous
	 * to the behaviour of \e std::istream_iterator.
	 *
	 * Exceptions are the same as for the previous
	 * increment operator, however, this postfix operator, as well
	 * as calling <em>T::create_unchecked(Connection&, sqloxx::Id)</em>,
	 * also calls the copy constructor for T, so might also throw any
	 * exceptions that are thrown by that copy constructor.
	 *
	 * If an exception is thrown, the TableIterator
	 * should not be used again, and neither should any other
	 * TableIterator referencing the same underlying SQLStatement.
	 * (Via application of the copy constructor, multiple
	 * TableIterators may reference the same SQLStatement.)
	 *
	 * Exception safety: <em>basic guarantee</em>, providing that the
	 * function <em>T::create_unchecked(Connection&, sqloxx::Id)</em>, as
	 * well as the copy constructor for T, also offer the basic
	 * guarantee.
	 */
	TableIterator operator++(int);

	/**
	 * @returns \e true if and \e only if the TableIterators on \e both sides
	 * are "null". Thus this can be used to compare an existing TableIterator
	 * with a TableIterator known to be null (e.g. one constructed with the
	 * default constructor), to see if it has read the last object in the
	 * result set.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 */
	bool operator==(TableIterator const& rhs) const;

	/**
	 * @return \e true if and only if the two sides do not compare equal
	 * using the equality operator. See documentation for the equality
	 * operator.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 */
	bool operator!=(TableIterator const& rhs) const;

private:

	void advance();

	class Impl
	{
	public:
		Impl(Connection&, std::string const&);
		Impl(Impl const&) = delete;
		Impl(Impl&&) = delete;
		Impl& operator=(Impl const&) = delete;
		Impl& operator=(Impl&&) = delete;
		~Impl() = default;
		void next(boost::optional<T>& out);
	private:
		Connection& m_connection;
		SQLStatement m_sql_statement;

	};  // class Impl

	typedef std::shared_ptr<Impl> Pimpl;
	Pimpl mutable m_impl;
	boost::optional<T> m_maybe_object;

};  // class TableIterator


// IMPLEMENTATION

template <typename T>
inline
TableIterator<T>::TableIterator
(	Connection& p_connection,
	std::string const& p_statement_text
):
	m_impl(new Impl(p_connection, p_statement_text))
{
	JEWEL_LOG_TRACE();
	advance();
}

template <typename T>
inline
T const&
TableIterator<T>::operator*() const
{
	JEWEL_ASSERT (m_maybe_object);  // precondition
	return *m_maybe_object;
}

template <typename T>
inline
T const*
TableIterator<T>::operator->() const
{
	JEWEL_ASSERT (m_maybe_object);  // precondition
	return m_maybe_object.operator->();
}

template <typename T>
inline
TableIterator<T>&
TableIterator<T>::operator++()
{
	advance();
	return *this;
}

template <typename T>
TableIterator<T>
TableIterator<T>::operator++(int)
{
	TableIterator ret(*this);
	advance();
	return ret;
}

template <typename T>
inline
bool
TableIterator<T>::operator==(TableIterator const& rhs) const
{
	return !(*this != rhs);
}

template <typename T>
inline
bool
TableIterator<T>::operator!=(TableIterator const& rhs) const
{
	return m_maybe_object || rhs.m_maybe_object;
}

template <typename T>
inline
void
TableIterator<T>::advance()
{
	if (m_impl) m_impl->next(m_maybe_object);
	return;
}

template <typename T>
inline
TableIterator<T>::Impl::Impl
(	Connection& p_connection,
	std::string const& p_statement_text
):
	m_connection(p_connection),
	m_sql_statement(p_connection, p_statement_text)
{
}

template <typename T>
void
TableIterator<T>::Impl::next
(	boost::optional<T>& out
)
{
	if (m_sql_statement.step())
	{
		out = T::create_unchecked
		(	m_connection,
			m_sql_statement.template extract<Id>(0)
		);
	}
	else
	{
		out = boost::optional<T>();
	}
}



}  // namespace sqloxx

#endif  // GUARD_table_iterator_hpp_9025044682913015
