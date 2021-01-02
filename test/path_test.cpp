/*
  Copyright 2007-2019 David Robillard <d@drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#undef NDEBUG

#include "raul/Path.hpp"
#include "raul/Symbol.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

int
main()
{
	using Path   = Raul::Path;
	using Symbol = Raul::Symbol;

	assert(Path("/foo/bar").parent() == Path("/foo"));
	assert(Path("/foo").parent() == Path("/"));
	assert(Path("/").parent() == Path("/"));

	assert(Path("/").is_parent_of(Path("/foo")));
	assert(Path("/foo").is_parent_of(Path("/foo/bar")));
	assert(!(Path("/foo").is_parent_of(Path("/foo2"))));
	assert(!(Path("/foo").is_parent_of(Path("/foo"))));

	assert(Path::lca(Path("/foo"), Path("/foo/bar/baz")) == Path("/"));
	assert(Path::lca(Path("/foo/bar"), Path("/foo/bar/baz")) == Path("/foo"));
	assert(Path::lca(Path("/foo/bar/quux"), Path("/foo/bar/baz")) == Path("/foo/bar"));

	assert(!Path::is_valid(""));
	assert(!Path::is_valid("hello"));
	assert(!Path::is_valid("/foo/bar/"));
	assert(!Path::is_valid("/foo//bar"));
	assert(!Path::is_valid("/foo/bar/d*s"));
	assert(Path::is_valid("/"));
	assert(!Path::is_valid("/foo/3foo/bar"));

	assert(Path::descendant_comparator(Path("/"), Path("/")));
	assert(Path::descendant_comparator(Path("/"), Path("/foo")));
	assert(Path::descendant_comparator(Path("/foo"), Path("/foo/bar")));
	assert(Path::descendant_comparator(Path("/foo"), Path("/foo")));
	assert(Path::descendant_comparator(Path("/"), Path("/")));
	assert(!Path::descendant_comparator(Path("/baz"), Path("/")));
	assert(!Path::descendant_comparator(Path("/foo"), Path("/bar")));
	assert(!Path::descendant_comparator(Path("/foo/bar"), Path("/foo")));

	assert(!Symbol::is_valid(""));
	assert(!Symbol::is_valid("/I/have/slashes"));
	assert(!Symbol::is_valid("!illegalchar"));
	assert(!Symbol::is_valid("0illegalleadingdigit"));
	assert(strcmp(Symbol::symbolify("").c_str(), ""));

	assert(Path("/foo").child(Symbol("bar")) == "/foo/bar");
	assert(Path("/foo").child(Path("/bar/baz")) == "/foo/bar/baz");
	assert(Path("/foo").child(Path("/")) == "/foo");

	assert(!strcmp(Path("/foo").symbol(), "foo"));
	assert(!strcmp(Path("/foo/bar").symbol(), "bar"));
	assert(!strcmp(Path("/").symbol(), ""));

	const Path original(std::string("/foo/bar"));
	assert(original == original);

	bool valid = true;
	try {
		Path path("/ends/in/slash/");
	} catch (const Path::BadPath& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		valid = false;
	}
	assert(!valid);

	valid = true;
	try {
		Path path(std::string("/has//double/slash"));
	} catch (const Path::BadPath& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		valid = false;
	}
	assert(!valid);

	return 0;
}
