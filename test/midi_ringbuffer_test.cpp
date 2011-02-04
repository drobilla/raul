#include <iostream>
#include <cstring>
#include <cstdio>
#include <stdio.h>
#include "raul/TimeStamp.hpp"
#include "raul/EventRingBuffer.hpp"
#include "raul/midi_names.h"

using namespace std;
using namespace Raul;

int
read_write_test(EventRingBuffer& rb, unsigned offset)
{
	TimeStamp     t(TimeUnit(TimeUnit::FRAMES, 48000), 0, 0);
	size_t        size;
	unsigned char write_buf[5];
	unsigned char read_buf[5];

	snprintf(reinterpret_cast<char*>(write_buf), 5, "%d", offset);
	size = strlen(reinterpret_cast<const char*>(write_buf));

#ifndef NDEBUG
	const size_t written = rb.write(t, size, write_buf);
	assert(written == size);
#endif

	rb.read(&t, &size, read_buf);

	return strncmp(
			reinterpret_cast<const char*>(write_buf),
			reinterpret_cast<const char*>(read_buf),
			size);
}


int
main()
{
	EventRingBuffer rb(32);

	for (size_t i = 0; i < 1000000; ++i)
		if (read_write_test(rb, i))
			return 1;

	return 0;
}

