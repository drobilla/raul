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

#include <exception>
#include <string>
#include <utility>

#ifndef RAUL_EXCEPTION_HPP
#define RAUL_EXCEPTION_HPP

namespace Raul {

/** An exception (unexpected error). */
class Exception : public std::exception {
public:
	const char* what() const noexcept { return _what.c_str(); }

protected:
	explicit Exception(const std::string& what) : _what(what) {}

private:
	const std::string _what;
};

} // namespace Raul

#endif // RAUL_EXCEPTION_HPP
