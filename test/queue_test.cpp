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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "raul/SRMWQueue.hpp"
#include "raul/SRSWQueue.hpp"
#include "raul/fmt.hpp"

using namespace std;
using namespace Raul;

static const unsigned NUM_DATA             = 10;
static const unsigned QUEUE_SIZE           = 128;
static const unsigned NUM_WRITERS          = 2;
static const unsigned PUSHES_PER_ITERATION = 3;

// Data to read/write using actions pumped through the queue
struct Record {
	Record() : read_count(0), write_count(0) {}

	std::atomic<int> read_count;
	std::atomic<int> write_count;
};

Record data[NUM_DATA];

// Actions pumped through the queue to manipulate data
struct WriteAction {
	WriteAction(unsigned idx) : index(idx) {}

	inline void read() const {
		++(data[index].read_count);
	};

	unsigned index;
};

// The victim
SRMWQueue<WriteAction> queue(QUEUE_SIZE);

static void
test_write(bool* exit_flag)
{
	while (!*exit_flag) {
		for (unsigned j=0; j < PUSHES_PER_ITERATION; ++j) {
			unsigned i = rand() % NUM_DATA;
			if (queue.push(WriteAction(i))) {
				++(data[i].write_count);
				//cout << "WRITE " << i << "\r\n";
			} else {
				//cerr << "FAILED WRITE\r\n";
			}
		}
	}

	cout << "Writer exiting." << endl;
};

// Returns 0 if all read count/write count pairs are equal,
// otherwise how far off total count was
static unsigned
data_is_sane()
{
	unsigned ret = 0;
	for (unsigned i = 0; i < NUM_DATA; ++i) {
		unsigned diff = abs(data[i].read_count.load() - data[i].write_count.load());
		ret += diff;
	}

	return ret;
}

int
main()
{
	size_t total_processed = 0;

	cout << "Testing size" << endl;
	for (size_t i=0; i < queue.capacity(); ++i) {
		queue.push(i);
		if (i == queue.capacity()-1) {
			if (!queue.full()) {
				cerr << "ERROR: Should be full at " << i
						<< " (size " << queue.capacity() << ")" << endl;
				return -1;
			}
		} else {
			if (queue.full()) {
				cerr << "ERROR: Prematurely full at " << i
					<< " (size " << queue.capacity() << ")" << endl;
				return -1;
			}
		}
	}

	for (size_t i = 0; i < queue.capacity(); ++i)
		queue.pop();

	if (!queue.empty()) {
		cerr << "ERROR: Should be empty" << endl;
		return -1;
	}

	cout << "Testing concurrent reading/writing" << endl;
	bool                 exit_flags[NUM_WRITERS];
	vector<std::thread*> writers(NUM_WRITERS, NULL);

	for (unsigned i = 0; i < NUM_WRITERS; ++i) {
		exit_flags[i] = false;
		writers[i]    = new std::thread(test_write, &exit_flags[i]);
	}

	// Read
	unsigned count = 0;
	for (unsigned i = 0; i < 10000000; ++i) {
		while (count < queue.capacity() && !queue.empty()) {
			WriteAction action = queue.front();
			queue.pop();
			action.read();
			++count;
			++total_processed;
		}

		/*if (count > 0)
			cout << "Processed " << count << " requests\t\t"
				<< "(total " << total_processed << ")\r\n";

		if (total_processed > 0 && total_processed % 128l == 0)
			cout << "Total processed: " << total_processed << "\r\n";*/
	}

	cout << "Processed " << total_processed << " requests" << endl;

	// Stop the writers
	for (unsigned i = 0; i < NUM_WRITERS; ++i) {
		exit_flags[i] = true;
		writers[i]->join();
	}

	//cout << "\n\n****************** DONE *********************\n\n";

	unsigned leftovers = 0;

	// Drain anything left in the queue
	while (!queue.empty()) {
		WriteAction action = queue.front();
		queue.pop();
		action.read();
		leftovers++;
		++total_processed;
	}

	if (leftovers > 0)
		cout << "Processed " << leftovers << " leftovers." << endl;

	//cout << "\n\n*********************************************\n\n";

	cout << "Total processed: " << total_processed << endl;
	if (total_processed > INT_MAX)
		cout << "(Counter had to wrap)" << endl;
	else
		cout << "(Counter did NOT have to wrap)" << endl;

	const unsigned diff = data_is_sane();

	if (diff == 0) {
		return EXIT_SUCCESS;
	} else {
		cout << "FAILED BY " << diff << endl;
		return EXIT_FAILURE;
	}

	return 0;
}

#if 0
int main()
{
	//SRSWQueue<int> q(10);
	SRMWQueue<int> q(10);

	cout << "New queue.  Should be empty: " << q.empty() << endl;
	cout << "Capacity: " << q.capacity() << endl;
	//cout << "Fill: " << q.fill() << endl;

	for (uint i=0; i < 5; ++i) {
		q.push(i);
		assert(!q.full());
		q.pop();
	}
	cout << "Pushed and popped 5 elements.  Queue should be empty: " << q.empty() << endl;
	//cout << "Fill: " << q.fill() << endl;

	for (uint i=10; i < 20; ++i) {
		assert(q.push(i));
	}
	cout << "Pushed 10 elements.  Queue should be full: " << q.full() << endl;
	//cout << "Fill: " << q.fill() << endl;

	cout << "The digits 10->19 should print: " << endl;
	while (!q.empty()) {
		int foo = q.front();
		q.pop();
		cout << "Popped: " << foo << endl;
	}
	cout << "Queue should be empty: " << q.empty() << endl;
	//cout << "Fill: " << q.fill() << endl;

	cout << "Attempting to add eleven elements to queue of size 10. Only first 10 should succeed:" << endl;
	for (uint i=20; i <= 39; ++i) {
		cout << i;
		//cout << " - Fill: " << q.fill();
		cout << " - full: " << q.full();
		cout << ", succeeded: " << q.push(i) << endl;
	}

	return 0;
}
#endif

