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

#include "raul/Atom.hpp"

using namespace std;
using namespace Raul;

int
main()
{
	//static const size_t buf_size = 9;
	//char buf[buf_size] = "atomtest";

	Forge forge;

	Atom nil_atom = forge.make();
	Atom int_atom = forge.make(42);
	Atom float_atom = forge.make(42.0f);
	Atom bool_atom = forge.make(true);
	Atom string_atom = forge.alloc("hello");
	//Atom blob_atom = forge.alloc("http://example.org/atomtype", buf_size, buf);

	return 0;
}

