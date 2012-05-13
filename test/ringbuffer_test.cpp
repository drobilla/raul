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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "raul/log.hpp"
#include "raul/RingBuffer.hpp"

using namespace std;
using namespace Raul;

#define MSG_SIZE 20

Raul::RingBuffer* ring       = 0;
size_t            n_writes   = 0;
bool              ring_error = false;

static int
gen_msg(int* msg, int start)
{
	for (int i = 0; i < MSG_SIZE; ++i) {
		msg[i] = start;
		start  = (start + 1) % INT_MAX;
	}
	return start;
}

static int
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

static void*
reader(void* arg)
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
				return NULL;
			}
			if (!cmp_msg(ref_msg, read_msg)) {
				fprintf(stderr, "FAIL: Message %zu is corrupt\n", count);
				ring_error = true;
				return NULL;
			}
			start = gen_msg(ref_msg, start);
			++count;
		}
	}

	printf("Reader finished\n");
	return NULL;
}

static void*
writer(void* arg)
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
				return NULL;
			}
			start = gen_msg(write_msg, start);
		}
	}

	printf("Writer finished\n");
	return NULL;
}

int
main(int argc, char** argv)
{
	if (argc > 1 && argv[1][0] == '-') {
		printf("Usage: %s SIZE N_WRITES\n", argv[0]);
		return 1;
	}

	int size = 1024;
	if (argc > 1) {
		size = atoi(argv[1]);
	}

	n_writes = size * 1024;
	if (argc > 2) {
		n_writes = atoi(argv[2]);
	}

	printf("Testing %zu writes of %d ints to a %d int ring...\n",
	       n_writes, MSG_SIZE, size);

	ring = new Raul::RingBuffer(size);

	pthread_t reader_thread;
	if (pthread_create(&reader_thread, NULL, reader, NULL)) {
		fprintf(stderr, "Failed to create reader thread\n");
		return 1;
	}

	pthread_t writer_thread;
	if (pthread_create(&writer_thread, NULL, writer, NULL)) {
		fprintf(stderr, "Failed to create writer thread\n");
		return 1;
	}

	pthread_join(reader_thread, NULL);
	pthread_join(writer_thread, NULL);

	if (ring_error) {
		fprintf(stderr, "FAIL: Error occurred\n");
		return 1;
	}

	delete ring;
	return 0;
}
