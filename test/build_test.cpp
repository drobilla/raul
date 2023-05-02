// Copyright 2007-2017 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "raul/Array.hpp"
#include "raul/Deletable.hpp"
#include "raul/DoubleBuffer.hpp"
#include "raul/Exception.hpp"
#include "raul/Maid.hpp"
#include "raul/Noncopyable.hpp"
#include "raul/Path.hpp"
#include "raul/RingBuffer.hpp"
#include "raul/Semaphore.hpp"
#include "raul/Symbol.hpp"

#ifndef _WIN32
#  include "raul/Process.hpp"
#  include "raul/Socket.hpp"
#endif

class DeletableThing : public raul::Deletable
{};

class NonCopyableThing : public raul::Noncopyable
{};

int
main()
{
  const raul::Array<int>        array;
  const DeletableThing          deletable;
  const raul::DoubleBuffer<int> double_buffer(0);
  const raul::Maid              maid;
  const NonCopyableThing        non_copyable;
  const raul::Path              path;
  const raul::RingBuffer        ring_buffer(64U);
  const raul::Semaphore         semaphore(0U);
  const raul::Symbol            symbol("foo");

  try {
    const raul::Symbol bad_symbol("not a valid symbol!");
    (void)bad_symbol;
  } catch (const raul::Exception&) {
  }

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#  if !defined(_FORTIFY_SOURCE)
  const char* cmd[] = {"echo", NULL};
  raul::Process::launch(cmd);
#  endif

  const raul::Socket socket(raul::Socket::Type::UNIX);

  (void)socket;

#endif

  (void)array;
  (void)deletable;
  (void)double_buffer;
  (void)maid;
  (void)non_copyable;
  (void)path;
  (void)ring_buffer;
  (void)symbol;

  return 0;
}
