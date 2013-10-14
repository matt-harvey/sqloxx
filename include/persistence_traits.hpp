/*
 * This file is part of the Sqloxx project and is distributed under the
 * terms of the license contained in the file LICENSE.txt distributed
 * with this package.
 * 
 * Author: Matthew Harvey <matthew@matthewharvey.net>
 *
 * Copyright (c) 2012-2013, Matthew Harvey.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
	 * The Base is the type such that the primary key of
	 * T is "ultimately" stored in the table given by
	 * Base::exclusive_table_name(). That table is the
	 * table that maintains the incrementing primary key
	 * sequence used both for instances of T and for instances of Base.
	 *
	 * Usually, T and Base are one and the same class.
	 * But in some cases, client code might contain a
	 * hierarchy such that one class ("Super") inherits directly
	 * from PersistentObject<Super, Connection>, and then
	 * another class ("Sub") inherits, in turn, from Super.
	 * Typically the "base table" both for Super and Sub will
	 * be the table given by Super::exclusive_table_name().
	 * In that case, PersistenceTraits<T> should be specialized such that
	 * PersistenceTraits<T>::Base is a typedef for Super.
	 *
	 * The Base class must have the following functions
	 * defined:
	 *
	 * <em>static std::string exclusive_table_name();</em>.
	 * This must return the name of the table in which
	 * the primary key of T is ultimately stored.
	 *
	 * <em>static std::string primary_key_name();<em>
	 * This must return the name of the primary key for T as it
	 * appears in the table named by Base::exclusive_table_name(). This
	 * must be a single-column integer primary key that is
	 * auto-incrementing (using the SQLite "autoincrement" keyword).
	 *
	 * @todo MEDIUM PRIORITY There should be an example, at least in
	 * the test code, of how this all works.
	 */
	typedef T Base;

};  // class PersistenceTraits

}  // namespace sqloxx

#endif  // GUARD_persistence_traits_hpp_5478759168404508
