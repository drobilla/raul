/*
  This file is part of Raul.
  Copyright 2012 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "raul/DoubleBuffer.hpp"

using namespace std;
using namespace Raul;

int
main(int argc, char** argv)
{
	DoubleBuffer<int> db(0);

	if (db.get() != 0) {
		return 1;
	}

	db.set(42);
	if (db.get() != 42) {
		return 1;
	}

	DoubleBuffer<int> db2(db);
	if (db2.get() != 42) {
		return 1;
	}

	db.set(43);
	if (db.get() != 43) {
		return 1;
	} else if (db2.get() != 42) {
		return 1;
	}

	return 0;
}
