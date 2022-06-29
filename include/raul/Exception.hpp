// Copyright 2007-2012 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAUL_EXCEPTION_HPP
#define RAUL_EXCEPTION_HPP

#include <exception>
#include <string>
#include <utility>

namespace raul {

/**
   An exception (unexpected error).

   @ingroup raul
*/
class Exception : public std::exception
{
public:
  // NOLINTNEXTLINE(modernize-use-override, hicpp-use-override)
  [[nodiscard]] const char* what() const noexcept final override
  {
    return _what.c_str();
  }

protected:
  explicit Exception(std::string what)
    : _what(std::move(what))
  {}

private:
  const std::string _what;
};

} // namespace raul

#endif // RAUL_EXCEPTION_HPP
