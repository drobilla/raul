/*
  Copyright 2007-2013 David Robillard <d@drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAUL_DOUBLEBUFFER_HPP
#define RAUL_DOUBLEBUFFER_HPP

#include <atomic>
#include <utility>

namespace raul {

/**
   Double buffer.

   Can be thought of as a wrapper class to make a non-atomic type atomically
   settable (with no locking).

   Read/Write realtime safe, many writers safe - but set calls may fail.

   Space:  2*sizeof(T) + sizeof(int) + sizeof(void*)

   @ingroup raul
*/
template<typename T>
class DoubleBuffer
{
public:
  explicit DoubleBuffer(T val)
    : _state(State::READ_WRITE)
  {
    _vals[0] = std::move(val);
  }

  DoubleBuffer(const DoubleBuffer&) = delete;
  DoubleBuffer& operator=(const DoubleBuffer&) = delete;

  DoubleBuffer(DoubleBuffer&&) = delete;
  DoubleBuffer& operator=(DoubleBuffer&&) = delete;

  ~DoubleBuffer() = default;

  inline const T& get() const
  {
    switch (_state.load(std::memory_order_acquire)) {
    case State::READ_WRITE:
    case State::READ_LOCK:
      return _vals[0];
    case State::WRITE_READ:
    case State::LOCK_READ:
      break;
    }
    return _vals[1];
  }

  inline bool set(T new_val)
  {
    if (transition(State::READ_WRITE, State::READ_LOCK)) {
      // Locked _vals[1] for writing
      _vals[1] = std::move(new_val);
      _state.store(State::WRITE_READ, std::memory_order_release);
      return true;
    }

    if (transition(State::WRITE_READ, State::LOCK_READ)) {
      // Locked _vals[0] for writing
      _vals[0] = std::move(new_val);
      _state.store(State::READ_WRITE, std::memory_order_release);
      return true;
    }

    return false;
  }

private:
  enum class State {
    READ_WRITE, ///< Read vals[0], Write vals[1]
    READ_LOCK,  ///< Read vals[0], Lock vals[1]
    WRITE_READ, ///< Write vals[0], Read vals[1]
    LOCK_READ   ///< Lock vals[0], Read vals[1]
  };

  bool transition(State from, const State to)
  {
    return _state.compare_exchange_strong(
      from, to, std::memory_order_release, std::memory_order_relaxed);
  }

  std::atomic<State> _state;
  T                  _vals[2];
};

} // namespace raul

#endif // RAUL_DOUBLEBUFFER_HPP
