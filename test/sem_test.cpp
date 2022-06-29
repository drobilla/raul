// Copyright 2007-2017 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#undef NDEBUG

#include "raul/Semaphore.hpp"

#include <cassert>
#include <chrono>
#include <thread>

static void
wait_for_sem(raul::Semaphore* sem)
{
  sem->wait();
}

static void
timed_wait_for_sem(raul::Semaphore* sem)
{
  while (!sem->timed_wait(std::chrono::milliseconds(100))) {
  }
}

int
main()
{
  raul::Semaphore sem(0);
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
  assert(end - start > std::chrono::milliseconds(80));
  assert(end - start < std::chrono::milliseconds(400));

  // Check that we can't successfully wait on a zero semaphore
  assert(!sem.timed_wait(std::chrono::milliseconds(100)));

  // Check that initial value works correctly
  raul::Semaphore sem2(2);
  assert(sem2.wait());
  assert(sem2.wait());
  assert(!sem2.try_wait());

  return 0;
}
