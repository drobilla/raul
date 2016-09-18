/*
  This file is part of Raul.
  Copyright 2007-2016 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

#include "raul/Array.hpp"

int
main(int argc, char** argv)
{
	Raul::Array<int> array1(32, 2);

	array1[0] = 42;
	if (array1[0] != 42) {
		fprintf(stderr, "array1[0] != 42\n");
		return 1;
	} else if (array1[1] != 2) {
		fprintf(stderr, "array1[1] != 2\n");
		return 1;
	} else if (array1.size() != 32) {
		fprintf(stderr, "array1.size() != 1\n");
		return 1;
	}

	array1.alloc(16, 0);
	if (array1[0] != 0) {
		fprintf(stderr, "array1[0] != 0\n");
		return 1;
	} else if (array1.at(0) != 0) {
		fprintf(stderr, "array1.at(0) != 0\n");
		return 1;
	} else if (array1.size() != 16) {
		fprintf(stderr, "array1.size() != 16\n");
		return 1;
	}

	array1.alloc(8, 0);
	if (array1.size() != 8) {
		fprintf(stderr, "array1.size() != 8\n");
		return 1;
	}

	Raul::Array<int> array2(array1);
	for (size_t i = 0; i < array1.size(); ++i) {
		if (array2[i] != array1[i]) {
			fprintf(stderr, "Mismatch at %zu\n", i);
			return 1;
		}
	}

	Raul::Array<int> array3(8, 47);
	if (array3[0] != 47) {
		fprintf(stderr, "array3[0] != 47\n");
		return 1;
	} else if (array3.size() != 8) {
		fprintf(stderr, "array3.size() != 8\n");
		return 1;
	}

	return 0;
}
