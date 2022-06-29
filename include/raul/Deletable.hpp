// Copyright 2007-2013 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAUL_DELETABLE_HPP
#define RAUL_DELETABLE_HPP

namespace raul {

/**
   Something with a virtual destructor.

   @ingroup raul
*/
class Deletable
{
public:
  Deletable() = default;

  Deletable(const Deletable&) = default;
  Deletable& operator=(const Deletable&) = default;

  Deletable(Deletable&&) = default;
  Deletable& operator=(Deletable&&) = default;

  virtual ~Deletable() = default;
};

} // namespace raul

#endif // RAUL_DELETABLE_HPP
