/*
  This file is part of Raul.
  Copyright 2007-2015 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAUL_PROCESS_HPP
#define RAUL_PROCESS_HPP

#include "raul/Noncopyable.hpp"

#include <sys/resource.h>
#include <unistd.h>

namespace Raul {

/** A child process.
 *
 * \ingroup raul
 */
class Process : Noncopyable
{
public:
	/** Launch a sub process.
	 *
	 * @param argv List of arguments, where argv[0] is the command name.
	 * @return True on success.
	 */
	static bool launch(const char* const argv[]) {
		// Use the same double fork() trick as JACK to prevent zombie children
		const int child = fork();

		if (child == 0) {
			// (in child)

			// Close all nonstandard file descriptors
			struct rlimit max_fds{};
			getrlimit(RLIMIT_NOFILE, &max_fds);
			for (rlim_t fd = 3; fd < max_fds.rlim_cur; ++fd) {
				close(static_cast<int>(fd));
			}

			// Fork child
			const int grandchild = fork();
			switch (grandchild) {
			case 0:
				// (in grandchild)
				setsid();
				execvp(argv[0], const_cast<char*const*>(argv));
				_exit(-1);

			case -1:
				// Fork failed, there is no grandchild
				_exit(-1);

			default:
				// Fork succeeded, return grandchild PID
				_exit(grandchild);
			}
		} else if (child < 0) {
			// Fork failed, there is no child
			return false;
		}

		return true;
	}

private:
	Process() = default;
};

} // namespace Raul

#endif // RAUL_PROCESS_HPP
