/* This file is part of Raul.
 * Copyright 2007-2011 David Robillard <http://drobilla.net>
 *
 * Raul is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Raul is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RAUL_ATOMIC_PTR_HPP
#define RAUL_ATOMIC_PTR_HPP

#include <glib.h>

namespace Raul {


/** Atomic pointer.
 * \ingroup raul
 */
template<typename T>
class AtomicPtr {
public:
	inline AtomicPtr()
		{ g_atomic_pointer_set(static_cast<volatile gpointer*>(&_val), NULL); }

	inline AtomicPtr(const AtomicPtr& copy)
		{ g_atomic_pointer_set(static_cast<volatile gpointer*>(&_val),
				static_cast<gpointer>(copy.get())); }

	inline T* get() const
		{ return static_cast<T*>(g_atomic_pointer_get(static_cast<volatile gpointer*>(&_val))); }

	inline void operator=(T* val)
		{ g_atomic_pointer_set(&_val, static_cast<gpointer>(val)); }

	/** Set value to newval iff current value is oldval */
	inline bool compare_and_exchange(gpointer oldval, gpointer newval)
		{ return g_atomic_pointer_compare_and_exchange(&_val, oldval, newval); }

private:
	mutable volatile gpointer _val;
};


} // namespace Raul

#endif // RAUL_ATOMIC_PTR_HPP
