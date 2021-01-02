/*
  Copyright 2007-2019 David Robillard <d@drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#undef NDEBUG

#include "raul/Socket.hpp"

#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

int
main()
{
  using Socket = raul::Socket;

  std::string unix_uri("unix:///tmp/raul_test_sock");
  std::string tcp_uri("tcp://127.0.0.1:12345");

  Socket unix_server_sock(Socket::Type::UNIX);
  Socket tcp_server_sock(Socket::Type::TCP);
  assert(unix_server_sock.bind(unix_uri));
  assert(unix_server_sock.listen());
  assert(tcp_server_sock.bind(tcp_uri));
  assert(tcp_server_sock.listen());

  const pid_t child_pid = fork();
  if (child_pid) {
    // This is the parent (server) process
    int status = 0;
    waitpid(child_pid, &status, 0);

    struct pollfd pfds[2];
    pfds[0].fd      = unix_server_sock.fd();
    pfds[0].events  = POLLIN;
    pfds[0].revents = 0;
    pfds[1].fd      = tcp_server_sock.fd();
    pfds[1].events  = POLLIN;
    pfds[1].revents = 0;

    unsigned n_received = 0;
    while (n_received < 2) {
      const int ret = poll(pfds, 2, -1);
      assert(ret != -1);

      if ((pfds[0].revents & POLLHUP) || pfds[1].revents & POLLHUP) {
        break;
      }

      if (ret == 0) {
        fprintf(stderr, "poll returned with no data\n");
        continue;
      }

      if (pfds[0].revents & POLLIN) {
        std::shared_ptr<Socket> conn = unix_server_sock.accept();
        ++n_received;
      }

      if (pfds[1].revents & POLLIN) {
        std::shared_ptr<Socket> conn = tcp_server_sock.accept();
        ++n_received;
      }
    }

    unix_server_sock.shutdown();
    tcp_server_sock.shutdown();
    unlink("/tmp/raul_test_sock");
    fprintf(stderr, "n received: %u\n", n_received);
    return n_received != 2;
  }

  // This is the child (client) process
  raul::Socket unix_sock(Socket::Type::UNIX);
  raul::Socket tcp_sock(Socket::Type::TCP);

  assert(unix_sock.connect(unix_uri));
  assert(tcp_sock.connect(tcp_uri));

  return 0;
}
