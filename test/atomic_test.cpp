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

#include "raul/AtomicInt.hpp"
#include "raul/AtomicPtr.hpp"

using namespace std;
using namespace Raul;

#define TEST(cond) { if (!(cond)) return 1; }

int
main()
{
	/* TODO: Actually test functionality... */
	AtomicInt i(0);
	TEST(i == 0);

	AtomicInt j(i);
	TEST(i == j);

	++i;
	TEST(i == 1);

	--i;
	TEST(i == 0);

	TEST(i + 1 == 1);
	TEST(i - 1 == -1);

	i += 2;
	TEST(i == 2);

	i -= 4;
	TEST(i == -2);

	TEST(i.compare_and_exchange(-2, 0));
	TEST(i == 0);

	TEST(i.exchange_and_add(5) == 0);
	TEST(i == 5);

	i = 1;
	TEST(i == 1);
	TEST(i.decrement_and_test());

	int five  = 5;
	int seven = 7;
	AtomicPtr<int> p;
	TEST(p.get() == NULL);

	AtomicPtr<int> q(p);
	TEST(p == q);

	p = &five;
	TEST(p.get() == &five);
	TEST(*p.get() == 5);
	TEST(p.compare_and_exchange(&five, &seven));
	TEST(p.get() == &seven);
	TEST(*p.get() = 7);

	return 0;
}
