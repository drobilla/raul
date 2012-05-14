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

#include <string>

#include "raul/Noncopyable.hpp"

namespace Raul {

struct ThreadImpl;

/** Abstract base class for a thread.
 *
 * Extend this and override the _run method to easily create a thread
 * to perform some task.
 *
 * The current Thread can be accessed using the get() method.
 *
 * \ingroup raul
 */
class Thread : Noncopyable
{
public:
	virtual ~Thread();

	/** Return the calling thread.
	 *
	 * If the calling thread does not yet have a Thread object associated with
	 * it yet, one will be created with the given name.
	 */
	static Thread& get(const std::string& name="");

	/** Start the thread if it is not already running. */
	virtual void start();

	/** Stop the thread if it is running. */
	virtual void stop();

	/** Wait until the thread exits. */
	virtual void join();

	/** Set the scheduling policy for this thread.
	 * @return True on success.
	 */
	virtual bool set_scheduling(bool realtime, unsigned priority);

	/** Return the name of this thread. */
	const std::string& name() const { return _name; }

protected:
	explicit Thread(const std::string& name="");
	Thread(pthread_t thread, const std::string& name="");

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
	static void* _static_run(void* me);

	ThreadImpl* _impl;
	std::string _name;
	bool        _thread_exists;
	bool        _own_thread;

protected:
	bool _exit_flag;
};

} // namespace Raul

#endif // RAUL_THREAD_HPP
