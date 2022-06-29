// Copyright 2007-2012 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAUL_RINGBUFFER_HPP
#define RAUL_RINGBUFFER_HPP

#include "raul/Noncopyable.hpp"

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>

namespace raul {

/**
   A lock-free RingBuffer.

   Thread-safe with a single reader and single writer, and real-time safe
   on both ends.

   @ingroup raul
*/
class RingBuffer : public Noncopyable
{
public:
  /**
     Create a new RingBuffer.

     @param size Size in bytes (note this may be rounded up).
  */
  explicit RingBuffer(uint32_t size)
    : _write_head(0)
    , _read_head(0)
    , _size(next_power_of_two(size))
    , _size_mask(_size - 1)
    , _buf(new char[_size])
  {
    assert(read_space() == 0);
    assert(write_space() == _size - 1);
  }

  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  RingBuffer(RingBuffer&&) = delete;
  RingBuffer& operator=(RingBuffer&&) = delete;

  ~RingBuffer() = default;

  /**
     Reset (empty) the RingBuffer.

     This method is NOT thread-safe, it may only be called when there are no
     readers or writers.
  */
  inline void reset()
  {
    _write_head = 0;
    _read_head  = 0;
  }

  /// Return the number of bytes of space available for reading
  inline uint32_t read_space() const
  {
    return read_space_internal(_read_head, _write_head);
  }

  /// Return the number of bytes of space available for writing
  inline uint32_t write_space() const
  {
    return write_space_internal(_read_head, _write_head);
  }

  /// Return the capacity (i.e. total write space when empty)
  inline uint32_t capacity() const { return _size - 1; }

  /// Read from the RingBuffer without advancing the read head
  inline uint32_t peek(uint32_t size, void* dst)
  {
    return peek_internal(_read_head, _write_head, size, dst);
  }

  /// Read from the RingBuffer and advance the read head
  inline uint32_t read(uint32_t size, void* dst)
  {
    const uint32_t r = _read_head;
    const uint32_t w = _write_head;

    if (peek_internal(r, w, size, dst)) {
      std::atomic_thread_fence(std::memory_order_acquire);
      _read_head = (r + size) & _size_mask;
      return size;
    }

    return 0;
  }

  /// Skip data in the RingBuffer (advance read head without reading)
  inline uint32_t skip(uint32_t size)
  {
    const uint32_t r = _read_head;
    const uint32_t w = _write_head;
    if (read_space_internal(r, w) < size) {
      return 0;
    }

    std::atomic_thread_fence(std::memory_order_acquire);
    _read_head = (r + size) & _size_mask;
    return size;
  }

  /// Write data to the RingBuffer
  inline uint32_t write(uint32_t size, const void* src)
  {
    const uint32_t r = _read_head;
    const uint32_t w = _write_head;
    if (write_space_internal(r, w) < size) {
      return 0;
    }

    if (w + size <= _size) {
      memcpy(&_buf[w], src, size);
      std::atomic_thread_fence(std::memory_order_release);
      _write_head = (w + size) & _size_mask;
    } else {
      const uint32_t this_size = _size - w;
      assert(this_size < size);
      assert(w + this_size <= _size);
      memcpy(&_buf[w], src, this_size);
      memcpy(
        &_buf[0], static_cast<const char*>(src) + this_size, size - this_size);
      std::atomic_thread_fence(std::memory_order_release);
      _write_head = size - this_size;
    }

    return size;
  }

private:
  static inline uint32_t next_power_of_two(uint32_t size)
  {
    // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    size--;
    size |= size >> 1U;
    size |= size >> 2U;
    size |= size >> 4U;
    size |= size >> 8U;
    size |= size >> 16U;
    size++;
    return size;
  }

  inline uint32_t write_space_internal(uint32_t r, uint32_t w) const
  {
    if (r == w) {
      return _size - 1;
    }

    if (r < w) {
      return ((r - w + _size) & _size_mask) - 1;
    }

    return (r - w) - 1;
  }

  inline uint32_t read_space_internal(uint32_t r, uint32_t w) const
  {
    if (r < w) {
      return w - r;
    }

    return (w - r + _size) & _size_mask;
  }

  inline uint32_t peek_internal(uint32_t r,
                                uint32_t w,
                                uint32_t size,
                                void*    dst) const
  {
    if (read_space_internal(r, w) < size) {
      return 0;
    }

    if (r + size < _size) {
      memcpy(dst, &_buf[r], size);
    } else {
      const uint32_t first_size = _size - r;
      memcpy(dst, &_buf[r], first_size);
      memcpy(static_cast<char*>(dst) + first_size, &_buf[0], size - first_size);
    }

    return size;
  }

  mutable uint32_t _write_head; ///< Read index into _buf
  mutable uint32_t _read_head;  ///< Write index into _buf

  const uint32_t _size;      ///< Size (capacity) in bytes
  const uint32_t _size_mask; ///< Mask for fast modulo

  const std::unique_ptr<char[]> _buf; ///< Contents
};

} // namespace raul

#endif // RAUL_RINGBUFFER_HPP
