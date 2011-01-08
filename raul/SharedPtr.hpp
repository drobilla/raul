/* A Reference Counting Smart Pointer.
 * Copyright (C) 2007-2009 David Robillard <http://drobilla.net>
 *
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RAUL_SHARED_PTR_HPP
#define RAUL_SHARED_PTR_HPP

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <set>

static std::set<void*> shared_ptr_counters;

// Use debug hooks to ensure 2 shared_ptrs never point to the same thing
namespace boost {
	inline void sp_scalar_constructor_hook(void* px, std::size_t size, void* pn) {
		assert(shared_ptr_counters.find(px) == shared_ptr_counters.end());
		shared_ptr_counters.push_back(px);
	}

	inline void sp_scalar_destructor_hook(void* px, std::size_t size, void* pn) {
		shared_ptr_counters.remove(px);
	}
}
#endif // BOOST_SP_ENABLE_DEBUG_HOOKS


#include <boost/shared_ptr.hpp>

#ifdef BOOST_AC_USE_PTHREADS
#error "Boost is using mutexes for shared_ptr reference counting."
#error "This is VERY slow.  Please report your platform to d@drobilla.net"
#endif

template <typename T> void NullDeleter(T* ptr) {}

#define SharedPtr boost::shared_ptr
#define PtrCast   boost::dynamic_pointer_cast

#endif // RAUL_SHARED_PTR_HPP

