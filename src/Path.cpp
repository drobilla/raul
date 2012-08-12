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

#include "raul/Path.hpp"

using std::string;

namespace Raul {

bool
Path::is_valid(const std::basic_string<char>& path)
{
	if (path.empty() || path[0] != '/') {
		return false;
	}

	// Root path
	if (path == "/") {
		return true;
	}

	// Not root, must not end with a slash
	if (*path.rbegin() == '/')
		return false;

	// Double slash not allowed
	if (path.find("//") != string::npos)
		return false;

	for (size_t i=0; i < path.length(); ++i)
		// All contained symbols must not start with a digit
		if (i > 0 && path[i-1] == '/' && isdigit(path[i]))
			return false;
		// All characters must be _, a-z, A-Z, 0-9
		else if ( path[i] != '/' && path[i] != '_'
				&& (path[i] < 'a' || path[i] > 'z')
				&& (path[i] < 'A' || path[i] > 'Z')
				&& (path[i] < '0' || path[i] > '9') )
			return false;

	return true;
}

Path
Path::pathify(const std::basic_string<char>& str)
{
	if (str.length() == 0)
		return Path("/"); // this might not be wise?

	const size_t first_slash = str.find('/');
	string path = (first_slash == string::npos)
			? string("/").append(str)
			: str.substr(first_slash);

	// Must start with a /
	if (path.empty() || path[0] != '/')
		path = string("/").append(path);

	// Must not end with a slash unless "/"
	if (path != "/" && path[path.length() - 1] == '/')
		path = path.substr(0, path.length() - 1); // chop trailing slash

	replace_invalid_chars(path, 0, false);
	return Path(path);
}

/** Replace any invalid characters in @a str with a suitable replacement.
 */
void
Path::replace_invalid_chars(std::string& str, size_t start, bool replace_slash)
{
	string prefix = str.substr(0, start);
	str = str.substr(start);

	size_t open_bracket = str.find_first_of('(');
	if (open_bracket != string::npos)
		str = str.substr(0, open_bracket);

	open_bracket = str.find_first_of('[');
	if (open_bracket != string::npos)
		str = str.substr(0, open_bracket);

	if (str[str.length()-1] == ' ')
		str = str.substr(0, str.length()-1);

	if (isdigit(str[0]))
		str = string("_").append(str);

	for (size_t i=0; i < str.length(); ++i) {
		if (i > 0 && str[i-1] == '/' && isdigit(str[i])) {
			str = str.substr(0, i) + "_" + str.substr(i);
		} else if (str[i] == '\'') {
			str = str.substr(0, i) + str.substr(i+1);
		} else if ( str[i] != '_' && str[i] != '/'
				&& (str[i] < 'a' || str[i] > 'z')
				&& (str[i] < 'A' || str[i] > 'Z')
				&& (str[i] < '0' || str[i] > '9') ) {
			if (i > 0 && str[i-1] == '_') {
				str = str.substr(0, i) + str.substr(i+1);
				--i;
			} else {
				str[i] = '_';
			}
		} else if (replace_slash && str[i] == '/') {
			str[i] = '_';
		}
	}

	if (str.length() != 1 && str[str.length()-1] == '_')
		str = str.substr(0, str.length()-1);

	str = prefix + str;
}

bool
Path::is_child_of(const Path& parent) const
{
	const string parent_base = parent.base();
	return (str().substr(0, parent_base.length()) == parent_base);
}

bool
Path::is_parent_of(const Path& child) const
{
	return child.is_child_of(*this);
}

Path
Path::lca(const Path& a, const Path& b)
{
	const size_t len = std::min(a.length(), b.length());

	size_t last_slash = 0;
	for (size_t i = 0; i < len; ++i) {
		if (a[i] == '/' && b[i] == '/') {
			last_slash = i;
		}
		if (a[i] != b[i]) {
			break;
		}
	}

	if (last_slash <= 1) {
		return Path("/");
	}
	return Path(a.str().substr(0, last_slash));
}

} // namespace Raul
