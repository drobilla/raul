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

#include <sstream>
#include <string>

#include "raul/log.hpp"
#include "raul_config.h"

namespace Raul {

#ifdef RAUL_LOG_COLOUR

LogBuffer info_buffer("", LogBuffer::GREEN);
LogBuffer warn_buffer("", LogBuffer::YELLOW);
LogBuffer error_buffer("", LogBuffer::RED);
#ifdef RAUL_LOG_DEBUG
LogBuffer debug_buffer("", LogBuffer::CYAN);
#else
NullBuffer debug_buffer;
#endif

#else // !RAUL_LOG_COLOUR

LogBuffer info_buffer("INFO: ");
LogBuffer warn_buffer("WARNING: ");
LogBuffer error_buffer("ERROR: ");
#ifdef RAUL_LOG_DEBUG
LogBuffer debug_buffer("DEBUG: ");
#else
NullBuffer debug_buffer;
#endif

#endif // RAUL_LOG_COLOUR

Raul::Log info(&info_buffer);
Raul::Log warn(&warn_buffer);
Raul::Log error(&error_buffer);
Raul::Log debug(&debug_buffer);

std::string
Raul::LogBuffer::colour(Colour c)
{
	std::stringstream ss;
	ss << "\033[0;" << _colour << "m";
	return ss.str();
}

std::string
Raul::LogBuffer::plain()
{
	return "\033[0m";
}

void
Raul::LogBuffer::emit()
{
	if (_colour != DEFAULT)
		_out << std::string(colour(_colour));

	_out << _prefix << _line;

	if (_colour != DEFAULT)
		_out << plain();

	_out << std::endl;
	_line.clear();
}

} // namespace Raul
