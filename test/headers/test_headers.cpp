/*
  Copyright 2022 David Robillard <d@drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "raul/Array.hpp"        // IWYU pragma: keep
#include "raul/Deletable.hpp"    // IWYU pragma: keep
#include "raul/DoubleBuffer.hpp" // IWYU pragma: keep
#include "raul/Exception.hpp"    // IWYU pragma: keep
#include "raul/Maid.hpp"         // IWYU pragma: keep
#include "raul/Noncopyable.hpp"  // IWYU pragma: keep
#include "raul/Path.hpp"         // IWYU pragma: keep
#include "raul/RingBuffer.hpp"   // IWYU pragma: keep
#include "raul/Semaphore.hpp"    // IWYU pragma: keep
#include "raul/Symbol.hpp"       // IWYU pragma: keep

#ifndef _WIN32
#  include "raul/Process.hpp" // IWYU pragma: keep
#  include "raul/Socket.hpp"  // IWYU pragma: keep
#endif

#if defined(__GNUC__)
__attribute__((const))
#endif
int
main()
{
  return 0;
}
