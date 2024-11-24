// Copyright 2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <raul/Array.hpp>        // IWYU pragma: keep
#include <raul/Deletable.hpp>    // IWYU pragma: keep
#include <raul/DoubleBuffer.hpp> // IWYU pragma: keep
#include <raul/Exception.hpp>    // IWYU pragma: keep
#include <raul/Maid.hpp>         // IWYU pragma: keep
#include <raul/Noncopyable.hpp>  // IWYU pragma: keep
#include <raul/Path.hpp>         // IWYU pragma: keep
#include <raul/RingBuffer.hpp>   // IWYU pragma: keep
#include <raul/Semaphore.hpp>    // IWYU pragma: keep
#include <raul/Symbol.hpp>       // IWYU pragma: keep

#ifndef _WIN32
#  include <raul/Process.hpp> // IWYU pragma: keep
#  include <raul/Socket.hpp>  // IWYU pragma: keep
#endif

#if defined(__GNUC__)
__attribute__((const))
#endif
int
main()
{
  return 0;
}
