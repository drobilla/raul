/*
  This file is part of Raul.
  Copyright 2012 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAUL_BARRIER_HPP
#define RAUL_BARRIER_HPP

#if defined(__APPLE__)
#include <libkern/OSAtomic.h>

namespace Raul {
static inline void barrier() {
	OSMemoryBarrier();
}
}

#elif defined(_WIN32)
#include <windows.h>

namespace Raul {
static inline void barrier() {
	MemoryBarrier();
}
}

#elif (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)

namespace Raul {
static inline void barrier() {
	__sync_synchronize();
}
}

#else
#pragma message("warning: No memory barriers, possible SMP bugs")

namespace Raul {
static inline void barrier() {
}
}

#endif

#endif // RAUL_BARRIER_HPP
