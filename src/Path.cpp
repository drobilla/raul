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

#include <string>

#include "raul/Path.hpp"

using std::string;

namespace Raul {

static URI root_uri("path:/");

const Path Path::root()                         { return Path(true, root_uri); }
void       Path::set_root(const Raul::URI& uri) { root_uri = uri.str(); }

bool
Path::is_path(const Raul::URI& uri)
{
	return uri.length() >= root_uri.length()
			&& uri.substr(0, root_uri.length()) == root_uri.str()
			&& Path::is_valid(uri.str());
}


Path::Path(const std::basic_string<char>& path)
	: URI(path[0] == '/' ? root_uri.str() + path.substr(1) : path)
{
	if (!is_valid(str()))
		throw BadPath(str());
}


Path::Path(const char* cpath)
	: URI(cpath[0] == '/' ? root_uri.str() + (cpath + 1) : cpath)
{
	if (!is_valid(str()))
		throw BadPath(str());
}


bool
Path::is_valid(const std::basic_string<char>& path_str)
{
	if (path_str.length() == 0)
		return false;

	if (path_str == root_uri.str())
		return true;

	if (path_str[0] != '/' &&
			(path_str.length() < root_uri.length()
				|| path_str.substr(0, root_uri.length()) != root_uri.str()))
		return false;

	const string path = (path_str[0] == '/')
			? path_str
			: path_str.substr(root_uri.length() - 1);

	// Must start with a /
	if (path[0] != '/')
		return false;

	// Must not end with a slash unless "/"
	if (path.length() > 1 && path[path.length()-1] == '/')
		return false;

	assert(path.find_last_of("/") != string::npos);

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


/** Convert a string to a valid full path.
 *
 * The returned string is a valid relative path without the root prefix,
 * i.e. the returned string starts with '/' followed by valid symbols,
 * each separated by '/'.
 */
string
Path::pathify(const std::basic_string<char>& str)
{
	if (str.length() == 0)
		return root().chop_scheme(); // this might not be wise?

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

	assert(path.find_last_of('/') != string::npos);

	replace_invalid_chars(path, 0, false);

	assert(is_valid(path));

	return path;
}


/** Convert a string to a valid name (or "method" - tokens between slashes)
 *
 * This will strip all slashes, etc, and always return a valid name/method.
 */
string
Path::nameify(const std::basic_string<char>& str)
{
	string name = str;

	if (name.length() == 0)
		return "_"; // this might not be wise

	replace_invalid_chars(name, 0, true);

	assert(is_valid(string("/") + name));

	return name;
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
	return (substr(0, parent_base.length()) == parent_base);
}


bool
Path::is_parent_of(const Path& child) const
{
	return child.is_child_of(*this);
}


} // namespace Raul

