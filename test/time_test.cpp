/*
  Copyright 2007-2015 David Robillard <d@drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "raul/TimeSlice.hpp"
#include "raul/TimeStamp.hpp"

#include <cstdint>
#include <iostream>

int
main()
{
  Raul::TimeUnit  unit(Raul::TimeUnit::BEATS, 19200);
  Raul::TimeSlice ts(48000, 19200, 120.0);

  double in_double = 2.5;

  Raul::TimeStamp t(
    unit,
    static_cast<uint32_t>(in_double),
    static_cast<uint32_t>((in_double - static_cast<uint32_t>(in_double)) *
                          unit.ppt()));

  std::cout << "\tSeconds: ";
  std::cout << ts.beats_to_seconds(t);
  std::cout << std::endl;

  std::cout << "\tTicks:   ";
  std::cout << ts.beats_to_ticks(t);
  std::cout << std::endl;

  return 0;
}
