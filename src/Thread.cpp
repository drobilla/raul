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

#include <cstring>
#include <string>

#include <pthread.h>

#include "raul/log.hpp"
#include "raul/Thread.hpp"
#include "raul/ThreadVar.hpp"

#define LOG(s) (s("[")(_name)("] "))

using std::endl;

namespace Raul {

struct ThreadImpl {
	pthread_t pthread;
};

Thread::Thread(const std::string& name)
	: _impl(new ThreadImpl())
	, _name(name)
	, _thread_exists(false)
	, _exit_flag(false)
{
}

Thread::~Thread()
{
	stop();
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
		LOG(info) << "Starting thread" << endl;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, 1500000);

		pthread_create(&_impl->pthread, &attr, _static_run, this);
		_thread_exists = true;
	}
}

void
Thread::stop()
{
	if (_thread_exists) {
		_exit_flag = true;
		pthread_cancel(_impl->pthread);
		pthread_join(_impl->pthread, NULL);
		_thread_exists = false;
		LOG(info) << "Exiting thread" << endl;
	}
}

void
Thread::join()
{
	pthread_join(_impl->pthread, NULL);
}

bool
Thread::set_scheduling(bool realtime, unsigned priority)
{
	sched_param sp;
	sp.sched_priority = priority;
	const int policy = realtime ? SCHED_FIFO : SCHED_OTHER;
	const int result = pthread_setschedparam(_impl->pthread, policy, &sp);
	if (!result) {
		LOG(info) << (fmt("Set scheduling policy to %1% priority %2%\n")
		              % (realtime ? "realtime" : "normal")
		              % sp.sched_priority);
	} else {
		LOG(warn) << (fmt("Unable to set scheduling policy (%1%)\n")
		              % strerror(result));
	}
	return !result;
}

} // namespace Raul

