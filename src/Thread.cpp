/* This file is part of Raul.
 * Copyright (C) 2007-2009 David Robillard <http://drobilla.net>
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

#include <cstring>
#include <string>

#include "raul/log.hpp"
#include "raul/Thread.hpp"

#define LOG(s) s << "[" << _name << "] "

using std::endl;

namespace Raul {

/* Thread-specific data key (once-only initialized) */
pthread_once_t Thread::_thread_key_once = PTHREAD_ONCE_INIT;
pthread_key_t  Thread::_thread_key;


Thread::Thread(const std::string& name)
	: _exit_flag(false)
	, _name(name)
	, _pthread_exists(false)
	, _own_thread(true)
{
	pthread_once(&_thread_key_once, thread_key_alloc);
	pthread_setspecific(_thread_key, this);
}


/** Must be called from thread */
Thread::Thread(pthread_t thread, const std::string& name)
	: _exit_flag(false)
	, _name(name)
	, _pthread_exists(true)
	, _own_thread(false)
	, _pthread(thread)
{
	pthread_once(&_thread_key_once, thread_key_alloc);
	pthread_setspecific(_thread_key, this);
}


/** Return the calling thread.
 * The return value of this should NOT be cached unless the thread is
 * explicitly user created with create().
 */
Thread&
Thread::get()
{
	Thread* this_thread = reinterpret_cast<Thread*>(pthread_getspecific(_thread_key));
	if (!this_thread)
		this_thread = new Thread(); // sets thread-specific data

	return *this_thread;
}


void*
Thread::_static_run(void* thread)
{
	Thread* me = static_cast<Thread*>(thread);
	pthread_setspecific(me->_thread_key, thread);
	me->_run();
	me->_pthread_exists = false;
	return NULL;
}


/** Launch and start the thread. */
void
Thread::start()
{
	if (!_pthread_exists) {
		LOG(info) << "Starting thread" << endl;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, 1500000);

		pthread_create(&_pthread, &attr, _static_run, this);
		_pthread_exists = true;
	}
}

/** Stop and terminate the thread. */
void
Thread::stop()
{
	if (_pthread_exists) {
		if (_own_thread) {
			_exit_flag = true;
			pthread_cancel(_pthread);
			pthread_join(_pthread, NULL);
		}
		_pthread = NULL;
		_pthread_exists = false;
		LOG(info) << "Exiting thread" << endl;
	}
}

void
Thread::set_scheduling(int policy, unsigned int priority)
{
	sched_param sp;
	sp.sched_priority = priority;
	int result = pthread_setschedparam(_pthread, policy, &sp);
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

