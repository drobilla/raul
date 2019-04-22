/*
  This file is part of Raul.
  Copyright 2013-2019 David Robillard <http://drobilla.net>

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

#include "raul/DoubleBuffer.hpp"

#include <cassert>

int
main()
{
	Raul::DoubleBuffer<int> db(0);

	assert(db.get() == 0);

	db.set(42);
	assert(db.get() == 42);

	db.set(43);
	assert(db.get() == 43);

	return 0;
}
