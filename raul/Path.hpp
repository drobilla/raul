/* This file is part of Raul.
 * Copyright (C) 2007 Dave Robillard <http://drobilla.net>
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

#ifndef RAUL_PATH_HPP
#define RAUL_PATH_HPP

#include <iostream>
#include <cctype>
#include <string>
#include <cstring>
#include <cassert>

#include "raul/Symbol.hpp"
#include "raul/URI.hpp"

namespace Raul {

	
/** Simple wrapper around standard string with useful path-specific methods.
 *
 * This enforces that a Path is a valid OSC path (though it is used for
 * GraphObject paths, which aren't directly OSC paths but a portion of one).
 *
 * A path is divided by slashes (/).  The first character MUST be a slash, and
 * the last character MUST NOT be a slash (except in the special case of the 
 * root path "/", which is the only valid single-character path).
 *
 * Valid characters are the 95 printable ASCII characters (32-126), excluding:
 * space # * , ? [ ] { }
 *
 * \ingroup raul
 */
class Path : public URI {
public:
	class BadPath : public std::exception {
	public:
		BadPath(const std::string& path) : _path(path) {}
		~BadPath() throw() {}
		const char* what() const throw() { return _path.c_str(); }
	private:
		std::string _path;
	};

	static const std::string scheme;
	static const std::string prefix;
	static const size_t      prefix_len;
	static const std::string root_uri;

	/** Construct an uninitialzed path, because the STL is annoying. */
	Path() : URI(root_uri) {}

	/** Construct a Path from an std::string.
	 *
	 * It is a fatal error to construct a Path from an invalid string,
	 * use is_valid first to check.
	 */
	Path(const std::basic_string<char>& path)
		: URI((path.find(":") == std::string::npos) ? prefix + path : path)
	{
		if (!is_valid(str()))
			throw BadPath(str());
	}
	
	/** Construct a Path from a C string.
	 *
	 * It is a fatal error to construct a Path from an invalid string,
	 * use is_valid first to check.
	 */
	Path(const char* cpath)
		: URI((std::string(cpath).find(":") == std::string::npos) ? prefix + cpath : cpath)
	{
		if (!is_valid(str()))
			throw BadPath(str());
	}
	
	static bool is_valid(const std::basic_string<char>& path);
	
	static bool is_valid_name(const std::basic_string<char>& name) {
		return name.length() > 0 && name.find("/") == std::string::npos
			&& is_valid(std::string("/").append(name));
	}

	static std::string pathify(const std::basic_string<char>& str);
	static std::string nameify(const std::basic_string<char>& str);

	static void replace_invalid_chars(std::string& str, size_t start, bool replace_slash = false);
	
	bool is_root() const { return str() == root_uri; }

	bool is_child_of(const Path& parent) const;
	bool is_parent_of(const Path& child) const;
	
	Path child(const std::string& s) const {
		if (is_valid(s))
			return std::string(base()) + Path(s).chop_scheme().substr(1);
		else
			return std::string(base()) + s;
	}

	Path operator+(const Path& p) const { return child(p); }
	
	/** Return the name of this object (everything after the last '/').
	 * This is the "method name" for OSC paths.
	 * The empty string may be returned (if the path is "/").
	 */
	inline std::string name() const {
		if (str() == root_uri)
			return "";
		else
			return substr(find_last_of("/")+1);
	}
	
	
	/** Return the name of this object (everything after the last '/').
	 * This is the "method name" for OSC paths.
	 * Note it is illegal to call this method on the path "/".
	 */
	inline Symbol symbol() const {
		return substr(find_last_of("/")+1);
	}
	
	
	/** Return the parent's path.
	 *
	 * Calling this on the path "/" will return "/".
	 * This is the (deepest) "container path" for OSC paths.
	 */
	inline Path parent() const {
		if (str() == root_uri) {
			return str();
		} else {
			size_t last_slash = find_last_of("/");
			return (last_slash == prefix_len) ? root_uri : substr(0, last_slash);
		}
	}
	

	/** Return path relative to some base path (chop prefix)
	 */
	inline Path relative_to_base(const Path& base) const {
		if (str() == base) {
			return "/";
		} else {
			assert(length() > base.length());
			return substr(base.length());
		}
	}


	/** Return path with a trailing "/".
	 *
	 * Returned value is guaranteed to be a valid parent path, i.e. a valid
	 * child path can be made using parent.base() + child_name.
	 */
	inline const std::string base() const {
		if (str() == root_uri)
			return str();
		else
			return str() + "/";
	}


	/** Return true if \a child is equal to, or a descendant of \a parent */
	static bool descendant_comparator(const Path& parent, const Path& child) {
		return ( child == parent || (child.length() > parent.length() &&
				(!std::strncmp(parent.c_str(), child.c_str(), parent.length())
						&& (parent.str() == root_uri || child[parent.length()] == '/'))) );
	}
};


} // namespace Raul

#endif // RAUL_PATH_HPP
