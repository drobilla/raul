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

#include "raul/TimeStamp.hpp"
#include "raul/TimeSlice.hpp"
#include <iostream>

using namespace std;
using namespace Raul;

int
main()
{
	TimeUnit  unit(TimeUnit::BEATS, 19200);
	TimeSlice ts(48000, 19200, 120.0);

	double in_double = 2.5;

	TimeStamp t(unit, static_cast<uint32_t>(in_double),
			static_cast<uint32_t>((in_double - static_cast<uint32_t>(in_double)) * unit.ppt()));

	cout << "\tSeconds: ";
	cout << ts.beats_to_seconds(t);
	cout << endl;

	cout << "\tTicks:   ";
	cout << ts.beats_to_ticks(t);
	cout << endl;

	return 0;
}
