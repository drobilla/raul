/*
  Copyright 2007-2017 David Robillard <d@drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "raul/Array.hpp"
#include "raul/Deletable.hpp"
#include "raul/DoubleBuffer.hpp"
#include "raul/Exception.hpp"
#include "raul/Maid.hpp"
#include "raul/Noncopyable.hpp"
#include "raul/Path.hpp"
#include "raul/RingBuffer.hpp"
#include "raul/Semaphore.hpp"
#include "raul/Symbol.hpp"
#include "raul/TimeSlice.hpp"
#include "raul/TimeStamp.hpp"

#ifndef _WIN32
#	include "raul/Process.hpp"
#	include "raul/Socket.hpp"
#endif

class DeletableThing : public Raul::Deletable
{
};

class NonCopyableThing : public Raul::Noncopyable
{
};

int
main()
{
	Raul::Array<int>        array;
	DeletableThing          deletable;
	Raul::DoubleBuffer<int> double_buffer(0);
	Raul::Maid              maid;
	NonCopyableThing        non_copyable;
	Raul::Path              path;
	Raul::RingBuffer        ring_buffer(64u);
	Raul::Semaphore         semaphore(0u);
	Raul::Symbol            symbol("foo");
	Raul::TimeSlice         time_slice(48000u, 960u, 120.0);

	Raul::TimeStamp time_stamp(Raul::TimeUnit(Raul::TimeUnit::BEATS, 960u));

	try {
		Raul::Symbol bad_symbol("not a valid symbol!");
		(void)bad_symbol;
	} catch (const Raul::Exception&) {
	}

#ifndef _WIN32
	const char* cmd[] = {"echo"};
	Raul::Process::launch(cmd);

	Raul::Socket socket(Raul::Socket::Type::UNIX);

	(void)socket;
#endif

	(void)array;
	(void)deletable;
	(void)double_buffer;
	(void)maid;
	(void)non_copyable;
	(void)path;
	(void)ring_buffer;
	(void)symbol;
	(void)time_slice;
	(void)time_stamp;

	return 0;
}
