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

#include <pthread.h>

#include "raul/Thread.hpp"

namespace Raul {

struct ThreadImpl {
	pthread_t pthread;
};

Thread::Thread()
	: _impl(new ThreadImpl())
	, _thread_exists(false)
	, _exit_flag(false)
{
}

Thread::~Thread()
{
	join();
	delete _impl;
}

void*
Thread::_static_run(void* thread)
{
	Thread* me = static_cast<Thread*>(thread);
	me->_run();
	me->_thread_exists = false;
	return NULL;
}

void
Thread::start()
{
	if (!_thread_exists) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, 1500000);

		pthread_create(&_impl->pthread, &attr, _static_run, this);
		_thread_exists = true;
	}
}

void
Thread::join()
{
	if (_thread_exists) {
		_exit_flag = true;
		pthread_join(_impl->pthread, NULL);
		_thread_exists = false;
	}
}

bool
Thread::set_scheduling(bool realtime, unsigned priority)
{
	sched_param sp;
	sp.sched_priority = priority;
	const int policy = realtime ? SCHED_FIFO : SCHED_OTHER;
	return !pthread_setschedparam(_impl->pthread, policy, &sp);
}

} // namespace Raul

