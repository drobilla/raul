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

#include "raul/Path.hpp"
#include "raul/Symbol.hpp"

#include <cstring>
#include <iostream>
#include <string>

int
main()
{
	using Path   = Raul::Path;
	using Symbol = Raul::Symbol;

#define CHECK(cond) \
	do { if (!(cond)) { \
		std::cerr << "Test failed: " << (cond) << std::endl; \
		return 1; \
	} } while (0)

	CHECK(Path("/foo/bar").parent() == Path("/foo"));
	CHECK(Path("/foo").parent() == Path("/"));
	CHECK(Path("/").parent() == Path("/"));

	CHECK(Path("/").is_parent_of(Path("/foo")));
	CHECK(Path("/foo").is_parent_of(Path("/foo/bar")));
	CHECK(!(Path("/foo").is_parent_of(Path("/foo2"))));
	CHECK(!(Path("/foo").is_parent_of(Path("/foo"))));

	CHECK(Path::lca(Path("/foo"), Path("/foo/bar/baz")) == Path("/"));
	CHECK(Path::lca(Path("/foo/bar"), Path("/foo/bar/baz")) == Path("/foo"));
	CHECK(Path::lca(Path("/foo/bar/quux"), Path("/foo/bar/baz")) == Path("/foo/bar"));

	CHECK(!Path::is_valid(""));
	CHECK(!Path::is_valid("hello"));
	CHECK(!Path::is_valid("/foo/bar/"));
	CHECK(!Path::is_valid("/foo//bar"));
	CHECK(!Path::is_valid("/foo/bar/d*s"));
	CHECK(Path::is_valid("/"));
	CHECK(!Path::is_valid("/foo/3foo/bar"));

	CHECK(Path::descendant_comparator(Path("/"), Path("/")));
	CHECK(Path::descendant_comparator(Path("/"), Path("/foo")));
	CHECK(Path::descendant_comparator(Path("/foo"), Path("/foo/bar")));
	CHECK(Path::descendant_comparator(Path("/foo"), Path("/foo")));
	CHECK(Path::descendant_comparator(Path("/"), Path("/")));
	CHECK(!Path::descendant_comparator(Path("/baz"), Path("/")));
	CHECK(!Path::descendant_comparator(Path("/foo"), Path("/bar")));
	CHECK(!Path::descendant_comparator(Path("/foo/bar"), Path("/foo")));

	CHECK(!Symbol::is_valid(""));
	CHECK(!Symbol::is_valid("/I/have/slashes"));
	CHECK(!Symbol::is_valid("!illegalchar"));
	CHECK(!Symbol::is_valid("0illegalleadingdigit"));
	CHECK(strcmp(Symbol::symbolify("").c_str(), ""));

	CHECK(Path("/foo").child(Symbol("bar")) == "/foo/bar");
	CHECK(Path("/foo").child(Path("/bar/baz")) == "/foo/bar/baz");
	CHECK(Path("/foo").child(Path("/")) == "/foo");

	CHECK(!strcmp(Path("/foo").symbol(), "foo"));
	CHECK(!strcmp(Path("/foo/bar").symbol(), "bar"));
	CHECK(!strcmp(Path("/").symbol(), ""));

	Path original(std::string("/foo/bar"));
	CHECK(original == Path(original));

	bool valid = true;
	try {
		Path path("/ends/in/slash/");
	} catch (const Path::BadPath& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		valid = false;
	}
	CHECK(!valid);

	valid = true;
	try {
		Path path(std::string("/has//double/slash"));
	} catch (const Path::BadPath& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		valid = false;
	}
	CHECK(!valid);

	return 0;
}
