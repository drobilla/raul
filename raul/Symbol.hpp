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

#ifndef RAUL_SYMBOL_HPP
#define RAUL_SYMBOL_HPP

#include <string>

#include "raul/Exception.hpp"

namespace Raul {

/** A restricted string which is a valid C identifier and Path component.
 *
 * A Symbol is a very restricted string suitable for use as an identifier.
 * It is a valid LV2 symbol, URI fragment, filename, OSC path fragment,
 * and identifier for virtually all programming languages.
 *
 * Valid characters are _, a-z, A-Z, 0-9, except the first character which
 * must not be 0-9.
 *
 * @ingroup raul
 */
class Symbol : public std::basic_string<char> {
public:
	/** Attempt to construct an invalid Symbol. */
	class BadSymbol : public Exception {
	public:
		explicit BadSymbol(const std::string& symbol) : Exception(symbol) {}
	};

	/** Construct a Symbol from a C++ string.
	 *
	 * This will throw an exception if `symbol` is invalid.  To avoid this,
	 * use is_valid() first to check.
	 */
	explicit Symbol(const std::basic_string<char>& symbol)
		: std::basic_string<char>(symbol)
	{
		if (!is_valid(symbol)) {
			throw BadSymbol(symbol);
		}
	}

	/** Construct a Symbol from a C string.
	 *
	 * This will throw an exception if `symbol` is invalid.  To avoid this,
	 * use is_valid() first to check.
	 */
	explicit Symbol(const char* symbol)
		: std::basic_string<char>(symbol)
	{
		if (!is_valid(symbol)) {
			throw BadSymbol(symbol);
		}
	}

	/** Copy a Symbol.
	 *
	 * Note this is faster than constructing a Symbol from another Symbol's
	 * string since validation is unnecessary.
	 */
	Symbol(const Symbol& symbol)
		: std::basic_string<char>(symbol)
	{}

	/** Return true iff `c` is a valid Symbol start character. */
	static inline bool is_valid_start_char(char c) {
		return (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	/** Return true iff `c` is a valid Symbol character. */
	static inline bool is_valid_char(char c) {
		return is_valid_start_char(c) || (c >= '0' && c <= '9');
	}

	/** Return true iff `str` is a valid Symbol. */
	static inline bool is_valid(const std::basic_string<char>& str) {
		if (str.empty() || (str[0] >= '0' && str[0] <= '9')) {
			return false;  // Must start with a letter or underscore
		}

		for (size_t i = 0; i < str.length(); ++i) {
			if (!is_valid_char(str[i])) {
				return false;  // All characters must be _, a-z, A-Z, 0-9
			}
		}

		return true;
	}

	/** Convert a string to a valid symbol.
	 *
	 * This will make a best effort at turning @a str into a complete, valid
	 * Symbol, and will always return one.
	 */
	static inline Symbol symbolify(const std::basic_string<char>& in) {
		if (in.empty()) {
			return Symbol("_");
		}

		std::basic_string<char> out(in);
		for (size_t i = 0; i < in.length(); ++i) {
			if (!is_valid_char(out[i])) {
				out[i] = '_';
			}
		}

		if (is_valid_start_char(out[0])) {
			return Symbol(out);
		} else {
			return Symbol(std::string("_") + out);
		}
	}
};

} // namespace Raul

#endif // RAUL_SYMBOL_HPP
