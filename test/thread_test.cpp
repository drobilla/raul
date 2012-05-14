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
#include "raul/Thread.hpp"
#include "raul/Semaphore.hpp"

using namespace std;
using namespace Raul;

class Waiter : public Raul::Thread {
public:
	Waiter(Semaphore& sem) : Raul::Thread("Waiter"), _sem(sem) {
	}

private:
	void _run() {
		cout << "[Waiter] Waiting for signal..." << endl;
		_sem.wait();
		cout << "[Waiter] Received signal, exiting" << endl;
	}

	Semaphore& _sem;
};

int
main()
{
	Semaphore sem(0);
	Waiter waiter(sem);
	waiter.start();

	cout << "[Main] Signalling..." << endl;
	sem.post();

	cout << "[Main] Waiting for waiter..." << endl;
	waiter.join();

	cout << "[Main] Exiting" << endl;

	return 0;
}
