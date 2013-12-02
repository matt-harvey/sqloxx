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

#ifndef GUARD_persistence_traits_hpp_5478759168404508
#define GUARD_persistence_traits_hpp_5478759168404508

namespace sqloxx
{

/**
 * Houses traits relevant for PersistentObject. Where you have
 * a particular instantiation of <b>PersistentObject<T, Connection></b>,
 * specialize this template if you want certain non-default behaviour.
 */
template <typename T>
struct PersistenceTraits
{
	/**
	 * \b Base is the type such that the primary key of
	 * \b T is "ultimately" stored in the table given by
	 * \b Base::exclusive_table_name(). That table is the
	 * table that maintains the incrementing primary key
	 * sequence used both for instances of \b T and for instances of \b Base.
	 *
	 * Usually, \b T and \b Base are one and the same class.
	 * But in some cases, client code might contain a
	 * hierarchy such that one class (\b "Super") inherits directly
	 * from <b>PersistentObject<Super, Connection></b>, and then
	 * another class (\b "Sub") inherits, in turn, from \b Super.
	 * Typically the "base table" both for \b Super and \b Sub will
	 * be the table given by \b Super::exclusive_table_name().
	 * In that case, \b PersistenceTraits<T> should be specialized such that
	 * \b PersistenceTraits<T>::Base is a typedef for \b Super.
	 *
	 * The \b Base class must have the following functions
	 * defined:
	 *
	 * <em>static std::string \b exclusive_table_name();</em>.
	 * This must return the name of the table in which
	 * the primary key of \b T is ultimately stored.
	 *
	 * <em>static std::string \b primary_key_name();</em>
	 * This must return the name of the primary key for \b T as it
	 * appears in the table named by \b Base::exclusive_table_name().
	 * This must be a single-column integer primary key that is
	 * auto-incrementing (using the SQLite "autoincrement" keyword).
	 *
	 * For an example of how this all works, see the classes \b ExampleB
	 * and \b ExampleC in "sqloxx/tests/example.hpp" and
	 * "sqloxx/tests/example.cpp". Here \b ExampleB is the \b Base class for
	 * \b ExampleC, and this is reflected in the specialization for
	 * \b PersistenceTraits<ExampleC> contained in "example.hpp".
	 */
	typedef T Base;

};  // class PersistenceTraits

}  // namespace sqloxx

#endif  // GUARD_persistence_traits_hpp_5478759168404508
