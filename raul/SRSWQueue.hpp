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

#ifndef RAUL_SRSW_QUEUE_HPP
#define RAUL_SRSW_QUEUE_HPP

#include <atomic>
#include <cassert>

#include "raul/Noncopyable.hpp"

namespace Raul {

/** Realtime-safe single-reader single-writer queue (aka lock-free ringbuffer)
 *
 * This is appropriate for a cross-thread queue of fixed size object.  If you
 * need to do variable sized reads and writes, use Raul::RingBuffer instead.
 *
 * Implemented as a dequeue in a fixed array.  This is read/write thread-safe,
 * pushing and popping may occur simultaneously by seperate threads, but
 * the push and pop operations themselves are not thread-safe (ie. there can
 * be at most 1 read and at most 1 writer thread).
 *
 * \ingroup raul
 */
template <typename T>
class SRSWQueue : Noncopyable
{
public:
	/** @param size Size in number of elements */
	explicit SRSWQueue(size_t size);
	~SRSWQueue();

	// Any thread:

	inline size_t capacity() const { return _size-1; }

	// Write thread(s):

	inline bool full() const;
	inline bool push(const T& obj);

	// Read thread:

	inline bool empty() const;
	inline T&   front() const;
	inline void pop();

private:
	std::atomic<size_t> _front; ///< Index to front of queue (circular)
	std::atomic<size_t> _back; ///< Index to back of queue (one past last element) (circular)
	const size_t _size; ///< Size of @ref _objects (you can store _size-1 objects)
	T* const     _objects; ///< Fixed array containing queued elements
};

template<typename T>
SRSWQueue<T>::SRSWQueue(size_t size)
	: _front(0)
	, _back(0)
	, _size(size + 1)
	, _objects(new T[_size])
{
	assert(size > 1);
}

template <typename T>
SRSWQueue<T>::~SRSWQueue()
{
	delete[] _objects;
}

/** Return whether or not the queue is empty.
 */
template <typename T>
inline bool
SRSWQueue<T>::empty() const
{
	return (_back.load() == _front.load());
}

/** Return whether or not the queue is full.
 */
template <typename T>
inline bool
SRSWQueue<T>::full() const
{
	return (((_front.load() - _back.load() + _size) % _size) == 1);
}

/** Return the element at the front of the queue without removing it
 */
template <typename T>
inline T&
SRSWQueue<T>::front() const
{
	return _objects[_front.load()];
}

/** Push an item onto the back of the SRSWQueue - realtime-safe, not thread-safe.
 *
 * @returns true if @a elem was successfully pushed onto the queue,
 * false otherwise (queue is full).
 */
template <typename T>
inline bool
SRSWQueue<T>::push(const T& elem)
{
	if (full()) {
		return false;
	} else {
		unsigned back = _back.load();
		_objects[back] = elem;
		_back = (back + 1) % _size;
		return true;
	}
}

/** Pop an item off the front of the queue - realtime-safe, not thread-safe.
 *
 * It is a fatal error to call pop() when the queue is empty.
 *
 * @returns the element popped.
 */
template <typename T>
inline void
SRSWQueue<T>::pop()
{
	assert(!empty());
	assert(_size > 0);

	_front = (_front.load() + 1) % (_size);
}

} // namespace Raul

#endif // RAUL_SRSW_QUEUE_HPP
