/*
  This file is part of Raul.
  Copyright 2007-2013 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "raul/Socket.hpp"

using namespace std;
using namespace Raul;

int
main(int argc, char** argv)
{
	Raul::URI unix_uri("unix:///tmp/raul_test_sock");
	Raul::URI tcp_uri("tcp://127.0.0.1:12345");

	Raul::Socket unix_server_sock(Socket::Type::UNIX);
	Raul::Socket tcp_server_sock(Socket::Type::TCP);
	if (!unix_server_sock.bind(unix_uri)) {
		fprintf(stderr, "Failed to bind UNIX server socket (%s)\n",
		        strerror(errno));
		return 1;
	} else if (!unix_server_sock.listen()) {
		fprintf(stderr, "Failed to listen on UNIX server socket (%s)\n",
		        strerror(errno));
		return 1;
	} else if (!tcp_server_sock.bind(tcp_uri)) {
		fprintf(stderr, "Failed to bind TCP server socket (%s)\n",
		        strerror(errno));
		return 1;
	} else if (!tcp_server_sock.listen()) {
		fprintf(stderr, "Failed to listen on TCP server socket (%s)\n",
		        strerror(errno));
		return 1;
	}

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
			if (ret == -1) {
				fprintf(stderr, "poll error (%s)\n", strerror(errno));
				break;
			} else if ((pfds[0].revents & POLLHUP) || pfds[1].revents & POLLHUP) {
				break;
			} else if (ret == 0) {
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
		fprintf(stderr, "n received: %d\n", n_received);
		return n_received != 2;
	}

	// This is the child (client) process
	Raul::Socket unix_sock(Socket::Type::UNIX);
	Raul::Socket tcp_sock(Socket::Type::TCP);

	if (!unix_sock.connect(unix_uri)) {
		fprintf(stderr, "Failed to connect to UNIX socket\n");
		return 1;
	} else if (!tcp_sock.connect(tcp_uri)) {
		fprintf(stderr, "Failed to connect to TCP socket\n");
		return 1;
	}

	return 0;
}
