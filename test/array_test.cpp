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

#include "raul/Array.hpp"

#include <cassert>
#include <cstdio>

int
main()
{
	Raul::Array<int> array1(32, 2);

	array1[0] = 42;
	assert(array1[0] == 42);
	assert(array1[1] == 2);
	assert(array1.size() == 32);

	array1.alloc(16, 0);
	assert(array1[0] == 0);
	assert(array1.at(0) == 0);
	assert(array1.size() == 16);

	array1.alloc(8, 0);
	assert(array1.size() == 8);

	Raul::Array<int> array2(array1);
	for (size_t i = 0; i < array1.size(); ++i) {
		assert(array2[i] == array1[i]);
	}

	Raul::Array<int> array3(8, 47);
	assert(array3[0] == 47);
	assert(array3.size() == 8);

	return 0;
}
