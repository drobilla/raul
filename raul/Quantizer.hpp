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

#ifndef RAUL_QUANTIZER_HPP
#define RAUL_QUANTIZER_HPP

#include <cmath>
#include "raul/TimeStamp.hpp"

namespace Raul {

/** Quantizer.
 * \ingroup raul
 */
class Quantizer {
public:
	inline static TimeStamp quantize(TimeStamp q, TimeStamp t) {
		assert(q.unit() == t.unit());
		// FIXME: Precision problem?  Should probably stay in discrete domain
		const double qd = q.to_double();
		const double td = t.to_double();
		return TimeStamp(t.unit(), (qd > 0) ? lrint(td / qd) * qd : td);
	}

	inline static double quantize(double q, double t) {
		return (q > 0)
			? lrint(t / q) * q
			: t;
	}
};

} // namespace Raul

#endif // RAUL_QUANTIZER_HPP
