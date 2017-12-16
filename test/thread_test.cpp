/*
  This file is part of Raul.
  Copyright 2007-2017 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <atomic>
#include <iostream>
#include <thread>

#include "raul/Semaphore.hpp"

using namespace std;
using namespace Raul;

namespace {

thread_local int var(0);
std::atomic<int> n_errors(0);

void
wait_for_sem(Semaphore* sem)
{
	var = 41;
	cout << "[Waiter] Waiting for signal..." << endl;
	sem->wait();
	cout << "[Waiter] Received signal, exiting" << endl;
	var = 42;
	if (var != 42) {
		cerr << "[Waiter] var != 42" << endl;
		++n_errors;
	}
}

} // namespace

int
main()
{
	Semaphore   sem(0);
	std::thread waiter(wait_for_sem, &sem);

	var = 24;

	cout << "[Main] Signalling..." << endl;
	sem.post();

	cout << "[Main] Waiting for waiter..." << endl;
	waiter.join();

	cout << "[Main] Exiting" << endl;

	if (var != 24) {
		cerr << "[Main] var != 24" << endl;
		++n_errors;
	}

	return n_errors.load();
}
