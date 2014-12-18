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

#include <limits.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "raul/Semaphore.hpp"

using namespace std;
using namespace Raul;

static void
wait_for_sem(Semaphore* sem)
{
	while (true) {
		if (sem->timed_wait(250)) {
			cout << "[Waiter] Received signal" << endl;
			break;
		} else {
			cout << "[Waiter] Timed out" << endl;
		}
	}
	cout << "[Waiter] Exiting" << endl;
}

int
main()
{
	try {
		Semaphore fail(UINT_MAX);
	} catch (...) {
	}

	Semaphore sem(0);

	if (sem.try_wait()) {
		cerr << "Successfully try-waited a 0 Semaphore" << endl;
		return 1;
	}

	std::thread waiter(wait_for_sem, &sem);

	sleep(1);

	cout << "[Main] Signalling..." << endl;
	sem.post();

	waiter.join();
	cout << "[Main] Exiting" << endl;

	return 0;
}
