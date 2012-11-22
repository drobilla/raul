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

#ifndef RAUL_THREAD_HPP
#define RAUL_THREAD_HPP

#include "raul/Noncopyable.hpp"

namespace Raul {

/** Abstract base class for a thread.
 *
 * Extend this and override the _run method to easily create a thread
 * to perform some task.
 *
 * \ingroup raul
 */
class Thread : Noncopyable
{
public:
	virtual ~Thread() {
		join();
	}

	/** Start the thread if it is not already running.
	 *
	 * This is separate from construction to prevent race conditions during
	 * construction of derived classes.
	 */
	virtual void start() {
		if (!_thread_exists) {
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setstacksize(&attr, 1500000);

			pthread_create(&_pthread, &attr, _static_run, this);
			_thread_exists = true;
		}
	}

	/** Stop the thread and block the caller until the thread exits.
	 *
	 * This sets _exit_flag to true, derived classes must ensure they actually
	 * exit when this occurs.
	 */
	virtual void join() {
		if (_thread_exists) {
			_exit_flag = true;
			pthread_join(_pthread, NULL);
			_thread_exists = false;
		}
	}

	/** Set the scheduling policy for this thread.
	 * @return True on success.
	 */
	virtual bool set_scheduling(bool realtime, unsigned priority) {
		if (_thread_exists) {
			sched_param sp;
			sp.sched_priority = priority;
			const int policy = realtime ? SCHED_FIFO : SCHED_OTHER;
			return !pthread_setschedparam(_pthread, policy, &sp);
		} else {
			return false;
		}
	}

protected:
	/** Construct a thread.
	 *
	 * Note this does not actually start a thread to prevent race conditions
	 * during construction.  To actually begin execution, call start().
	 */
	explicit Thread()
		: _thread_exists(false)
		, _exit_flag(false)
	{}


	/** Thread function to execute.
	 *
	 * This is called once on start, and terminated on stop.
	 * Implementations likely want to put some infinite loop here.
	 *
	 * When _exit_flag becomes true (via a call to stop()) the loop
	 * should exit.
	 */
	virtual void _run() {}

private:
	static void* _static_run(void* thread) {
		Thread* me = static_cast<Thread*>(thread);
		me->_run();
		me->_thread_exists = false;
		return NULL;
	}

	pthread_t _pthread;
	bool      _thread_exists;

protected:
	bool _exit_flag;
};

} // namespace Raul

#endif // RAUL_THREAD_HPP
