// Copyright 2007-2013 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAUL_NONCOPYABLE_HPP
#define RAUL_NONCOPYABLE_HPP

namespace raul {

/**
   Convenience base for non-copyable and non-movable classes.

   @ingroup raul
*/
class Noncopyable
{
public:
  Noncopyable(const Noncopyable&) = delete;
  const Noncopyable& operator=(const Noncopyable&) = delete;

  Noncopyable(Noncopyable&&) = delete;
  Noncopyable& operator=(Noncopyable&&) = delete;

protected:
  Noncopyable()  = default;
  ~Noncopyable() = default;
};

} // namespace raul

#endif // RAUL_NONCOPYABLE_HPP
