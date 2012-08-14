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

#ifndef RAUL_ATOMIC_PTR_HPP
#define RAUL_ATOMIC_PTR_HPP

#include <cstddef>

#include "raul/barrier.hpp"

namespace Raul {

/** Atomic pointer.
 * \ingroup raul
 */
template<typename T>
class AtomicPtr {
public:
	inline AtomicPtr() : _val(NULL) {}

	inline AtomicPtr(const AtomicPtr& copy) : _val(copy.get()) {}

	inline T* get() const {
		Raul::barrier();
		return static_cast<T*>(_val);
	}

	inline bool operator==(const AtomicPtr& other) const {
		Raul::barrier();
		return _val == other._val;
	}

	inline void operator=(T* val) {
		_val = val;
		Raul::barrier();
	}

	/** Set value to @a val iff current value is @a old.
	 * @return true iff set succeeded.
	 */
	inline bool compare_and_exchange(void* old, void* val) {
		return __sync_bool_compare_and_swap(&_val, old, val);
	}

private:
	void* _val;
};

} // namespace Raul

#endif // RAUL_ATOMIC_PTR_HPP
