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

#include "raul/Symbol.hpp"

#include <iostream>
#include <list>
#include <string>

using namespace std;
using namespace Raul;

int
main()
{
#define CHECK(cond) \
	do { if (!(cond)) { \
		cerr << "Test failed: " << (cond) << endl; \
		return 1; \
	} } while (0)

	list<string> names;
	names.push_back("Dry/Wet Balance");
	names.push_back("foo+1bar(baz)");
	names.push_back("ThisCRAR");
	names.push_back("NAME");
	names.push_back("thing with a bunch of spaces");
	names.push_back("thing-with-a-bunch-of-dashes");
	names.push_back("CamelCaseABC");
	names.push_back("Signal Level [dB]");
	names.push_back("Gain dB");
	names.push_back("Dry/Wet Balance");
	names.push_back("Phaser1 - Similar to CSound's phaser1 by Sean Costello");
	names.push_back("1");
	names.push_back("");

	for (list<string>::iterator i = names.begin(); i != names.end(); ++i) {
		CHECK(!Symbol::symbolify(*i).empty());
	}

	Symbol original("sym");
	Symbol copy(original);
	CHECK(original == copy);

	bool valid = true;
	try {
		Symbol symbol("0startswithdigit");
	} catch (const Symbol::BadSymbol& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		valid = false;
	}
	CHECK(!valid);

	valid = true;
	try {
		Symbol symbol(std::string("invalid/symbol"));
	} catch (const Symbol::BadSymbol& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		valid = false;
	}
	CHECK(!valid);

	return 0;
}
