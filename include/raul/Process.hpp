// Copyright 2007-2015 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAUL_PROCESS_HPP
#define RAUL_PROCESS_HPP

#include "raul/Noncopyable.hpp"

#include <sys/resource.h>
#include <unistd.h>

namespace raul {

/**
   A child process.

   @ingroup raul
*/
class Process : Noncopyable
{
public:
  /**
     Launch a sub process.

     @param argv List of arguments, where argv[0] is the command name.
     @return True on success.
  */
  static bool launch(const char* const argv[])
  {
    // Use the same double fork() trick as JACK to prevent zombie children
    const int child = fork();

    if (child == 0) {
      // (in child)

      // Close all nonstandard file descriptors
      rlimit max_fds{};
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
        execvp(argv[0], const_cast<char* const*>(argv));
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

} // namespace raul

#endif // RAUL_PROCESS_HPP
