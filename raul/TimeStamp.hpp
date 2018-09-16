/*
  This file is part of Raul.
  Copyright 2007-2014 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAUL_TIMESTAMP_HPP
#define RAUL_TIMESTAMP_HPP

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

namespace Raul {

/** A type of time stamp
 * \ingroup raul
 */
class TimeUnit {
public:
	enum Type {
		FRAMES,
		BEATS,
		SECONDS
	};

	/** `ppt` (parts per tick) is the sample rate for FRAMES, PPQN for BEATS,
	 *  and ignored for SECONDS.
	 */
	inline TimeUnit(Type type, uint32_t ppt) {
		assert(type == SECONDS || ppt != 0);
		_type = type;
		_ppt = ppt;
	}

	static inline TimeUnit frames(uint32_t srate) { return {FRAMES, srate}; }
	static inline TimeUnit beats(uint32_t ppqn)   { return {BEATS, ppqn}; }
	static inline TimeUnit seconds()              { return {BEATS, std::numeric_limits<uint32_t>::max()}; }

	inline Type     type() const { return _type; }
	inline uint32_t ppt()  const { return _ppt; }

	inline bool operator==(const TimeUnit& rhs) const {
		return (_type == rhs._type && _ppt == rhs._ppt);
	}

	inline bool operator!=(const TimeUnit& rhs) const {
		return (_type != rhs._type || _ppt != rhs._ppt);
	}

private:
	Type     _type;
	uint32_t _ppt;
};

/** A real-time time stamp (possible units: frame, absolute (s), or beat).
 *
 * This is a uint32_t:uint32_t fixed point representation, capable of
 * sub-sample accurate frame time, beat time (at any remotely sane
 * tempo and sample rate), and absolute time.  The absolute time (seconds)
 * is compatible with standard OSC/NTP time stamps.
 *
 * \ingroup raul
 */
class TimeStamp {
public:
	explicit TimeStamp(TimeUnit unit, uint32_t ticks = 0, uint32_t subticks = 0)
		: _ticks(ticks)
		, _subticks(subticks)
		, _unit(unit)
	{}

	inline TimeStamp(TimeUnit unit, double dec)
		: _unit(unit)
	{
		dec = std::max(0.0, dec);
		dec = std::min(double(std::numeric_limits<uint32_t>::max()), dec);
		double       integral;
		const double fractional = modf(dec, &integral);
		_ticks    = static_cast<uint32_t>(integral);
		_subticks = static_cast<uint32_t>(fractional * unit.ppt());
	}

	inline TimeStamp(const TimeStamp&) = default;
	TimeStamp& operator=(const TimeStamp&) = default;

	inline TimeUnit unit()     const { return _unit; }
	inline uint32_t ticks()    const { return _ticks; }
	inline uint32_t subticks() const { return _subticks; }

	inline double to_double() const {
		return _ticks + (_subticks / static_cast<double>(_unit.ppt()));
	}

	inline bool is_zero() const {
		return _ticks == 0 && _subticks == 0;
	}

	inline TimeStamp& operator=(uint32_t ticks) {
		_ticks = ticks;
		_subticks = 0;
		return *this;
	}

	inline bool operator==(const TimeStamp& rhs) const {
		return _ticks == rhs._ticks
			&& _subticks == rhs._subticks
			&& _unit == rhs._unit;
	}

	inline bool operator!=(const TimeStamp& rhs) const {
		return !operator==(rhs);
	}

	inline bool operator<(const TimeStamp& rhs) const {
		assert(_unit == rhs._unit);
		return (_ticks < rhs._ticks
				|| (_ticks == rhs._ticks && _subticks < rhs._subticks));
	}

	inline bool operator>(const TimeStamp& rhs) const {
		assert(_unit == rhs._unit);
		return (_ticks > rhs._ticks
				|| (_ticks == rhs._ticks && _subticks > rhs._subticks));
	}

	inline bool operator<=(const TimeStamp& rhs) const {
		return (*this == rhs) || ((*this) < rhs);
	}

	inline bool operator>=(const TimeStamp& rhs) const {
		return (*this == rhs) || ((*this) > rhs);
	}

	inline TimeStamp& operator+=(const TimeStamp& rhs) {
		assert(_unit == rhs._unit);
		_ticks += rhs._ticks;
		if (_subticks + rhs._subticks <= _unit.ppt()) {
			_subticks += rhs._subticks;
		} else if (rhs._subticks > 0) {
			++_ticks;
			_subticks = rhs._subticks + _subticks - _unit.ppt();
		}
		return *this;
	}

	inline TimeStamp& operator-=(const TimeStamp& rhs) {
		assert(_unit == rhs._unit);
		assert(rhs <= *this);
		_ticks -= rhs._ticks;
		if (_subticks >= rhs._subticks) {
			_subticks -= rhs._subticks;
		} else if (rhs._subticks > 0) {
			--_ticks;
			_subticks = _unit.ppt() - (rhs._subticks - _subticks);
		}
		return *this;
	}

	inline TimeStamp operator+(const TimeStamp& rhs) const {
		assert(_unit == rhs._unit);
		TimeStamp result = *this;
		result += rhs;
		return result;
	}

	inline TimeStamp operator-(const TimeStamp& rhs) const {
		assert(_unit == rhs._unit);
		TimeStamp result = *this;
		result -= rhs;
		return result;
	}

private:
	uint32_t _ticks;
	uint32_t _subticks;
	TimeUnit _unit;
};

static inline std::ostream&
operator<<(std::ostream& os, const TimeStamp& t)
{
	os << t.ticks() << ":" << t.subticks();
	switch (t.unit().type()) {
	case TimeUnit::FRAMES:
		os << " frames";
		break;
	case TimeUnit::BEATS:
		os << " beats";
		break;
	case TimeUnit::SECONDS:
		os << " seconds";
		break;
	}
	return os;
}

class FrameStamp : public TimeStamp {
public:
	explicit FrameStamp(uint32_t rate, uint32_t ticks = 0, uint32_t subticks = 0)
		: TimeStamp(TimeUnit(TimeUnit::FRAMES, rate), ticks, subticks)
	{}
};

class BeatStamp : public TimeStamp {
public:
	explicit BeatStamp(uint32_t ppqn, uint32_t ticks = 0, uint32_t subticks = 0)
		: TimeStamp(TimeUnit(TimeUnit::BEATS, ppqn), ticks, subticks)
	{}
};

/** Same thing as TimeStamp really, but makes code clearer and enforces
 * correct semantics via type safety */
using TimeDuration = TimeStamp;

} // namespace Raul

#endif // RAUL_TIMESTAMP_HPP
