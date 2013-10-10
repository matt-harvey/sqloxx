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
	 * The Base is the type such that its primary key is
	 * \e ultimately stored in this table. Typically
	 * where we have a Base and a Derived class, both of
	 * which are PersistentObject instantiations, the Base
	 * class will be the Base of Derived. The primary
	 * key of Derived will be in a column in the "Base table"
	 * in the database. Then Derived may have its own table with
	 * a column that references the primary key column in the
	 * Base table. The Base class must have the following functions
	 * defined:
	 *
	 * <em>static std::string exclusive_table_name();</em>.
	 * This must return the table name of the table in which
	 * the primary key of T is ultimately stored.
	 *
	 * <em>static std::string primary_key_name();<em>
	 * This must return the name of the primary key for T as it
	 * appears in the table named by exclusive_table_name(). This
	 * must be a single-column integer primary key that is
	 * auto-incrementing (using the SQLite "autoincrement" keyword).
	 *
	 * @todo Improve this explanation.
	 */
	typedef T Base;

};  // class PersistenceTraits

}  // namespace sqloxx

#endif  // GUARD_persistence_traits_hpp_5478759168404508
