/* This file is part of Raul.
 * Copyright 2009-2012 David Robillard <http://drobilla.net>
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

#ifndef RAUL_LOG_HPP
#define RAUL_LOG_HPP

#include <boost/format.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace Raul {

typedef boost::basic_format<char> fmt;

/** Buffer for (possibly coloured) log printing.
 * \ingroup raul
 */
class LogBuffer : public std::streambuf
{
public:
	enum Colour {
		DEFAULT = 0,
		RED = 31,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		WHITE
	};

	LogBuffer(const char* prefix="", Colour colour=DEFAULT)
		: _prefix(prefix)
		, _out(std::cout)
		, _colour(colour)
	{}

	/** Change the colour of the output, e.g. out << colour(RED) << "red" << endl; */
	std::string colour(Colour c);

	/** Reset the colour of the output, e.g. out << plain() << "plain" << endl; */
	std::string plain();

protected:
	int_type overflow(int_type c) {
		if (c == '\n')
			emit();
		else if (c != traits_type::eof())
			_line += c;

		return c;
	}

	int sync() {
		if (!_line.empty())
			emit();
		return 0;
	}

private:
	void emit();

	std::string   _prefix;
	std::string   _line;
	std::ostream& _out;
	Colour        _colour;
};

class NullBuffer : public std::streambuf
{
protected:
	int_type overflow(int_type c) { return c; }
	int      sync()               { return 0; }
};

class Log : public std::ostream {
public:
	explicit Log(std::streambuf* buf) : std::ostream(buf) {}
	template<typename T> Log& operator()(const T& o) {
		*this << o;
		return *this;
	}
};

extern Raul::Log info;
extern Raul::Log warn;
extern Raul::Log error;
extern Raul::Log debug;

} // namespace Raul

#endif // RAUL_LOG_HPP
