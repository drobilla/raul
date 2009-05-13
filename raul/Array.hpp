/* This file is part of Raul.
 * Copyright (C) 2007 Dave Robillard <http://drobilla.net>
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

#ifndef RAUL_ARRAY_HPP
#define RAUL_ARRAY_HPP

#include <cassert>
#include <cstdlib>
#include "Deletable.hpp"

namespace Raul {


/** An array.
 *
 * Has a stack-like push_back(), but is NOT a resizeable array (the size given
 * to the constructor or alloc method is the maximum number of elements which
 * can be pushed).
 */
template <class T>
class Array : public Deletable
{
public:
	Array(size_t size = 0) : _size(size), _top(0), _elems(NULL) {
		if (size > 0)
			_elems = new T[size];
	}

	Array(size_t size, T initial_value) : _size(size), _top(0), _elems(NULL) {
		if (size > 0) {
			_elems = new T[size];
			for (size_t i=0; i < size; ++i)
				_elems[i] = initial_value;
		}
	}

	Array(size_t size, const Array<T>& contents) : _size(size), _top(size+1) {
		_elems = new T[size];
		if (size <= contents.size())
			memcpy(_elems, contents._elems, size * sizeof(T));
		else
			memcpy(_elems, contents._elems, contents.size() * sizeof(T));
	}

	~Array() {
		delete[] _elems;
	}

	void alloc(size_t num_elems) {
		assert(num_elems > 0);

		delete[] _elems;
		_size = num_elems;
		_top = 0;

		_elems = new T[num_elems];
	}

	void alloc(size_t num_elems, T initial_value) {
		assert(num_elems > 0);

		delete[] _elems;
		_size = num_elems;
		_top = 0;

		_elems = new T[num_elems];
		for (size_t i=0; i < _size; ++i)
			_elems[i] = initial_value;
	}

	void push_back(T n) {
		assert(_top < _size);
		_elems[_top++] = n;
	}

	inline size_t size() const  { return _size; }

	inline T& operator[](size_t i) const { assert(i < _size); return _elems[i]; }

	inline T& at(size_t i) const { assert(i < _size); return _elems[i]; }

private:
	size_t _size;
	size_t _top; // points to empty element above "top" element
	T*     _elems;
};


} // namespace Raul

#endif // RAUL_ARRAY_HPP
