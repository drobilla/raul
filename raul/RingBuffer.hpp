/* This file is part of Raul.
 * Copyright (C) 2007-2009 David Robillard <http://drobilla.net>
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

#include <stdint.h>

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <glib.h>

#include "raul/log.hpp"

namespace Raul {


/** A lock-free RingBuffer.
 * Read/Write realtime safe.
 * Single-reader Single-writer thread safe.
 */
class RingBuffer {
public:
	/** @param size Size in bytes.
	 */
	explicit RingBuffer(uint32_t size)
		: _buf(static_cast<char*>(malloc(size)))
		, _size(size)
	{
		reset();
		assert(read_space() == 0);
		assert(write_space() == size - 1);
	}

	virtual ~RingBuffer() {
		free(_buf);
	}

	/** Reset(empty) the ringbuffer.
	 * NOT thread safe.
	 */
	void reset() {
		g_atomic_int_set(&_write_ptr, 0);
		g_atomic_int_set(&_read_ptr, 0);
	}

	uint32_t write_space() const {
		const uint32_t w = g_atomic_int_get(&_write_ptr);
		const uint32_t r = g_atomic_int_get(&_read_ptr);

		if (w > r) {
			return ((r - w + _size) % _size) - 1;
		} else if (w < r) {
			return (r - w) - 1;
		} else {
			return _size - 1;
		}
	}

	uint32_t read_space() const {
		const uint32_t w = g_atomic_int_get(&_write_ptr);
		const uint32_t r = g_atomic_int_get(&_read_ptr);

		if (w > r) {
			return w - r;
		} else {
			return (w - r + _size) % _size;
		}
	}

	uint32_t capacity() const { return _size; }

	uint32_t peek(uint32_t size, void* dst);
	bool     full_peek(uint32_t size, void* dst);

	uint32_t read(uint32_t size, void* dst);
	bool     full_read(uint32_t size, void* dst);

	bool     skip(uint32_t size);

	void     write(uint32_t size, const void* src);

protected:
	mutable uint32_t _write_ptr;
	mutable uint32_t _read_ptr;

	char*    _buf;  ///< Contents
	uint32_t _size; ///< Size (capacity) in bytes
};


/** Peek at the ringbuffer (read w/o advancing read pointer).
 *
 * Note that a full read may not be done if the data wraps around.
 * Caller must check return value and call again if necessary, or use the
 * full_peek method which does this automatically.
 */
inline uint32_t
RingBuffer::peek(uint32_t size, void* dst)
{
	const uint32_t priv_read_ptr = g_atomic_int_get(&_read_ptr);

	const uint32_t read_size = (priv_read_ptr + size < _size)
			? size
			: _size - priv_read_ptr;

	memcpy(dst, &_buf[priv_read_ptr], read_size);

	return read_size;
}


inline bool
RingBuffer::full_peek(uint32_t size, void* dst)
{
	if (read_space() < size) {
		return false;
	}

	const uint32_t read_size = peek(size, dst);

	if (read_size < size) {
		peek(size - read_size, (char*)dst + read_size);
	}

	return true;
}


/** Read from the ringbuffer.
 *
 * Note that a full read may not be done if the data wraps around.
 * Caller must check return value and call again if necessary, or use the
 * full_read method which does this automatically.
 */
inline uint32_t
RingBuffer::read(uint32_t size, void* dst)
{
	const uint32_t priv_read_ptr = g_atomic_int_get(&_read_ptr);

	const uint32_t read_size = (priv_read_ptr + size < _size)
			? size
			: _size - priv_read_ptr;

	memcpy(dst, &_buf[priv_read_ptr], read_size);

	g_atomic_int_set(&_read_ptr, (priv_read_ptr + read_size) % _size);

	return read_size;
}


inline bool
RingBuffer::full_read(uint32_t size, void* dst)
{
	if (read_space() < size) {
		return false;
	}

	const uint32_t read_size = read(size, dst);

	if (read_size < size) {
		read(size - read_size, (char*)dst + read_size);
	}

	return true;
}


inline bool
RingBuffer::skip(uint32_t size)
{
	if (read_space() < size) {
		warn << "Attempt to skip past end of RingBuffer" << std::endl;
		return false;
	}

	const uint32_t priv_read_ptr = g_atomic_int_get(&_read_ptr);
	g_atomic_int_set(&_read_ptr, (priv_read_ptr + size) % _size);

	return true;
}


inline void
RingBuffer::write(uint32_t size, const void* src)
{
	const uint32_t priv_write_ptr = g_atomic_int_get(&_write_ptr);

	if (priv_write_ptr + size <= _size) {
		memcpy(&_buf[priv_write_ptr], src, size);
		g_atomic_int_set(&_write_ptr, (priv_write_ptr + size) % _size);
	} else {
		const uint32_t this_size = _size - priv_write_ptr;
		assert(this_size < size);
		assert(priv_write_ptr + this_size <= _size);
		memcpy(&_buf[priv_write_ptr], src, this_size);
		memcpy(&_buf[0], (char*)src + this_size, size - this_size);
		g_atomic_int_set(&_write_ptr, size - this_size);
	}
}


} // namespace Raul

#endif // RAUL_RING_BUFFER_HPP

