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

#include <iostream>
#include <cstddef>
#include "raul/log.hpp"
#include "raul/List.hpp"

using namespace std;
using namespace Raul;

int
main()
{
#define CHECK(cond) \
	do { if (!(cond)) { \
		error << "Test at " << __FILE__ << ":" << __LINE__ << " failed: " << __STRING(cond) << endl; \
		return 1; \
	} } while (0)

	List<int> l;

	l.push_back(new List<int>::Node(1));
	l.push_back(new List<int>::Node(2));
	l.push_back(new List<int>::Node(3));
	l.push_back(new List<int>::Node(4));
	l.push_back(new List<int>::Node(5));
	l.push_back(new List<int>::Node(6));
	l.push_back(new List<int>::Node(7));
	l.push_back(new List<int>::Node(8));

	/*cout << "List:" << endl;
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i) {
		cout << *i << endl;
	}
	cout << endl;*/

	// Remove 4
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i) {
		if ((*i) == 4) {
			delete l.erase(i);
			break;
		}
	}

	// Check
	int idx = 0;
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i, ++idx) {
		if (idx < 3)
			CHECK(*i == idx + 1);
		else
			CHECK(*i == idx + 2);
	}

	// Remove 1 (head)
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i) {
		if ((*i) == 1) {
			delete l.erase(i);
			break;
		}
	}

	// Check
	idx = 0;
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i, ++idx) {
		if (idx < 2)
			CHECK(*i == idx + 2);
		else
			CHECK(*i == idx + 3);
	}

	// Remove 8 (tail)
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i) {
		if ((*i) == 8) {
			delete l.erase(i);
			break;
		}
	}

	// Check
	idx = 0;
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i, ++idx) {
		if (idx < 2)
			CHECK(*i == idx + 2);
		else if (idx < 4)
			CHECK(*i == idx + 3);
		else
			CHECK(*i == 7);
	}

	// Create, push, erase (should get empty list)
	List<int> r;
	r.push_back(new List<int>::Node(9));
	delete r.erase(r.begin());
	CHECK(r.size() == 0);
	CHECK(r.empty());

	// Appending to an empty list
	l.clear();
	CHECK(l.size() == 0);
	CHECK(l.empty());

	List<int> l2;
	l2.push_back(new List<int>::Node(0));
	l2.push_back(new List<int>::Node(2));
	l2.push_back(new List<int>::Node(4));
	l2.push_back(new List<int>::Node(6));

	l.append(l2);
	idx = 0;
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i, ++idx) {
		CHECK(*i == idx * 2);
	}

	// Appending non-empty lists
	l2.push_back(new List<int>::Node(5));
	l2.push_back(new List<int>::Node(6));
	l2.push_back(new List<int>::Node(7));
	l2.push_back(new List<int>::Node(8));

	l.append(l2);
	idx = 0;
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i, ++idx) {
		if (idx < 4)
			CHECK(*i == idx * 2);
		else
			CHECK(*i == idx + 1);
	}

	// Appending an empty list
	l2.clear();
	l.append(l2);

	idx = 0;
	for (List<int>::iterator i = l.begin(); i != l.end(); ++i, ++idx) {
		if (idx < 4)
			CHECK(*i == idx * 2);
		else
			CHECK(*i == idx + 1);
	}

	return 0;
}
