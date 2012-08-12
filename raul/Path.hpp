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

#ifndef RAUL_PATH_HPP
#define RAUL_PATH_HPP

#include <cassert>
#include <cctype>
#include <cstring>
#include <exception>
#include <string>

#include "raul/Symbol.hpp"

namespace Raul {

/** A restricted path of Symbols separated by, and beginning with, "/".
 *
 * This enforces that a Path is a valid path, where each fragment is a valid
 * Symbol, separated by exactly one slash (/).
 *
 * A path is divided by slashes (/).  The first character MUST be a slash, and
 * the last character MUST NOT be a slash (except in the special case of the
 * root path "/", which is the only valid single-character path).
 *
 * \ingroup raul
 */
class Path {
public:
	class BadPath : public std::exception {
	public:
		explicit BadPath(const std::string& path) : _path(path) {}
		~BadPath() throw() {}
		const char* what() const throw() { return _path.c_str(); }
	private:
		const std::string _path;
	};

	/** Construct an uninitialzed path, because the STL is annoying. */
	Path() : _str(g_intern_string("/")) {}

	/** Construct a Path from an std::string.
	 *
	 * It is a fatal error to construct a Path from an invalid string,
	 * use is_valid first to check.
	 */
	Path(const std::basic_string<char>& path) throw (BadPath)
		: _str(g_intern_string(path.c_str()))
	{
		if (!is_valid(path)) {
			throw BadPath(path);
		}
	}

	/** Construct a Path from a C string.
	 *
	 * It is a fatal error to construct a Path from an invalid string,
	 * use is_valid first to check.
	 */
	Path(const char* cpath) throw(BadPath)
		: _str(g_intern_string(cpath))
	{
		if (!is_valid(cpath)) {
			throw BadPath(cpath);
		}
	}

	/** Construct a Path from another path.
	 *
	 * This is faster than constructing a path from the other path's string
	 * representation, since validity checking is avoided.
	 */
	Path& operator=(const Path& other) {
		_str = other._str;
		return *this;
	}

	static bool is_valid(const std::basic_string<char>& path);

	/** Convert a string to a valid Path. */
	static Path pathify(const std::basic_string<char>& str);

	static void replace_invalid_chars(std::string& str,
	                                  size_t       start,
	                                  bool         replace_slash = false);

	bool is_root() const { return !strcmp(_str, "/"); }

	bool is_child_of(const Path& parent) const;
	bool is_parent_of(const Path& child) const;

	Path operator+(const Path& p) const { return child(p); }

	/** Return the lowest common ancestor of a and b. */
	static Path lca(const Path& a, const Path& b);

	/** Return the symbol of this path (everything after the last '/').
	 * This is e.g. the "method name" for OSC paths, the filename
	 * for filesystem paths, etc.
	 * The empty string may be returned (if the path is the root path).
	 */
	inline const char* symbol() const {
		if (!is_root()) {
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
		if (is_root()) {
			return *this;
		} else {
			const std::string str(this->str());
			const size_t first_slash = str.find('/');
			const size_t last_slash  = str.find_last_of('/');
			return (first_slash == last_slash) ? Path("/") : str.substr(0, last_slash);
		}
	}

	Path child(const Path& p) const {
		return base() + p.str().substr(1);
	}

	/** Return the path's child with the given symbol
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
			assert(length() > base.base().length());
			return str().substr(base.base().length() - 1);
		}
	}

	/** Return path with a trailing "/".
	 *
	 * Returned value is guaranteed to be a valid parent path, i.e. a valid
	 * child path can be made using parent.base() + symbol.
	 */
	inline const std::string base() const {
		std::string ret = str();
		if (is_root() && ret[ret.length() - 1] == '/')
			return ret;
		else
			return ret + '/';
	}

	/** Return true if \a child is equal to, or a descendant of \a parent */
	static bool descendant_comparator(const Path& parent, const Path& child) {
		return (child == parent ||
		        (child.length() > parent.length() &&
		         (!std::strncmp(parent.c_str(), child.c_str(), parent.length())
		          && (parent.is_root() || child.str()[parent.length()] == '/'))) );
	}

	inline std::string substr(size_t start, size_t end=std::string::npos) const {
		return str().substr(start, end);
	}

	inline bool operator<(const Path& path)  const { return strcmp(_str, path.c_str()) < 0; }
	inline bool operator<=(const Path& path) const { return (*this) == path || (*this) < path; }
	inline bool operator==(const Path& path) const { return _str == path._str; }
	inline bool operator!=(const Path& path) const { return _str != path._str; }

	inline char operator[](int i) const { return _str[i]; }

	inline size_t length()                   const { return str().length(); }
	inline size_t find(const std::string& s) const { return str().find(s); }

	inline const std::string str()   const { return _str; }
	inline const char*       c_str() const { return _str; }

private:
	const char* _str;
};

} // namespace Raul

static inline
std::ostream&
operator<<(std::ostream& os, const Raul::Path& path)
{
	return (os << path.c_str());
}

#endif // RAUL_PATH_HPP
