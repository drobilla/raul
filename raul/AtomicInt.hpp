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

#ifndef RAUL_ATOMIC_INT_HPP
#define RAUL_ATOMIC_INT_HPP

#include "raul/barrier.hpp"

namespace Raul {

/** Atomic integer.
 * \ingroup raul
 */
class AtomicInt {
public:
	inline AtomicInt(int val) : _val(val) {}

	inline AtomicInt(const AtomicInt& copy) : _val(copy.get()) {}

	inline int get() const {
		Raul::barrier();
		return _val;
	}

	inline void operator=(int val) {
		_val = val;
		Raul::barrier();
	}

	inline void operator=(const AtomicInt& val) {
		_val = val.get();
		Raul::barrier();
	}

	inline AtomicInt& operator++()              { return operator+=(1); }
	inline AtomicInt& operator--()              { return operator-=(1); }
	inline bool       operator==(int val) const { return get() == val; }
	inline int        operator+(int val)  const { return get() + val; }
	inline int        operator-(int val)  const { return get() - val; }

	inline bool operator==(const AtomicInt& other) const {
		Raul::barrier();
		return _val == other._val;
	}

	inline AtomicInt& operator+=(int val) {
		__sync_fetch_and_add(&_val, val);
		return *this;
	}

	inline AtomicInt& operator-=(int val) {
		__sync_fetch_and_sub(&_val, val);
		return *this;
	}

	/** Set value to @a val iff current value is @a old.
	 * @return true iff set succeeded.
	 */
	inline bool compare_and_exchange(int old, int val) {
		return __sync_bool_compare_and_swap(&_val, old, val);
	}

	/** Add val to value.
	 * @return value immediately before addition took place.
	 */
	inline int exchange_and_add(int val) {
		return __sync_fetch_and_add(&_val, val);
	}

	/** Decrement value.
	 * @return true if value is now 0, otherwise false.
	 */
	inline bool decrement_and_test() {
		return __sync_sub_and_fetch(&_val, 1) == 0;
	}

private:
	int _val;
};

} // namespace Raul

#endif // RAUL_ATOMIC_INT_HPP
