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

#ifndef RAUL_SOCKET_HPP
#define RAUL_SOCKET_HPP

#include <memory>

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "raul/Noncopyable.hpp"
#include "raul/URI.hpp"

namespace Raul {

/** A safe and simple interface for UNIX or TCP sockets. */
class Socket : public Raul::Noncopyable {
public:
	enum class Type {
		UNIX,
		TCP
	};

	/** Create a new unbound/unconnected socket of a given type. */
	explicit Socket(Type t);

	/** Wrap an existing open socket. */
	Socket(Type             t,
	       const Raul::URI& uri,
	       struct sockaddr* addr,
	       socklen_t        addr_len,
	       int              fd);

	~Socket();

	/** Bind a server socket to an address.
	 * @param uri Address URI, e.g. unix:///tmp/foo, or tcp://hostname:1234.
	 *            Use "*" as hostname to listen on all interfaces.
	 * @return True on success.
	 */
	bool bind(const Raul::URI& uri);

	/** Connect a client socket to a server address.
	 * @param uri Address URI, e.g. unix:///tmp/foo or tcp://somehost:1234
	 * @return True on success.
	 */
	bool connect(const Raul::URI& uri);

	/** Mark server socket as passive to listen for incoming connections.
	 * @return True on success.
	 */
	bool listen();

	/** Accept a connection.
	 * @return An new open socket for the connection.
	 */
	std::shared_ptr<Socket> accept();

	/** Return the file descriptor for the socket. */
	int fd() { return _sock; }

	const Raul::URI& uri() const { return _uri; }

	/** Close the socket. */
	void close();

	/** Shut down the socket.
	 * This terminates any connections associated with the sockets, and will
	 * (unlike close()) cause a poll on the socket to return.
	 */
	void shutdown();

private:
	bool set_addr(const Raul::URI& uri);

	Raul::URI        _uri;
	struct sockaddr* _addr;
	socklen_t        _addr_len;
	Type             _type;
	int              _sock;
};

#ifndef NI_MAXHOST
#    define NI_MAXHOST 1025
#endif

inline
Socket::Socket(Type t)
	: _uri(t == Type::UNIX ? "unix:" : "tcp:")
	, _addr(NULL)
	, _addr_len(0)
	, _type(t)
	, _sock(-1)
{
	switch (t) {
	case Type::UNIX:
		_sock = socket(AF_UNIX, SOCK_STREAM, 0);
		break;
	case Type::TCP:
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		break;
	}
}

inline
Socket::Socket(Type             t,
               const Raul::URI& uri,
               struct sockaddr* addr,
               socklen_t        addr_len,
               int              fd)
	: _uri(uri)
	, _addr(addr)
	, _addr_len(addr_len)
	, _type(t)
	, _sock(fd)
{
}

inline
Socket::~Socket()
{
	free(_addr);
	close();
}

inline bool
Socket::set_addr(const Raul::URI& uri)
{
	free(_addr);
	if (_type == Type::UNIX && uri.substr(0, strlen("unix://")) == "unix://") {
		const std::string   path  = uri.substr(strlen("unix://"));
		struct sockaddr_un* uaddr = (struct sockaddr_un*)calloc(
			1, sizeof(struct sockaddr_un));
		uaddr->sun_family = AF_UNIX;
		strncpy(uaddr->sun_path, path.c_str(), sizeof(uaddr->sun_path) - 1);
		_uri      = uri;
		_addr     = (sockaddr*)uaddr;
		_addr_len = sizeof(struct sockaddr_un);
		return true;
	} else if (_type == Type::TCP && uri.find("://") != std::string::npos) {
		const std::string authority = uri.substr(uri.find("://") + 3);
		const size_t      port_sep  = authority.find(':');
		if (port_sep == std::string::npos) {
			return false;
		}

		std::string       host = authority.substr(0, port_sep);
		const std::string port = authority.substr(port_sep + 1).c_str();
		if (host.empty() || host == "*") {
			host = "0.0.0.0";  // INADDR_ANY
		}

		struct addrinfo* ainfo;
		int              st = 0;
		if ((st = getaddrinfo(host.c_str(), port.c_str(), NULL, &ainfo))) {
			return false;
		}

		_uri      = uri;
		_addr     = (struct sockaddr*)malloc(ainfo->ai_addrlen);
		_addr_len = ainfo->ai_addrlen;
		memcpy(_addr, ainfo->ai_addr, ainfo->ai_addrlen);
		freeaddrinfo(ainfo);
		return true;
	}
	return false;
}

inline bool
Socket::bind(const Raul::URI& uri)
{
	if (set_addr(uri) && ::bind(_sock, _addr, _addr_len) != -1) {
		return true;
	}

	return false;
}

inline bool
Socket::connect(const Raul::URI& uri)
{
	if (set_addr(uri) && ::connect(_sock, _addr, _addr_len) != -1) {
		return true;
	}

	return false;
}

inline bool
Socket::listen()
{
	if (::listen(_sock, 64) == -1) {
		return false;
	} else {
		return true;
	}
}

inline std::shared_ptr<Socket>
Socket::accept()
{
	socklen_t        client_addr_len = _addr_len;
	struct sockaddr* client_addr     = (struct sockaddr*)calloc(
		1, client_addr_len);

	int conn = ::accept(_sock, client_addr, &client_addr_len);
	if (conn == -1) {
		return std::shared_ptr<Socket>();
	}

	Raul::URI client_uri = _uri;
	if (_type != Type::UNIX) {
		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];
		if (!getnameinfo(client_addr, client_addr_len,
		                 host, sizeof(host), serv, sizeof(serv), 0)) {
			client_uri = Raul::URI(_uri.scheme() + "://" + host + ":" + serv);
		}
	}

	return std::shared_ptr<Socket>(
		new Socket(_type, client_uri, client_addr, client_addr_len, conn));
}

inline void
Socket::close()
{
	if (_sock != -1) {
		::close(_sock);
		_sock = -1;
	}
}

inline void
Socket::shutdown()
{
	if (_sock != -1) {
		::shutdown(_sock, SHUT_RDWR);
	}
}

}  // namespace Raul

#endif  // RAUL_SOCKET_HPP
