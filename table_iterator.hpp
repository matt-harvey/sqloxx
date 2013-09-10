#ifndef GUARD_table_iterator_hpp
#define GUARD_table_iterator_hpp

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
 * @todo HIGH PRIORITY Documentation.
 *
 * @todo HIGH PRIORITY Testing.
 */
template <typename T, typename Connection>
class TableIterator: private boost::noncopyable
{
public:
	TableIterator();
	TableIterator
	(	Connection& p_connection,
		std::string const& p_statement_text =
		(	"select " + T::primary_key_name() +
			" from " + T::exclusive_table_name()
		)
	);
	virtual ~TableIterator();
	T const& operator*() const;
	T const* operator->() const;
	TableIterator& operator++();
	bool operator==(TableIterator const& rhs);
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
		void advance_subsequent();

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
		m_sql_statement.template extract<typename T::Id>(0)
	);
}



}  // namespace sqloxx

#endif  // GUARD_table_iterator_hpp
