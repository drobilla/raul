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

#ifdef RAUL_CPP0x

#include <atomic>

/** Atomic integer.
 * \ingroup raul
 */
class AtomicInt {
public:
	inline AtomicInt(int val) : _val(val) {}

	inline AtomicInt(const AtomicInt& copy) : _val(copy.get()) {}

	inline int get() const { return _val.load(); }

	inline void operator=(int val) { _val = val; }
	inline void operator=(const AtomicInt& val) { _val = val.get(); }

	inline void operator+=(int val) { _val += val; }

	inline void operator-=(int val) { _val -= val; }

	inline bool operator==(int val) const { return _val == val; }

	inline int operator+(int val) const { return _val + val; }

	inline AtomicInt& operator++() { ++_val; return *this; }

	inline AtomicInt& operator--() { --_val; return *this; }

	/** Set value to @a val iff current value is @a old.
	 * @return true iff set succeeded.
	 */
	inline bool compare_and_exchange(int old, int val) {
		return _val.compare_exchange_strong(old, val);
	}

	/** Add val to value.
	 * @return value immediately before addition took place.
	 */
	inline int exchange_and_add(int val) {
		return _val.fetch_add(val);
	}

	/** Decrement value.
	 * @return true if value is now 0, otherwise false.
	 */
	inline bool decrement_and_test() {
		return _val.fetch_add(-1) == 0;
	}

private:
	std::atomic<int> _val;
};

#else // !RAUL_CPP0x

#include <glib.h>

namespace Raul {

/** Atomic integer.
 * \ingroup raul
 */
class AtomicInt {
public:
	inline AtomicInt(int val)
		{ g_atomic_int_set(static_cast<volatile gint*>(&_val), val); }

	inline AtomicInt(const AtomicInt& copy)
		{ g_atomic_int_set(static_cast<volatile gint*>(&_val), copy.get()); }

	inline int get() const
		{ return g_atomic_int_get(static_cast<volatile gint*>(&_val)); }

	inline void operator=(int val)
		{ g_atomic_int_set(static_cast<volatile gint*>(&_val), val); }

	inline void operator+=(int val)
		{ g_atomic_int_add(static_cast<volatile gint*>(&_val), val); }

	inline void operator-=(int val)
		{ g_atomic_int_add(static_cast<volatile gint*>(&_val), -val); }

	inline bool operator==(int val) const
		{ return get() == val; }

	inline int operator+(int val) const
		{ return get() + val; }

	inline AtomicInt& operator++() // prefix
		{ g_atomic_int_inc(static_cast<volatile gint*>(&_val)); return *this; }

	inline AtomicInt& operator--() // prefix
		{ g_atomic_int_add(static_cast<volatile gint*>(&_val), -1); return *this; }

	/** Set value to @a val iff current value is @a old.
	 * @return true iff set succeeded.
	 */
	inline bool compare_and_exchange(int old, int val)
		{ return g_atomic_int_compare_and_exchange(static_cast<volatile gint*>(&_val), old, val); }

	/** Add val to value.
	 * @return value immediately before addition took place.
	 */
	inline int exchange_and_add(int val) {
#if GLIB_CHECK_VERSION(2, 30, 0)
		return g_atomic_int_add(static_cast<volatile gint*>(&_val), val);
#else
		return g_atomic_int_exchange_and_add(static_cast<volatile gint*>(&_val), val);
#endif
	}

	/** Decrement value.
	 * @return true if value is now 0, otherwise false.
	 */
	inline bool decrement_and_test()
		{ return g_atomic_int_dec_and_test(static_cast<volatile gint*>(&_val)); }

private:
	volatile mutable int _val;
};

} // namespace Raul

#endif // RAUL_CPP0x

#endif // RAUL_ATOMIC_INT_HPP
