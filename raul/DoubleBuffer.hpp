/*
  This file is part of Raul.
  Copyright 2007-2013 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAUL_DOUBLE_BUFFER_HPP
#define RAUL_DOUBLE_BUFFER_HPP

#include <atomic>

namespace Raul {

/** Double buffer.
 *
 * Can be thought of as a wrapper class to make a non-atomic type atomically
 * settable (with no locking).
 *
 * Read/Write realtime safe, many writers safe - but set calls may fail.
 *
 * Space:  2*sizeof(T) + sizeof(int) + sizeof(void*)
 * \ingroup raul
 */
template<typename T>
class DoubleBuffer {
public:
	inline DoubleBuffer(T val)
		: _state(State::READ_WRITE)
	{
		_vals[0]  = val;
		_read_val = &_vals[0];
	}

	inline DoubleBuffer(const DoubleBuffer& copy)
		: _state(State::READ_WRITE)
	{
		_vals[0]  = copy.get();
		_read_val = &_vals[0];
	}

	inline T& get() const {
		return *_read_val.load();
	}

	inline bool set(T new_val) {
		State expected = State::READ_WRITE;
		if (_state.compare_exchange_strong(expected, State::READ_LOCK)) {
			// Locked _vals[1] for writing
			_vals[1]  = new_val;
			_read_val = &_vals[1];
			_state    = State::WRITE_READ;
			return true;
		}

		expected = State::WRITE_READ;
		if (_state.compare_exchange_strong(expected, State::LOCK_READ)) {
			// Locked _vals[0] for writing
			_vals[0]  = new_val;
			_read_val = &_vals[0];
			_state    = State::READ_WRITE;
			return true;
		}

		return false;
	}

private:
	enum class State {
		READ_WRITE,  ///< Read vals[0], Write vals[1]
		READ_LOCK,   ///< Read vals[0], Lock vals[1]
		WRITE_READ,  ///< Write vals[0], Write vals[1]
		LOCK_READ    ///< Lock vals[0], Read vals[1]
	};

	std::atomic<State> _state;
	std::atomic<T*>    _read_val;
	T                  _vals[2];
};

} // namespace Raul

#endif // RAUL_DOUBLE_BUFFER_HPP
