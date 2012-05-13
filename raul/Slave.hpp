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

#ifndef RAUL_SLAVE_HPP
#define RAUL_SLAVE_HPP

#include "raul/Semaphore.hpp"
#include "raul/Thread.hpp"

namespace Raul {

/** Thread driven by (realtime safe) signals.
 *
 * Use this to perform some task in a separate thread you want to 'drive'
 * from a realtime (or otherwise) thread.
 *
 * \ingroup raul
 */
class Slave : public Thread
{
public:
	Slave() : _whip(0) {}

	/** Tell the slave to do whatever work it does.  Realtime safe. */
	inline void whip() { _whip.post(); }

protected:
	/** Worker method.
	 *
	 * This is called once from this thread every time whip() is called.
	 * Implementations likely want to put a single (non loop) chunk of code
	 * here, e.g. to process an event.
	 */
	virtual void _whipped() = 0;

	Semaphore _whip;

private:
	inline void _run() {
		while (true) {
			_whip.wait();
			_whipped();
		}
	}
};

} // namespace Raul

#endif // RAUL_SLAVE_HPP
