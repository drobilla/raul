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

#include "raul/Quantizer.hpp"
#include <iostream>

using namespace std;
using namespace Raul;

int
main()
{
	TimeStamp q(TimeUnit(TimeUnit::BEATS, 19200), 0.25);

	for (double in = 0.0; in < 32; in += 0.23) {
		TimeStamp beats(TimeUnit(TimeUnit::BEATS, 19200), in);

		/*cout << "Q(" << in << ", 1/4) = "
			<< Quantizer::quantize(q, beats) << endl;*/

		if (Quantizer::quantize(q, beats).subticks() % (19200/4) != 0)
			return 1;
	}

	return 0;
}

