// Copyright 2007-2019 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#undef NDEBUG

#include "raul/Semaphore.hpp"

#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

namespace {

using Semaphore = raul::Semaphore;

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
