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

#include <set>
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

	static Thread* create(const std::string& name="")
		{ return new Thread(name); }

	static Thread* create_for_this_thread(const std::string& name="");

	/** Return the calling thread.
	 *
	 * If the calling thread does not yet have a Thread object associated with
	 * it, one will be created.
	 */
	static Thread& get();

	virtual void start();
	virtual void stop();

	virtual void join();

	void set_scheduling(int policy, unsigned int priority);

	const std::string& name() const { return _name; }
	void set_name(const std::string& name) { _name = name; }

	bool is_context(unsigned context) const { return _contexts.find(context) != _contexts.end(); }
	void set_context(unsigned context) { _contexts.insert(context); }

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

	bool _exit_flag;

private:
	static void* _static_run(void* me);

	ThreadImpl*        _impl;
	std::set<unsigned> _contexts;
	std::string        _name;
	bool               _thread_exists;
	bool               _own_thread;
};

} // namespace Raul

#endif // RAUL_THREAD_HPP
