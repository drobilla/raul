/*
  Copyright 2007-2019 David Robillard <d@drobilla.net>

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

#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

namespace {

using Semaphore = Raul::Semaphore;

thread_local int var(0);
std::atomic<int> n_errors(0);

void
wait_for_sem(Semaphore* sem)
{
  var = 41;
  std::cout << "[Waiter] Waiting for signal..." << std::endl;
  sem->wait();
  std::cout << "[Waiter] Received signal, exiting" << std::endl;
  var = 42;
  assert(var == 42);
}

} // namespace

int
main()
{
  Semaphore   sem(0);
  std::thread waiter(wait_for_sem, &sem);

  var = 24;

  std::cout << "[Main] Signalling..." << std::endl;
  sem.post();

  std::cout << "[Main] Waiting for waiter..." << std::endl;
  waiter.join();

  std::cout << "[Main] Exiting" << std::endl;

  assert(var == 24);

  return n_errors.load();
}
