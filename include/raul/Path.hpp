/*
  Copyright 2007-2014 David Robillard <d@drobilla.net>

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

#include "raul/Exception.hpp"
#include "raul/Symbol.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

namespace raul {

/**
   A restricted path of Symbols separated by, and beginning with, "/".

   A Path never ends with a "/", except for the root Path "/", which is the
   only valid single-character Path.

   @ingroup raul
*/
class Path : public std::basic_string<char>
{
public:
  /// Attempt to construct an invalid Path
  class BadPath : public Exception
  {
  public:
    explicit BadPath(const std::string& path)
      : Exception(path)
    {}
  };

  /// Construct the root path `/`
  Path()
    : std::basic_string<char>("/")
  {}

  /**
     Construct a Path from a C++ string.

     This will throw an exception if `path` is invalid.  To avoid this, use
     is_valid() first to check.
  */
  explicit Path(const std::basic_string<char>& path)
    : std::basic_string<char>(path)
  {
    if (!is_valid(path)) {
      throw BadPath(path);
    }
  }

  /**
     Construct a Path from a C string.

     This will throw an exception if `path` is invalid.  To avoid this, use
     is_valid() first to check.
  */
  explicit Path(const char* path)
    : std::basic_string<char>(path)
  {
    if (!is_valid(path)) {
      throw BadPath(path);
    }
  }

  Path(const Path& path) = default;
  Path& operator=(const Path& path) = default;

  Path(Path&& path) = default;
  Path& operator=(Path&& path) = default;

  ~Path() = default;

  /// Return true iff `c` is a valid Path character
  static inline bool is_valid_char(char c)
  {
    return c == '/' || Symbol::is_valid_char(c);
  }

  /// Return true iff `str` is a valid Path
  static inline bool is_valid(const std::basic_string<char>& str)
  {
    if (str.empty() || (str[0] != '/')) {
      return false; // Must start with '/'
    }

    if (str != "/" && *str.rbegin() == '/') {
      return false; // Must not end with '/' except for the root
    }

    for (size_t i = 1; i < str.length(); ++i) {
      if (!is_valid_char(str[i])) {
        return false; // All characters must be /, _, a-z, A-Z, 0-9
      }

      if (str[i - 1] == '/') {
        if (str[i] == '/') {
          return false; // Must not contain "//"
        }

        if (!Symbol::is_valid_start_char(str[i])) {
          return false; // Invalid symbol start character (digit)
        }
      }
    }

    return true;
  }

  /// Return true iff this path is the root path ("/")
  inline bool is_root() const { return *this == "/"; }

  /// Return true iff this path is a child of `parent` at any depth
  inline bool is_child_of(const Path& parent) const
  {
    const std::string parent_base = parent.base();
    return substr(0, parent_base.length()) == parent_base;
  }

  /// Return true iff this path is a parent of `child` at any depth
  inline bool is_parent_of(const Path& child) const
  {
    return child.is_child_of(*this);
  }

  /**
     Return the symbol of this path (everything after the last '/').

     This is what is called the "basename" for file paths, the "method name"
     for OSC paths, and so on.  Since the root path does not have a symbol,
     this does not return a raul::Symbol but may return the empty string.
  */
  inline const char* symbol() const
  {
    if (!is_root()) {
      const size_t last_sep = rfind('/');
      if (last_sep != std::string::npos) {
        return c_str() + last_sep + 1;
      }
    }
    return "";
  }

  /**
     Return the parent's path.

     Calling this on the path "/" will return "/".
     This is the (deepest) "container path" for OSC paths.
  */
  inline Path parent() const
  {
    if (is_root()) {
      return *this;
    }

    const size_t first_sep = find('/');
    const size_t last_sep  = find_last_of('/');
    return (first_sep == last_sep) ? Path("/") : Path(substr(0, last_sep));
  }

  /// Return a child Path of this path
  inline Path child(const Path& p) const
  {
    return p.is_root() ? *this : Path(base() + p.substr(1));
  }

  /// Return a direct child Path of this Path with the given Symbol
  inline Path child(const raul::Symbol& symbol) const
  {
    return Path(base() + symbol.c_str());
  }

  /**
     Return path with a trailing "/".

     The returned string is such that appending a valid Symbol to it is
     guaranteed to form a valid path.
  */
  inline std::string base() const
  {
    if (is_root()) {
      return *this;
    }

    return *this + '/';
  }

  /// Return the lowest common ancestor of a and b
  static inline Path lca(const Path& a, const Path& b)
  {
    const size_t len      = std::min(a.length(), b.length());
    size_t       last_sep = 0;
    for (size_t i = 0; i < len; ++i) {
      if (a[i] == '/' && b[i] == '/') {
        last_sep = i;
      }
      if (a[i] != b[i]) {
        break;
      }
    }

    if (last_sep <= 1) {
      return Path("/");
    }

    return Path(a.substr(0, last_sep));
  }

  /// Return true iff `child` is equal to, or a descendant of `parent`
  static inline bool descendant_comparator(const Path& parent,
                                           const Path& child)
  {
    return (child == parent || child.is_child_of(parent));
  }
};

} // namespace raul

#endif // RAUL_PATH_HPP
