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

#include "raul/Symbol.hpp"

#include <cassert>
#include <iostream>
#include <list>
#include <string>

int
main()
{
  using Symbol = raul::Symbol;

  std::list<std::string> names;
  names.emplace_back("Dry/Wet Balance");
  names.emplace_back("foo+1bar(baz)");
  names.emplace_back("ThisCRAR");
  names.emplace_back("NAME");
  names.emplace_back("thing with a bunch of spaces");
  names.emplace_back("thing-with-a-bunch-of-dashes");
  names.emplace_back("CamelCaseABC");
  names.emplace_back("Signal Level [dB]");
  names.emplace_back("Gain dB");
  names.emplace_back("Dry/Wet Balance");
  names.emplace_back("Phaser1 - Similar to CSound's phaser1 by Sean Costello");
  names.emplace_back("1");
  names.emplace_back("");

  for (const auto& name : names) {
    assert(!Symbol::symbolify(name).empty());
  }

  const Symbol original("sym");
  assert(original == original);

  bool valid = true;
  try {
    Symbol symbol("0startswithdigit");
  } catch (const Symbol::BadSymbol& e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    valid = false;
  }
  assert(!valid);

  valid = true;
  try {
    Symbol symbol(std::string("invalid/symbol"));
  } catch (const Symbol::BadSymbol& e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    valid = false;
  }
  assert(!valid);

  return 0;
}
