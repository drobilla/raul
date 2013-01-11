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

#include <atomic>
#include <iostream>

#include "raul/Semaphore.hpp"
#include "raul/Thread.hpp"
#include "raul/ThreadVar.hpp"

using namespace std;
using namespace Raul;

Raul::ThreadVar<int> var(0);
std::atomic<int>     n_errors(0);

class Waiter : public Raul::Thread {
public:
	Waiter(Semaphore& sem) : Raul::Thread(), _sem(sem) {
		if (set_scheduling(true, 10)) {
			cerr << "Set priority on non-existent thread" << endl;
		}
	}

private:
	void _run() {
		if (!set_scheduling(true, 10)) {
			cerr << "Failed to set priority" << endl;
		}
		var = 41;
		cout << "[Waiter] Waiting for signal..." << endl;
		_sem.wait();
		cout << "[Waiter] Received signal, exiting" << endl;
		var = 42;
		if (var != 42) {
			cerr << "[Waiter] var != 42" << endl;
			++n_errors;
		}
	}

	Semaphore& _sem;
};

int
main()
{
	Semaphore sem(0);
	Waiter waiter(sem);
	waiter.start();

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
