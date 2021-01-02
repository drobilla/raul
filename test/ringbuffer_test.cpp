/*
  Copyright 2007-2019 David Robillard <d@drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#undef NDEBUG

#include "raul/RingBuffer.hpp"

#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <thread>

constexpr const auto MSG_SIZE = 20u;

namespace {

using RingBuffer = Raul::RingBuffer;

struct Context {
	std::unique_ptr<RingBuffer> ring{};
	size_t n_writes{0};
};

int
gen_msg(int* msg, int start)
{
	for (unsigned i = 0u; i < MSG_SIZE; ++i) {
		msg[i] = start;
		start  = (start + 1) % INT_MAX;
	}
	return start;
}

void
check_msg(const int* msg1, const int* msg2)
{
	for (unsigned i = 0u; i < MSG_SIZE; ++i) {
		assert(msg1[i] == msg2[i]);
	}
}

void
reader(Context& ctx)
{
	printf("Reader starting\n");

	int    ref_msg[MSG_SIZE];   // Reference generated for comparison
	int    read_msg[MSG_SIZE];  // Read from ring
	size_t count = 0;
	int    start = gen_msg(ref_msg, 0);
	for (size_t i = 0; i < ctx.n_writes; ++i) {
		if (ctx.ring->read_space() >= MSG_SIZE * sizeof(int)) {
			const uint32_t n_read = ctx.ring->read(MSG_SIZE * sizeof(int), read_msg);
			assert(n_read == MSG_SIZE * sizeof(int));
			check_msg(ref_msg, read_msg);
			start = gen_msg(ref_msg, start);
			++count;
		}
	}

	printf("Reader finished\n");
}

void
writer(Context& ctx)
{
	printf("Writer starting\n");

	int write_msg[MSG_SIZE];  // Written to ring
	int start = gen_msg(write_msg, 0);
	for (size_t i = 0; i < ctx.n_writes; ++i) {
		if (ctx.ring->write_space() >= MSG_SIZE * sizeof(int)) {
			const uint32_t n_write = ctx.ring->write(MSG_SIZE * sizeof(int), write_msg);
			assert(n_write == MSG_SIZE * sizeof(int));
			start = gen_msg(write_msg, start);
		}
	}

	printf("Writer finished\n");
}

} // namespace

int
main(int argc, char** argv)
{
	if (argc > 1 && argv[1][0] == '-') {
		printf("Usage: %s SIZE N_WRITES\n", argv[0]);
		return 1;
	}

	Context ctx;

	uint32_t size = 512u;
	if (argc > 1) {
		size = static_cast<uint32_t>(std::stoi(argv[1]));
	}

	ctx.n_writes = size * 1024u;
	if (argc > 2) {
		ctx.n_writes = std::stoul(argv[2]);
	}

	printf("Testing %zu writes of %u ints to a %u int ring...\n",
	       ctx.n_writes, MSG_SIZE, size);

	ctx.ring = std::unique_ptr<RingBuffer>(new RingBuffer(size));

	auto& ring = ctx.ring;
	assert(ring->capacity() >= size - 1);

	assert(!ring->skip(1));

	char buf[6] = { 'h', 'e', 'l', 'l', '0', '\0' };
	assert(!ring->read(1, buf));

	ring->write(sizeof(buf), buf);
	ring->skip(1);
	char buf2[sizeof(buf) - 1];
	ring->read(sizeof(buf2), buf2);
	assert(!strcmp(buf2, buf + 1));

	ring->reset();
	assert(ring->read_space() == 0);

	for (uint32_t i = 0; i < ring->capacity(); ++i) {
		const char c = 'X';
		assert(ring->write(1, &c) == 1);
	}

	assert(ring->write_space() == 0);
	assert(ring->write(1, buf) == 0);
	assert(ring->peek(1, buf2) == 1);
	assert(buf2[0] == 'X');

	ring->reset();

	std::thread reader_thread(reader, std::ref(ctx));
	std::thread writer_thread(writer, std::ref(ctx));

	reader_thread.join();
	writer_thread.join();

	return 0;
}
