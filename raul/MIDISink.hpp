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

#ifndef RAUL_MIDI_SINK_HPP
#define RAUL_MIDI_SINK_HPP

#include <stdexcept>
#include "raul/TimeStamp.hpp"
#include "raul/Deletable.hpp"

namespace Raul {

/** Pure virtual base for anything you can write MIDI to.
 * \ingroup raul
 */
class MIDISink : public Deletable {
public:
	virtual void write_event(TimeStamp      time,
	                         size_t         ev_size,
	                         const uint8_t* ev) = 0;
};

} // namespace Raul

#endif // RAUL_MIDI_SINK_HPP

