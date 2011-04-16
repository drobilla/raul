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

#ifndef RAUL_ARRAYSTACK_HPP
#define RAUL_ARRAYSTACK_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>

#include "raul/Array.hpp"
#include "raul/Deletable.hpp"

namespace Raul {


/** An array that can also be used as a stack (with a fixed maximum size).
 * \ingroup raul
 */
template <class T>
class ArrayStack : public Array<T>
{
public:
	explicit ArrayStack(size_t size = 0) : Array<T>(size), _top(0) {}

	ArrayStack(size_t size, T initial_value) : Array<T>(size, initial_value), _top(0) {}

	ArrayStack(size_t size, const Array<T>& contents) : Array<T>(size, contents), _top(size + 1) {}

	~Array() {
		delete[] _elems;
	}

	void alloc(size_t num_elems) {
		Array<T>::alloc(num_elems);
		_top = 0;
	}

	void alloc(size_t num_elems, T initial_value) {
		Array<T>::alloc(num_elems, initial_value);
		_top = 0;
	}

	void push_back(T n) {
		assert(_top < _size);
		_elems[_top++] = n;
	}

	inline size_t size() const  { return _size; }

	inline T& operator[](size_t i) const { assert(i < _size); return _elems[i]; }

	inline T& at(size_t i) const { assert(i < _size); return _elems[i]; }

private:
	size_t _top; // Index of empty element following the top element
};


} // namespace Raul

#endif // RAUL_ARRAY_HPP
