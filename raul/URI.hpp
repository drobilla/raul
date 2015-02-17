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

#ifndef RAUL_URI_HPP
#define RAUL_URI_HPP

#include <string>

#include "raul/Exception.hpp"

namespace Raul {

/** A URI (RFC3986) string.
 *
 * @ingroup raul
 */
class URI : public std::basic_string<char> {
public:
	/** Attempt to construct an invalid URI. */
	class BadURI : public Exception {
	public:
		explicit BadURI(const std::string& uri) : Exception(uri) {}
	};

	/** Construct a URI from a C++ string.
	 *
	 * This will throw an exception if `uri` is invalid.  To avoid this, use
	 * is_valid() first to check.
	 */
	explicit URI(const std::basic_string<char>& uri)
		: std::basic_string<char>(uri)
	{
		if (!is_valid(uri)) {
			throw BadURI(uri);
		}
	}

	/** Construct a URI from a C string.
	 *
	 * This will throw an exception if `uri` is invalid.  To avoid this, use
	 * is_valid() first to check.
	 */
	explicit URI(const char* uri)
		: std::basic_string<char>(uri)
	{
		if (!is_valid(uri)) {
			throw BadURI(uri);
		}
	}

	/** Copy a URI.
	 *
	 * Note this is faster than constructing a URI from another URI's string
	 * since validation is unnecessary.
	 */
	URI(const URI& uri)
		: std::basic_string<char>(uri)
	{}

	/** Return true iff `c` is a valid URI start character. */
	static inline bool is_valid_start_char(char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	/** Return true iff `c` is a valid URI scheme character. */
	static inline bool is_valid_scheme_char(char c) {
		// S3.1: scheme ::= ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
		return is_valid_start_char(c) || (c >= '0' && c <= '9') ||
			c == '+' || c == '-' || c == '.';
	}

	/** Return true iff `str` is a valid URI.
	 *
	 * Currently this only checks that `starts` with a valid URI scheme.
	 */
	static inline bool is_valid(const std::basic_string<char>& str) {
		if (!is_valid_start_char(str[0])) {
			return false;  // Must start with a-z A-Z
		}

		for (size_t i = 1; i < str.length(); ++i) {
			if (str[i] == ':') {
				return true;  // Starts with a valid scheme
			} else if (!is_valid_scheme_char(str[i])) {
				return false;  // Invalid scheme character encountered
			}
		}

		return false;  // Must start with a scheme followed by ':'
	}

	/** Return the URI scheme (everything before the first ':') */
	inline std::string scheme() const { return substr(0, find(":")); }
};

} // namespace Raul

#endif // RAUL_URI_HPP
