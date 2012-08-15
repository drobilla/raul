/*
  This file is part of Raul.
  Copyright 2007-2012 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAUL_ARRAY_HPP
#define RAUL_ARRAY_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>

#include "raul/Disposable.hpp"

namespace Raul {

/** An array.
 *
 * Has a stack-like push_back(), but is NOT a resizeable array (the size given
 * to the constructor or alloc method is the maximum number of elements which
 * can be pushed).
 * \ingroup raul
 */
template <class T>
class Array : public Disposable
{
public:
	explicit Array(size_t size = 0) : _size(size), _elems(NULL) {
		if (size > 0)
			_elems = new T[size];
	}

	Array(size_t size, T initial_value) : _size(size), _elems(NULL) {
		if (size > 0) {
			_elems = new T[size];
			for (size_t i = 0; i < size; ++i)
				_elems[i] = initial_value;
		}
	}

	Array(size_t size, const Array<T>& contents) : _size(size) {
		assert(contents.size() >= size);
		_elems = new T[size];
		for (size_t i = 0; i < std::min(size, contents.size()); ++i)
			_elems[i] = contents[i];
	}

	Array(size_t size, const Array<T>& contents, T initial_value=T()) : _size(size) {
		_elems = new T[size];
		const size_t end = std::min(size, contents.size());
		for (size_t i = 0; i < end; ++i)
			_elems[i] = contents[i];
		for (size_t i = end; i < size; ++i)
			_elems[i] = initial_value;
	}

	~Array() {
		delete[] _elems;
	}

	virtual void alloc(size_t num_elems) {
		assert(num_elems > 0);

		delete[] _elems;
		_size = num_elems;

		_elems = new T[num_elems];
	}

	virtual void alloc(size_t num_elems, T initial_value) {
		assert(num_elems > 0);

		delete[] _elems;
		_size = num_elems;

		_elems = new T[num_elems];
		for (size_t i = 0; i < _size; ++i)
			_elems[i] = initial_value;
	}

	inline size_t size() const  { return _size; }

	inline T& operator[](size_t i) const { assert(i < _size); return _elems[i]; }

	inline T& at(size_t i) const { assert(i < _size); return _elems[i]; }

private:
	size_t _size;
	T*     _elems;
};

} // namespace Raul

#endif // RAUL_ARRAY_HPP
