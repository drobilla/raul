/*
  This file is part of Raul.
  Copyright 2007-2012 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAUL_SEMAPHORE_HPP
#define RAUL_SEMAPHORE_HPP

#include <stdexcept>

#ifdef __APPLE__
#    include <mach/mach.h>
#elif defined(_WIN32)
#    include <windows.h>
#else
#    include <errno.h>
#    include <semaphore.h>
#    include <time.h>
#endif

namespace Raul {

/**
   Unnamed (process local) counting semaphore.

   The civilized person's synchronisation primitive.  A counting semaphore is
   an integer which is always non-negative, so, an attempted decrement (or
   "wait") will block if the value is 0, until another thread does an increment
   (or "post").

   At least on Lignux, the main advantage of this is that it is fast and the
   only safe way to reliably signal from a real-time audio thread.  The
   counting semantics also complement ringbuffers of events nicely.
*/
class Semaphore
{
public:
	/**
	   Create a new semaphore.

	   Chances are you want 1 wait() per 1 post(), an initial value of 0.
	*/
	inline Semaphore(unsigned initial) {
		if (!init(initial)) {
			throw std::runtime_error("Failed to create semaphore");
		}
	}

	inline ~Semaphore() {
		destroy();
	}

	/** Destroy and reset to a new initial value. */
	inline void reset(unsigned initial) {
		destroy();
		init(initial);
	}

	/** Post/Increment/Signal */
	inline void post();

	/** Wait/Decrement.  Return false on error. */
	inline bool wait();

	/** Attempt Wait/Decrement.  Return true iff decremented. */
	inline bool try_wait();

	/** Wait for at most @p ms milliseconds.  Return true iff decremented. */
	inline bool timed_wait(unsigned ms);

private:
	inline bool init(unsigned initial);
	inline void destroy();

#if defined(__APPLE__)
	semaphore_t _sem;  // sem_t is a worthless broken mess on OSX
#elif defined(_WIN32)
	HANDLE _sem;  // types are overrated anyway
#else
	sem_t _sem;
#endif
};

#ifdef __APPLE__

inline bool
Semaphore::init(unsigned initial)
{
	if (semaphore_create(mach_task_self(), &_sem, SYNC_POLICY_FIFO, 0)) {
		return false;
	}
	return true;
}

inline void
Semaphore::destroy()
{
	semaphore_destroy(mach_task_self(), _sem);
}

inline void
Semaphore::post()
{
	semaphore_signal(_sem);
}

inline bool
Semaphore::wait()
{
	if (semaphore_wait(_sem) != KERN_SUCCESS) {
		return false;
	}
	return true;
}

inline bool
Semaphore::try_wait()
{
	const mach_timespec_t zero = { 0, 0 };
	return semaphore_timedwait(_sem, zero) == KERN_SUCCESS;
}

inline bool
Semaphore::timed_wait(unsigned ms)
{
	const unsigned seconds = ms / 1000;
	const mach_timespec_t t = { seconds, (ms - (seconds * 1000)) * 1000000 };
	return semaphore_timedwait(_sem, t) == KERN_SUCCESS;
}

#elif defined(_WIN32)

inline bool
Semaphore::init(unsigned initial)
{
	if (!(_sem = CreateSemaphore(NULL, initial, LONG_MAX, NULL))) {
		return false;
	}
	return true;
}

inline void
Semaphore::destroy()
{
	CloseHandle(_sem);
}

inline void
Semaphore::post()
{
	ReleaseSemaphore(_sem, 1, NULL);
}

inline bool
Semaphore::wait()
{
	if (WaitForSingleObject(_sem, INFINITE) != WAIT_OBJECT_0) {
		return false;
	}
	return true;
}

inline bool
Semaphore::try_wait()
{
	return WaitForSingleObject(_sem, 0) == WAIT_OBJECT_0;
}

inline bool
Semaphore::timed_wait(unsigned ms)
{
	return WaitForSingleObject(_sem, ms) == WAIT_OBJECT_0;
}

#else  /* !defined(__APPLE__) && !defined(_WIN32) */

inline bool
Semaphore::init(unsigned initial)
{
	if (sem_init(&_sem, 0, initial)) {
		return false;
	}
	return true;
}

inline void
Semaphore::destroy()
{
	sem_destroy(&_sem);
}

inline void
Semaphore::post()
{
	sem_post(&_sem);
}

inline bool
Semaphore::wait()
{
	while (sem_wait(&_sem)) {
		if (errno != EINTR) {
			return false;  // We are all doomed
		}
		/* Otherwise, interrupted (rare/weird), so try again. */
	}

	return true;
}

inline bool
Semaphore::try_wait()
{
	return (sem_trywait(&_sem) == 0);
}

inline bool
Semaphore::timed_wait(unsigned ms)
{
	const unsigned seconds = ms / 1000;

	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	struct timespec delta = { seconds, (ms - (seconds * 1000)) * 1000000 };
	struct timespec end   = { now.tv_sec + delta.tv_sec,
	                          now.tv_nsec + delta.tv_nsec };
	if (end.tv_nsec >= 1000000000L) {
		++end.tv_sec;
		end.tv_nsec -= 1000000000L;
	}
	return (sem_timedwait(&_sem, &end) == 0);
}

#endif

} // namespace Raul

#endif // RAUL_SEMAPHORE_HPP
