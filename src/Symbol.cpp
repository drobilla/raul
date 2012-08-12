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

#include <string>

#include "raul/Symbol.hpp"
#include "raul/Path.hpp"

using std::string;

namespace Raul {

bool
Symbol::is_valid(const std::basic_string<char>& symbol)
{
	if (symbol.length() == 0)
		return false;

	// Slash not allowed
	if (symbol.find("/") != string::npos)
		return false;

	// All characters must be _, a-z, A-Z, 0-9
	for (size_t i=0; i < symbol.length(); ++i)
		if (symbol[i] != '_'
				&& (symbol[i] < 'a' || symbol[i] > 'z')
				&& (symbol[i] < 'A' || symbol[i] > 'Z')
				&& (symbol[i] < '0' || symbol[i] > '9') )
			return false;

	// First character must not be a number
	if (std::isdigit(symbol[0]))
		return false;

	return true;
}

/** Convert a string to a valid symbol.
 *
 * This will make a best effort at turning @a str into a complete, valid
 * Symbol, and will always return one.
 */
Raul::Symbol
Symbol::symbolify(const std::basic_string<char>& str)
{
	string symbol = str;

	Path::replace_invalid_chars(symbol, 0, true);

	return Raul::Symbol(symbol.empty() ? "_" : symbol);
}

} // namespace Raul

