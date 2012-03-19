/* This file is part of Raul.
 * Copyright 2007-2011 David Robillard <http://drobilla.net>
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

#ifndef RAUL_URI_HPP
#define RAUL_URI_HPP

#include <string>
#include <cstring>
#include <exception>
#include <ostream>
#include <glib.h>

namespace Raul {

/** Simple wrapper around standard string with useful URI-specific methods.
 *
 * This "should" be used for proper URIs (RFC3986), but not much support or
 * validation is built-in yet.  The URI string MUST have a scheme though.
 * \ingroup raul
 */
class URI {
public:
	class BadURI : public std::exception {
	public:
		BadURI(const std::string& uri) : _uri(uri) {}
		~BadURI() throw() {}
		const char* what() const throw() { return _uri.c_str(); }
	private:
		std::string _uri;
	};

	/** Construct a URI from an std::string.
	 *
	 * It is a fatal error to construct a URI from an invalid string,
	 * use is_valid first to check.
	 */
	URI(const std::basic_string<char>& uri="nil:0") : _str(g_intern_string(uri.c_str())) {
		if (!is_valid(uri))
			throw BadURI(uri);
	}

	/** Construct a URI from a C string.
	 *
	 * It is a fatal error to construct a URI from an invalid string,
	 * use is_valid first to check.
	 */
	URI(const char* uri) : _str(g_intern_string(uri)) {
		if (!is_valid(uri))
			throw BadURI(uri);
	}

	static bool is_valid(const std::basic_string<char>& uri) {
		return uri.find(":") != std::string::npos;
	}

	/** Return path with everything up to and including the first occurence of str chopped */
	inline const std::string chop_start(const std::string& str) const {
		return substr(find(str) + str.length());
	}

	/** Return the URI with the scheme removed (as a string) */
	std::string chop_scheme() const { return chop_start(":"); }

	/** Return the URI scheme (everything before the first ':') */
	inline std::string scheme() const { return substr(0, find(":")); }

	inline const std::string str()   const { return _str; }
	inline const char*       c_str() const { return _str; }

	inline std::string substr(size_t start, size_t end=std::string::npos) const {
		return str().substr(start, end);
	}

	inline bool operator<(const URI& uri)  const { return strcmp(_str, uri.c_str()) < 0; }
	inline bool operator<=(const URI& uri) const { return (*this) == uri || (*this) < uri; }
	inline bool operator==(const URI& uri) const { return _str == uri._str; }
	inline bool operator!=(const URI& uri) const { return _str != uri._str; }

	inline size_t length()                           const { return str().length(); }
	inline size_t find(const std::string& s)         const { return str().find(s); }
	inline size_t find_last_of(char c) const { return str().find_last_of(c); }

private:
	const char* _str;
};

static inline
std::ostream&
operator<<(std::ostream& os, const URI& uri)
{
	return (os << uri.c_str());
}

} // namespace Raul

#endif // RAUL_URI_HPP

