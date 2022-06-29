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
  raul::Array<int>        array;
  DeletableThing          deletable;
  raul::DoubleBuffer<int> double_buffer(0);
  raul::Maid              maid;
  NonCopyableThing        non_copyable;
  raul::Path              path;
  raul::RingBuffer        ring_buffer(64U);
  raul::Semaphore         semaphore(0U);
  raul::Symbol            symbol("foo");

  try {
    raul::Symbol bad_symbol("not a valid symbol!");
    (void)bad_symbol;
  } catch (const raul::Exception&) {
  }

#ifndef _WIN32
  const char* cmd[] = {"echo"};
  raul::Process::launch(cmd);

  raul::Socket socket(raul::Socket::Type::UNIX);

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
