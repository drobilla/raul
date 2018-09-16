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

#undef NDEBUG

#include "raul/Semaphore.hpp"

#include <cassert>
#include <chrono>
#include <thread>

static void
wait_for_sem(Raul::Semaphore* sem)
{
	sem->wait();
}

static void
timed_wait_for_sem(Raul::Semaphore* sem)
{
	while (!sem->timed_wait(std::chrono::milliseconds(100))) {}
}

int
main()
{
	Raul::Semaphore sem(0);
	assert(!sem.try_wait());

	// Check that semaphore wakes up strict waiter
	std::thread waiter(wait_for_sem, &sem);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	sem.post();
	waiter.join();

	// Check that semaphore wakes up timed waiter
	std::thread timed_waiter(timed_wait_for_sem, &sem);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	sem.post();
	timed_waiter.join();

	// Check that timed_wait actually waits
	const auto start = std::chrono::steady_clock::now();
	sem.timed_wait(std::chrono::milliseconds(100));
	const auto end = std::chrono::steady_clock::now();
	assert(end - start > std::chrono::milliseconds(100));
	assert(end - start < std::chrono::milliseconds(200));

	// Check that we can't successfully wait on a zero semaphore
	assert(!sem.timed_wait(std::chrono::milliseconds(100)));

	// Check that initial value works correctly
	Raul::Semaphore sem2(2);
	assert(sem2.wait());
	assert(sem2.wait());
	assert(!sem2.try_wait());

	return 0;
}
