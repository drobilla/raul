// Copyright 2007-2013 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAUL_ARRAY_HPP
#define RAUL_ARRAY_HPP

#include <raul/Maid.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>

namespace raul {

/**
   A disposable array with a size.

   @ingroup raul
*/
template<class T>
class Array : public Maid::Disposable
{
public:
  explicit Array(size_t size = 0)
    : Maid::Disposable()
    , _size(size)
    , _elems(size ? new T[size] : nullptr)
  {}

  Array(size_t size, T initial_value)
    : Maid::Disposable()
    , _size(size)
    , _elems(size ? new T[size] : nullptr)
  {
    if (size > 0) {
      for (size_t i = 0; i < size; ++i) {
        _elems[i] = initial_value;
      }
    }
  }

  Array(const Array<T>& array)
    : Maid::Disposable()
    , _size(array._size)
    , _elems(_size ? new T[_size] : nullptr)
  {
    for (size_t i = 0; i < _size; ++i) {
      _elems[i] = array._elems[i];
    }
  }

  ~Array() override = default;

  Array<T>& operator=(const Array<T>& array)
  {
    if (&array == this) {
      return *this;
    }

    _size  = array._size;
    _elems = _size ? new T[_size] : nullptr;

    for (size_t i = 0; i < _size; ++i) {
      _elems[i] = array._elems[i];
    }
  }

  Array(Array<T>&& array) noexcept
    : _size(array._size)
    , _elems(std::move(array._elems))
  {}

  Array<T>& operator=(Array<T>&& array) noexcept
  {
    _size  = array._size;
    _elems = std::move(array._elems);
    return *this;
  }

  Array(size_t size, const Array<T>& contents)
    : _size(size)
    , _elems(size ? new T[size] : nullptr)
  {
    assert(contents.size() >= size);
    for (size_t i = 0; i < std::min(size, contents.size()); ++i) {
      _elems[i] = contents[i];
    }
  }

  Array(size_t size, const Array<T>& contents, T initial_value = T())
    : _size(size)
    , _elems(size ? new T[size] : nullptr)
  {
    const size_t end = std::min(size, contents.size());
    for (size_t i = 0; i < end; ++i) {
      _elems[i] = contents[i];
    }
    for (size_t i = end; i < size; ++i) {
      _elems[i] = initial_value;
    }
  }

  virtual void alloc(size_t num_elems)
  {
    _size = num_elems;

    if (num_elems > 0) {
      _elems = std::unique_ptr<T[]>(new T[num_elems]);
    } else {
      _elems.reset();
    }
  }

  virtual void alloc(size_t num_elems, T initial_value)
  {
    _size = num_elems;

    if (num_elems > 0) {
      _elems = std::unique_ptr<T[]>(new T[num_elems]);
      for (size_t i = 0; i < _size; ++i) {
        _elems[i] = initial_value;
      }
    } else {
      _elems.reset();
    }
  }

  [[nodiscard]] size_t size() const { return _size; }

  [[nodiscard]] T& operator[](size_t i) const
  {
    assert(i < _size);
    return _elems[i];
  }

  [[nodiscard]] T& at(size_t i) const
  {
    assert(i < _size);
    return _elems[i];
  }

private:
  size_t               _size;
  std::unique_ptr<T[]> _elems;
};

} // namespace raul

#endif // RAUL_ARRAY_HPP
