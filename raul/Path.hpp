/* This file is part of Raul.
 * Copyright (C) 2007-2009 David Robillard <http://drobilla.net>
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


/** A URI which is a path (for example a filesystem or OSC path).
 *
 * This enforces that a Path is a valid path, where each fragment is a valid
 * Symbol, separated by exactly one slash (/).
 *
 * A path is divided by slashes (/).  The first character MUST be a slash, and
 * the last character MUST NOT be a slash (except in the special case of the
 * root path "/", which is the only valid single-character path).  A Path
 * is actually a URI, the relative path is appended to the root URI
 * automatically, so a Patch can always be used as a URI.
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

	/** Return the root path.
	 * The default root path is the URI "path:/"
	 *
	 * A Path is either the root path, or a child of a root path (i.e. the root
	 * path followed by a sequence of Symbols separated by '/')
	 */
	static const Path root();

	/** Set the root path.
	 * The default root path is the URI "path:/"
	 *
	 * Note this should be done on application start up.  Changing the root
	 * path while any Path objects exist will break things horribly; don't!
	 *
	 * The root can be set to any URI, there are no restrictions on valid
	 * characters and such like there are for relative paths (but it must be
	 * a valid URI, i.e. begin with a scheme, and in particular not begin
	 * with '/').  Relative paths are appended to the root path's URI,
	 * i.e. every Path, as a string, begins with the root URI.  The part after
	 * that is a strict path (a sequence of Symbols separated by '/').
	 */
	static void set_root(const Raul::URI& uri);

	static bool is_path(const Raul::URI& uri);

	/** Construct an uninitialzed path, because the STL is annoying. */
	Path() : URI(root()) {}

	/** Construct a Path from an std::string.
	 *
	 * It is a fatal error to construct a Path from an invalid string,
	 * use is_valid first to check.
	 */
	Path(const std::basic_string<char>& path);

	/** Construct a Path from a C string.
	 *
	 * It is a fatal error to construct a Path from an invalid string,
	 * use is_valid first to check.
	 */
	Path(const char* cpath);


	/** Construct a Path from another path.
	 *
	 * This is faster than constructing a path from the other path's string
	 * representation, since validity checking is avoided.
	 */
	Path(const Path& copy) : URI(copy) {}

	static bool is_valid(const std::basic_string<char>& path);

	static bool is_valid_name(const std::basic_string<char>& name) {
		return name.length() > 0 && name.find("/") == std::string::npos
			&& is_valid(std::string("/").append(name));
	}

	static std::string pathify(const std::basic_string<char>& str);
	static std::string nameify(const std::basic_string<char>& str);

	static void replace_invalid_chars(std::string& str, size_t start, bool replace_slash = false);

	bool is_root() const { return (*this) == root(); }

	bool is_child_of(const Path& parent) const;
	bool is_parent_of(const Path& child) const;

	Path child(const std::string& s) const {
		if (is_valid(s))
			return base() + Path(s).chop_scheme().substr(1);
		else
			return base() + s;
	}

	Path child(const Path& p) const {
		return base() + p.chop_scheme().substr(1);
	}

	Path operator+(const Path& p) const { return child(p); }

	/** Return the symbol of this path (everything after the last '/').
	 * This is e.g. the "method name" for OSC paths, the filename
	 * for filesystem paths, etc.
	 * The empty string may be returned (if the path is the root path).
	 */
	inline const char* symbol() const {
		if ((*this) != root()) {
			const char* last_slash = strrchr(c_str(), '/');
			if (last_slash) {
				return last_slash + 1;
			}
		}
		return "";
	}

	/** Return the parent's path.
	 *
	 * Calling this on the path "/" will return "/".
	 * This is the (deepest) "container path" for OSC paths.
	 */
	inline Path parent() const {
		if ((*this) == root()) {
			return *this;
		} else {
			const std::string str(this->str());
			const size_t first_slash = str.find('/');
			const size_t last_slash  = str.find_last_of('/');
			return (first_slash == last_slash) ? root() : str.substr(0, last_slash);
		}
	}


	/** Return the path's child with the given name (symbol)
	 */
	inline Path child(const Raul::Symbol& symbol) const {
		return base() + symbol.c_str();
	}


	/** Return path relative to some base path (chop prefix)
	 */
	inline Path relative_to_base(const Path& base) const {
		if ((*this) == base) {
			return "/";
		} else {
			assert(length() > base.length());
			return substr(base.length() - 1);
		}
	}


	/** Return path with a trailing "/".
	 *
	 * Returned value is guaranteed to be a valid parent path, i.e. a valid
	 * child path can be made using parent.base() + child_name.
	 */
	inline const std::string base() const {
		std::string ret = str();
		if ((*this) == root() && ret[ret.length() - 1] == '/')
			return ret;
		else
			return ret + '/';
	}

	/** Return path with a trailing "/".
	 *
	 * Returned value is guaranteed to be a valid parent path, i.e. a valid
	 * child path can be made using parent.base() + child_name.
	 */
	inline const std::string base_no_scheme() const {
		return base().substr(find(":") + 1);
	}


	/** Return true if \a child is equal to, or a descendant of \a parent */
	static bool descendant_comparator(const Path& parent, const Path& child) {
		return ( child == parent || (child.length() > parent.length() &&
				(!std::strncmp(parent.c_str(), child.c_str(), parent.length())
						&& (parent == root() || child.str()[parent.length()] == '/'))) );
	}

private:
	inline Path(bool unchecked, const URI& uri) : URI(uri) {}
};


} // namespace Raul

#endif // RAUL_PATH_HPP
