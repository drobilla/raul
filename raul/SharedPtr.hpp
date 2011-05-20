/* A Reference Counting Smart Pointer.
 * Copyright 2007-2011 David Robillard <http://drobilla.net>
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

template <typename T> void NullDeleter(T* ptr) {}

#ifdef RAUL_CPP0x
#    include <memory>
#    define SharedPtr std::shared_ptr
#    define PtrCast   std::dynamic_pointer_cast
#else
#    include <boost/shared_ptr.hpp>
#    ifdef BOOST_AC_USE_PTHREADS
#        error "Boost is using mutexes for shared_ptr reference counting."
#        error "This is VERY slow.  Please report your platform to d@drobilla.net"
#    endif
#    define SharedPtr boost::shared_ptr
#    define PtrCast   boost::dynamic_pointer_cast
#endif

#endif // RAUL_SHARED_PTR_HPP

