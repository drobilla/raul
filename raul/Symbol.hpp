/* This file is part of Raul.
 * Copyright (C) 2008-2009 David Robillard <http://drobilla.net>
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

#ifndef RAUL_SYMBOL_HPP
#define RAUL_SYMBOL_HPP

#include <iostream>
#include <cctype>
#include <string>
#include <cstring>
#include <cassert>
#include <glib.h>

namespace Raul {


/** A restricted string (C identifier, which is a component of a Path).
 *
 * A Symbol is a very restricted string suitable for use as an identifier.
 * It is a valid LV2 symbol, URI fragment, filename, OSC path fragment,
 * and identifier for most programming languages (including C).
 *
 * Valid characters are _, a-z, A-Z, 0-9, except the first character which
 * must not be 0-9.
 *
 * \ingroup raul
 */
class Symbol {
public:
	/** Construct a Symbol from an std::string.
	 *
	 * It is a fatal error to construct a Symbol from an invalid string,
	 * use is_valid first to check.
	 */
	Symbol(const std::basic_string<char>& symbol)
		: _str(g_intern_string(symbol.c_str()))
	{
		assert(is_valid(symbol));
	}


	/** Construct a Symbol from a C string.
	 *
	 * It is a fatal error to construct a Symbol from an invalid string,
	 * use is_valid first to check.
	 */
	Symbol(const char* csymbol)
		: _str(g_intern_string(csymbol))
	{
		assert(is_valid(csymbol));
	}

	inline const char* c_str() const { return _str; }

	inline bool operator==(const Symbol& other) const {
		return _str == other._str;
	}

	inline bool operator!=(const Symbol& other) const {
		return _str != other._str;
	}

	inline bool operator<(const Symbol& other) const {
		return strcmp(_str, other._str) < 0;
	}

	static bool is_valid(const std::basic_string<char>& symbol);

	static std::string symbolify(const std::basic_string<char>& str);

private:
	const char* _str;
};


} // namespace Raul

static inline std::ostream& operator<<(std::ostream& os, const Raul::Symbol& symbol)
{
	return (os << symbol.c_str());
}

#endif // RAUL_SYMBOL_HPP
