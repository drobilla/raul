// Copyright 2013-2019 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#undef NDEBUG

#include <raul/DoubleBuffer.hpp>

#include <cassert>

int
main()
{
  raul::DoubleBuffer<int> db(0);

  assert(db.get() == 0);

  db.set(42);
  assert(db.get() == 42);

  db.set(43);
  assert(db.get() == 43);

  return 0;
}
