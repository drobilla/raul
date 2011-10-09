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

#ifndef RAUL_RING_BUFFER_HPP
#define RAUL_RING_BUFFER_HPP

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "raul/Noncopyable.hpp"

namespace Raul {

/**
   A lock-free RingBuffer.

   Thread-safe with a single reader and single writer, and realtime safe
   on both ends.

   @ingroup raul
*/
class RingBuffer : public Noncopyable {
public:
	/**
	   Create a new RingBuffer.
	   @param size Size in bytes (note this may be rounded up).
	 */
	explicit RingBuffer(uint32_t size)
		: _size(next_power_of_two(size))
		, _size_mask(_size - 1)
		, _buf(static_cast<char*>(malloc(_size)))
	{
		reset();
		assert(read_space() == 0);
		assert(write_space() == _size - 1);
	}

	/**
	   Destroy a RingBuffer.
	*/
	inline ~RingBuffer() {
		free(_buf);
	}

	/**
	   Reset (empty) the RingBuffer.

	   This method is NOT thread-safe, it may only be called when there are no
	   readers or writers.
	*/
	inline void reset() {
		g_atomic_int_set(&_write_ptr, 0);
		g_atomic_int_set(&_read_ptr, 0);
	}

	/**
	   Return the number of bytes of space available for reading.
	*/
	inline uint32_t read_space() const {
		const uint32_t r = g_atomic_int_get(&_read_ptr);
		const uint32_t w = g_atomic_int_get(&_write_ptr);
		return read_space_internal(r, w);
	}

	/**
	   Return the number of bytes of space available for writing.
	*/
	inline uint32_t write_space() const {
		const uint32_t r = g_atomic_int_get(&_read_ptr);
		const uint32_t w = g_atomic_int_get(&_write_ptr);
		return write_space_internal(r, w);
	}

	/**
	   Return the capacity (i.e. total write space when empty).
	*/
	inline uint32_t capacity() const { return _size - 1; }

	/**
	   Read from the RingBuffer without advancing the read head.
	*/
	inline uint32_t peek(uint32_t size, void* dst) {
		const uint32_t r = g_atomic_int_get(&_read_ptr);
		const uint32_t w = g_atomic_int_get(&_write_ptr);
		return peek_internal(r, w, size, dst);
	}

	/**
	   Read from the RingBuffer and advance the read head.
	*/
	inline uint32_t read(uint32_t size, void* dst) {
		const uint32_t r = g_atomic_int_get(&_read_ptr);
		const uint32_t w = g_atomic_int_get(&_write_ptr);

		if (peek_internal(r, w, size, dst)) {
			g_atomic_int_set(&_read_ptr, (r + size) & _size_mask);
			return size;
		} else {
			return 0;
		}
	}

	/**
	   Skip data in the RingBuffer (advance read head without reading).
	*/
	inline uint32_t skip(uint32_t size) {
		const uint32_t r = g_atomic_int_get(&_read_ptr);
		const uint32_t w = g_atomic_int_get(&_write_ptr);
		if (read_space_internal(r, w) < size) {
			return 0;
		}

		g_atomic_int_set(&_read_ptr, (r + size) & _size_mask);
		return size;
	}

	/**
	   Write data to the RingBuffer.
	*/
	inline uint32_t write(uint32_t size, const void* src) {
		const uint32_t r = g_atomic_int_get(&_read_ptr);
		const uint32_t w = g_atomic_int_get(&_write_ptr);
		if (write_space_internal(r, w) < size) {
			return 0;
		}

		if (w + size <= _size) {
			memcpy(&_buf[w], src, size);
			g_atomic_int_set(&_write_ptr, (w + size) & _size_mask);
		} else {
			const uint32_t this_size = _size - w;
			assert(this_size < size);
			assert(w + this_size <= _size);
			memcpy(&_buf[w], src, this_size);
			memcpy(&_buf[0], (char*)src + this_size, size - this_size);
			g_atomic_int_set(&_write_ptr, size - this_size);
		}

		return size;
	}

private:
	static inline uint32_t next_power_of_two(uint32_t size) {
		// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		size--;
		size |= size >> 1;
		size |= size >> 2;
		size |= size >> 4;
		size |= size >> 8;
		size |= size >> 16;
		size++;
		return size;
	}

	inline uint32_t write_space_internal(uint32_t r, uint32_t w) const {
		if (w > r) {
			return ((r - w + _size) & _size_mask) - 1;
		} else if (w < r) {
			return (r - w) - 1;
		} else {
			return _size - 1;
		}
	}

	inline uint32_t read_space_internal(uint32_t r, uint32_t w) const {
		if (w > r) {
			return w - r;
		} else {
			return (w - r + _size) & _size_mask;
		}
	}

	inline uint32_t peek_internal(uint32_t r, uint32_t w,
	                              uint32_t size, void* dst) const {
		if (read_space_internal(r, w) < size) {
			return 0;
		}

		if (r + size < _size) {
			memcpy(dst, &_buf[r], size);
		} else {
			const uint32_t first_size = _size - r;
			memcpy(dst, &_buf[r], first_size);
			memcpy((char*)dst + first_size, &_buf[0], size - first_size);
		}

		return size;
	}

	mutable uint32_t _write_ptr;  ///< Read index into _buf
	mutable uint32_t _read_ptr;   ///< Write index into _buf

	const uint32_t _size;       ///< Size (capacity) in bytes
	const uint32_t _size_mask;  ///< Mask for fast modulo

	char* const _buf;  ///< Contents
};

} // namespace Raul

#endif // RAUL_RING_BUFFER_HPP
