/* This file is part of Raul.
 * Copyright 2007-2011 David Robillard <http://drobilla.net>
 *
 * Raul is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Raul is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RAUL_SEMAPHORE_HPP
#define RAUL_SEMAPHORE_HPP

#ifdef __APPLE__
#include <limits.h>
#include <CoreServices/CoreServices.h>
#else
#include <semaphore.h>
#endif

#include <boost/utility.hpp>

namespace Raul {

/** Counting semaphore.
 *
 * \ingroup raul
 */
class Semaphore : boost::noncopyable {
public:
	inline Semaphore(unsigned int initial) {
		#ifdef __APPLE__
		MPCreateSemaphore(UINT_MAX, initial, &_sem);
		#else
		sem_init(&_sem, 0, initial);
		#endif
	}

	inline ~Semaphore() {
		#ifdef __APPLE__
		MPDeleteSemaphore(_sem);
		#else
		sem_destroy(&_sem);
		#endif
	}

	/** Destroy and reset the semaphore to an initial value.
	 *
	 * This must not be called while there are any waiters.
	 */
	inline void reset(unsigned int initial) {
		#ifdef __APPLE__
		MPDeleteSemaphore(_sem);
		MPCreateSemaphore(UINT_MAX, initial, &_sem);
		#else
		sem_destroy(&_sem);
		sem_init(&_sem, 0, initial);
		#endif
	}

	/** Increment (and signal any waiters).
	 *
	 * Realtime safe.
	 */
	inline void post() {
		#ifdef __APPLE__
		MPSignalSemaphore(_sem);
		#else
		sem_post(&_sem);
		#endif
	}

	/** Wait until count is > 0, then decrement.
	 *
	 * Obviously not realtime safe.
	 */
	inline void wait() {
		#ifdef __APPLE__
		MPWaitOnSemaphore(_sem, kDurationForever);
		#else
		/* Note that sem_wait always returns 0 in practice, except in
		   gdb (at least), where it returns nonzero, so the while is
		   necessary (and is the correct/safe solution in any case).
		*/
		while (sem_wait(&_sem) != 0) {}
		#endif
	}

	/** Non-blocking version of wait().
	 *
	 * \return true if decrement was successful (lock was acquired).
	 *
	 * Realtime safe?
	 */
	inline bool try_wait() {
		#ifdef __APPLE__
		return MPWaitOnSemaphore(_sem, kDurationImmediate) == noErr;
 		#else
		return (sem_trywait(&_sem) == 0);
		#endif
	}

private:
	#ifdef __APPLE__
	MPSemaphoreID _sem;
	#else
	sem_t _sem;
	#endif
};

} // namespace Raul

#endif // RAUL_SEMAPHORE_HPP
