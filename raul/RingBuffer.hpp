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

#ifndef RAUL_RING_BUFFER_HPP
#define RAUL_RING_BUFFER_HPP

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "raul/Noncopyable.hpp"
#include "raul/barrier.hpp"

namespace Raul {

/**
   A lock-free RingBuffer.

   Thread-safe with a single reader and single writer, and real-time safe
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
		_write_head = 0;
		_read_head  = 0;
	}

	/**
	   Return the number of bytes of space available for reading.
	*/
	inline uint32_t read_space() const {
		return read_space_internal(_read_head, _write_head);
	}

	/**
	   Return the number of bytes of space available for writing.
	*/
	inline uint32_t write_space() const {
		return write_space_internal(_read_head, _write_head);
	}

	/**
	   Return the capacity (i.e. total write space when empty).
	*/
	inline uint32_t capacity() const { return _size - 1; }

	/**
	   Read from the RingBuffer without advancing the read head.
	*/
	inline uint32_t peek(uint32_t size, void* dst) {
		return peek_internal(_read_head, _write_head, size, dst);
	}

	/**
	   Read from the RingBuffer and advance the read head.
	*/
	inline uint32_t read(uint32_t size, void* dst) {
		const uint32_t r = _read_head;
		const uint32_t w = _write_head;

		if (peek_internal(r, w, size, dst)) {
			Raul::barrier();
			_read_head = (r + size) & _size_mask;
			return size;
		} else {
			return 0;
		}
	}

	/**
	   Skip data in the RingBuffer (advance read head without reading).
	*/
	inline uint32_t skip(uint32_t size) {
		const uint32_t r = _read_head;
		const uint32_t w = _write_head;
		if (read_space_internal(r, w) < size) {
			return 0;
		}

		Raul::barrier();
		_read_head = (r + size) & _size_mask;
		return size;
	}

	/**
	   Write data to the RingBuffer.
	*/
	inline uint32_t write(uint32_t size, const void* src) {
		const uint32_t r = _read_head;
		const uint32_t w = _write_head;
		if (write_space_internal(r, w) < size) {
			return 0;
		}

		if (w + size <= _size) {
			memcpy(&_buf[w], src, size);
			Raul::barrier();
			_write_head = (w + size) & _size_mask;
		} else {
			const uint32_t this_size = _size - w;
			assert(this_size < size);
			assert(w + this_size <= _size);
			memcpy(&_buf[w], src, this_size);
			memcpy(&_buf[0], (const char*)src + this_size, size - this_size);
			Raul::barrier();
			_write_head = size - this_size;
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
		if (r == w) {
			return _size - 1;
		} else if (r < w) {
			return ((r - w + _size) & _size_mask) - 1;
		} else {
			return (r - w) - 1;
		}
	}

	inline uint32_t read_space_internal(uint32_t r, uint32_t w) const {
		if (r < w) {
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

	mutable uint32_t _write_head;  ///< Read index into _buf
	mutable uint32_t _read_head;   ///< Write index into _buf

	const uint32_t _size;       ///< Size (capacity) in bytes
	const uint32_t _size_mask;  ///< Mask for fast modulo

	char* const _buf;  ///< Contents
};

} // namespace Raul

#endif // RAUL_RING_BUFFER_HPP
