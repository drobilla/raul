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

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "raul/RingBuffer.hpp"

using namespace std;
using namespace Raul;

#define MSG_SIZE 20

namespace {

Raul::RingBuffer* ring       = 0;
size_t            n_writes   = 0;
bool              ring_error = false;

int
gen_msg(int* msg, int start)
{
	for (int i = 0; i < MSG_SIZE; ++i) {
		msg[i] = start;
		start  = (start + 1) % INT_MAX;
	}
	return start;
}

int
cmp_msg(int* msg1, int* msg2)
{
	for (int i = 0; i < MSG_SIZE; ++i) {
		if (msg1[i] != msg2[i]) {
			fprintf(stderr, "ERROR: %d != %d @ %d\n", msg1[i], msg2[i], i);
			return 0;
		}
	}

	return 1;
}

void
reader()
{
	printf("Reader starting\n");

	int    ref_msg[MSG_SIZE];   // Reference generated for comparison
	int    read_msg[MSG_SIZE];  // Read from ring
	size_t count = 0;
	int    start = gen_msg(ref_msg, 0);
	for (size_t i = 0; i < n_writes; ++i) {
		if (ring->read_space() >= MSG_SIZE * sizeof(int)) {
			const uint32_t n_read = ring->read(MSG_SIZE * sizeof(int), read_msg);
			if (n_read != MSG_SIZE * sizeof(int)) {
				fprintf(stderr, "FAIL: Read size incorrect\n");
				ring_error = true;
				return;
			}
			if (!cmp_msg(ref_msg, read_msg)) {
				fprintf(stderr, "FAIL: Message %zu is corrupt\n", count);
				ring_error = true;
				return;
			}
			start = gen_msg(ref_msg, start);
			++count;
		}
	}

	printf("Reader finished\n");
}

void
writer()
{
	printf("Writer starting\n");

	int write_msg[MSG_SIZE];  // Written to ring
	int start = gen_msg(write_msg, 0);
	for (size_t i = 0; i < n_writes; ++i) {
		if (ring->write_space() >= MSG_SIZE * sizeof(int)) {
			const uint32_t n_write = ring->write(MSG_SIZE * sizeof(int), write_msg);
			if (n_write != MSG_SIZE * sizeof(int)) {
				fprintf(stderr, "FAIL: Write size incorrect\n");
				ring_error = true;
				return;
			}
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

	size_t size = 1024;
	if (argc > 1) {
		size = std::stoul(argv[1]);
	}

	n_writes = size * 1024;
	if (argc > 2) {
		n_writes = std::stoul(argv[2]);
	}

	printf("Testing %zu writes of %u ints to a %zu int ring...\n",
	       n_writes, MSG_SIZE, size);

	ring = new Raul::RingBuffer(uint32_t(size));
	if (ring->capacity() < size - 1) {
		fprintf(stderr, "Ring capacity is smaller than expected\n");
		return 1;
	}

	if (ring->skip(1)) {
		fprintf(stderr, "Successfully skipped in empty RingBuffer\n");
		return 1;
	}

	char buf[6] = { 'h', 'e', 'l', 'l', '0', '\0' };
	if (ring->read(1, buf)) {
		fprintf(stderr, "Successfully read from empty RingBuffer\n");
		return 1;
	}

	ring->write(sizeof(buf), buf);
	ring->skip(1);
	char buf2[sizeof(buf) - 1];
	ring->read(sizeof(buf2), buf2);
	if (strcmp(buf2, buf + 1)) {
		fprintf(stderr, "Skip failed\n");
		return 1;
	}

	ring->reset();
	if (ring->read_space() != 0) {
		fprintf(stderr, "Reset RingBuffer is not empty\n");
		return 1;
	}

	for (uint32_t i = 0; i < ring->capacity(); ++i) {
		const char c = 'X';
		if (ring->write(1, &c) != 1) {
			fprintf(stderr, "Write failed\n");
			return 1;
		}
	}

	if (ring->write_space() != 0) {
		fprintf(stderr, "Ring is not full as expected\n");
		return 1;
	}

	if (ring->write(1, buf) != 0) {
		fprintf(stderr, "Successfully wrote to full RingBuffer\n");
		return 1;
	}

	if (ring->peek(1, buf2) != 1 || buf2[0] != 'X') {
		fprintf(stderr, "Failed to read from full RingBuffer\n");
		return 1;
	}

	ring->reset();

	std::thread reader_thread(reader);
	std::thread writer_thread(writer);

	reader_thread.join();
	writer_thread.join();

	if (ring_error) {
		fprintf(stderr, "FAIL: Error occurred\n");
		return 1;
	}

	delete ring;
	return 0;
}
