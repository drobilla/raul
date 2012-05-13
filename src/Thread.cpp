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

#define LOG(s) (s("[")(_name)("] "))

using std::endl;

namespace Raul {

struct ThreadImpl {
	pthread_t pthread;
};

static pthread_once_t s_thread_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t  s_thread_key;

static void thread_key_alloc() {
	pthread_key_create(&s_thread_key, NULL);
}

Thread::Thread(const std::string& name)
	: _exit_flag(false)
	, _impl(new ThreadImpl())
	, _name(name)
	, _thread_exists(false)
	, _own_thread(true)
{
	pthread_once(&s_thread_key_once, thread_key_alloc);
	pthread_setspecific(s_thread_key, this);
}

/** Must be called from thread */
Thread::Thread(pthread_t thread, const std::string& name)
	: _exit_flag(false)
	, _impl(new ThreadImpl())
	, _name(name)
	, _thread_exists(true)
	, _own_thread(false)
{
	_impl->pthread = thread;
	pthread_once(&s_thread_key_once, thread_key_alloc);
	pthread_setspecific(s_thread_key, this);
}

Thread::~Thread()
{
	stop();
	delete _impl;
}

Thread*
Thread::create_for_this_thread(const std::string& name)
{
	return new Thread(pthread_self(), name);
}

Thread&
Thread::get()
{
	pthread_once(&s_thread_key_once, thread_key_alloc);
	Thread* this_thread = reinterpret_cast<Thread*>(
		pthread_getspecific(s_thread_key));
	if (!this_thread)
		this_thread = create_for_this_thread("");

	return *this_thread;
}

void*
Thread::_static_run(void* thread)
{
	Thread* me = static_cast<Thread*>(thread);
	pthread_setspecific(s_thread_key, thread);
	me->_run();
	me->_thread_exists = false;
	return NULL;
}

/** Launch and start the thread. */
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

/** Stop and terminate the thread. */
void
Thread::stop()
{
	if (_thread_exists) {
		if (_own_thread) {
			_exit_flag = true;
			pthread_cancel(_impl->pthread);
			pthread_join(_impl->pthread, NULL);
		}
		_thread_exists = false;
		LOG(info) << "Exiting thread" << endl;
	}
}

void
Thread::join()
{
	pthread_join(_impl->pthread, NULL);
}

void
Thread::set_scheduling(int policy, unsigned int priority)
{
	sched_param sp;
	sp.sched_priority = priority;
	int result = pthread_setschedparam(_impl->pthread, policy, &sp);
	if (!result) {
		LOG(info) << "Set scheduling policy to ";
		switch (policy) {
			case SCHED_FIFO:  info << "SCHED_FIFO";  break;
			case SCHED_RR:    info << "SCHED_RR";    break;
			case SCHED_OTHER: info << "SCHED_OTHER"; break;
			default:          info << "UNKNOWN";     break;
		}
		info << ", priority " << sp.sched_priority << endl;
	} else {
		LOG(info) << "Unable to set scheduling policy ("
			<< strerror(result) << ")" << endl;
	}
}

} // namespace Raul

