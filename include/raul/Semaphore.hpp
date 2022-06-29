/*
  Copyright 2007-2014 David Robillard <d@drobilla.net>

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

#ifdef __APPLE__
#  include <mach/mach.h>
#elif defined(_WIN32)
#  define NOMINMAX
#  include <windows.h>
#else
#  include <cerrno>
#  include <ctime>
#  include <semaphore.h>
#endif

#include <chrono>
#include <stdexcept>

namespace raul {

/**
   Unnamed (process local) counting semaphore.

   The civilized person's synchronisation primitive.  A counting semaphore is
   an integer which is always non-negative, so, an attempted decrement (or
   "wait") will block if the value is 0, until another thread does an increment
   (or "post").

   At least on Lignux, the main advantage of this is that it is fast and the
   only safe way to reliably signal from a real-time audio thread.  The
   counting semantics also complement ringbuffers of events nicely.

   @ingroup raul
*/
class Semaphore
{
public:
  /**
     Create a new semaphore.

     Chances are you want 1 wait() per 1 post(), an initial value of 0.
  */
  explicit Semaphore(unsigned initial)
    : _sem()
  {
    if (!init(initial)) {
      throw std::runtime_error("Failed to create semaphore");
    }
  }

  inline Semaphore(const Semaphore&) = delete;
  inline Semaphore& operator=(const Semaphore&) = delete;

  inline Semaphore(Semaphore&&) = delete;
  inline Semaphore& operator=(Semaphore&&) = delete;

  inline ~Semaphore() { destroy(); }

  /// Destroy and reset to a new initial value
  inline void reset(unsigned initial)
  {
    destroy();
    init(initial);
  }

  /// Post/Increment/Signal
  inline void post();

  /// Wait/Decrement, return false on error
  inline bool wait();

  /// Attempt Wait/Decrement, return true iff decremented
  inline bool try_wait();

  /// Wait for at most `ms` milliseconds, return true iff decremented
  template<class Rep, class Period>
  inline bool timed_wait(const std::chrono::duration<Rep, Period>& wait);

private:
  inline bool init(unsigned initial);
  inline void destroy();

#if defined(__APPLE__)
  semaphore_t _sem; // sem_t is a worthless broken mess on OSX
#elif defined(_WIN32)
  HANDLE _sem; // types are overrated anyway
#else
  sem_t _sem;
#endif
};

#ifdef __APPLE__

inline bool
Semaphore::init(unsigned initial)
{
  if (semaphore_create(
        mach_task_self(), &_sem, SYNC_POLICY_FIFO, int(initial))) {
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
  const mach_timespec_t zero = {0, 0};
  return semaphore_timedwait(_sem, zero) == KERN_SUCCESS;
}

template<class Rep, class Period>
inline bool
Semaphore::timed_wait(const std::chrono::duration<Rep, Period>& wait)
{
  namespace chr = std::chrono;

  const chr::seconds     sec(chr::duration_cast<chr::seconds>(wait));
  const chr::nanoseconds nsec(wait - sec);

  const mach_timespec_t t = {static_cast<unsigned>(sec.count()),
                             static_cast<int>(nsec.count())};

  return semaphore_timedwait(_sem, t) == KERN_SUCCESS;
}

#elif defined(_WIN32)

inline bool
Semaphore::init(unsigned initial)
{
  if (!(_sem = CreateSemaphore(NULL, (LONG)initial, LONG_MAX, NULL))) {
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

template<class Rep, class Period>
inline bool
Semaphore::timed_wait(const std::chrono::duration<Rep, Period>& wait)
{
  namespace chr = std::chrono;

  const chr::milliseconds ms(chr::duration_cast<chr::milliseconds>(wait));
  return WaitForSingleObject(_sem, static_cast<DWORD>(ms.count())) ==
         WAIT_OBJECT_0;
}

#else /* !defined(__APPLE__) && !defined(_WIN32) */

inline bool
Semaphore::init(unsigned initial)
{
  return !sem_init(&_sem, 0, initial);
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
      return false; // We are all doomed
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

template<class Rep, class Period>
inline bool
Semaphore::timed_wait(const std::chrono::duration<Rep, Period>& wait)
{
  namespace chr = std::chrono;

  // Use clock_gettime to ensure sem_timedwait uses the same clock
  timespec time{};
  clock_gettime(CLOCK_REALTIME, &time);

  const auto now(chr::seconds(time.tv_sec) + chr::nanoseconds(time.tv_nsec));
  const auto end(now + wait);

  const chr::seconds end_sec(chr::duration_cast<chr::seconds>(end));
  const chr::nanoseconds end_nsec(end - end_sec);

  const timespec ts_end = {static_cast<time_t>(end_sec.count()),
                           static_cast<long>(end_nsec.count())};

  return (sem_timedwait(&_sem, &ts_end) == 0);
}

#endif

} // namespace raul

#endif // RAUL_SEMAPHORE_HPP
