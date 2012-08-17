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

#include <cassert>
#include <iostream>

#include "raul/URI.hpp"

using namespace std;
using namespace Raul;

int
main()
{
#define CHECK(cond) \
	do { if (!(cond)) { \
		cerr << "Test failed: " << (cond) << endl; \
		assert(0); \
		return 1; \
	} } while (0)

	CHECK(URI("http://example.org").scheme() == "http");
	CHECK(URI("svn+ssh://example.org/").scheme() == "svn+ssh");
	CHECK(URI("osc.udp://example.org/").scheme() == "osc.udp");
	CHECK(URI("weird-scheme://example.org/").scheme() == "weird-scheme");

	URI original(std::string("http://example.org"));
	URI copy(original);
	CHECK(original == copy);

	bool valid = true;
	try {
		URI uri("not/a/uri");
	} catch (const URI::BadURI& e) {
		valid = false;
	}
	CHECK(!valid);

	valid = true;
	try {
		URI uri(std::string("/this/is/a/path"));
	} catch (const URI::BadURI& e) {
		valid = false;
	}
	CHECK(!valid);

	return 0;
}
