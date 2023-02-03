// Copyright 2007-2019 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#undef NDEBUG

#include "raul/Array.hpp"

#include <cassert>
#include <cstdio>

int
main()
{
  raul::Array<int> array1(32, 2);

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

  const raul::Array<int> array2{array1};
  for (size_t i = 0; i < array1.size(); ++i) {
    assert(array2[i] == array1[i]);
  }

  const raul::Array<int> array3{8, 47};
  assert(array3[0] == 47);
  assert(array3.size() == 8);

  return 0;
}
