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

#undef NDEBUG

#include "raul/Maid.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <memory>
#include <thread>
#include <vector>

using Raul::Maid;

static const size_t n_threads         = 8U;
static const size_t n_junk_per_thread = 1U << 16U;

static std::atomic<size_t> n_junk(0);
static std::atomic<size_t> n_finished_threads(0);

class Junk : public Maid::Disposable {
public:
	explicit Junk(size_t v) : _val(v) { ++n_junk; }

	Junk(const Junk&) = delete;
	Junk& operator=(const Junk&) = delete;

	Junk(Junk&&) = delete;
	Junk& operator=(Junk&&) = delete;

	~Junk() override { --n_junk; }

	size_t value() const { return _val; }

private:
	size_t _val;
};

static void
litter(Maid* maid)
{
	for (size_t i = 0; i < n_junk_per_thread; ++i) {
		Maid::managed_ptr<Junk> a = maid->make_managed<Junk>(i);
		assert(a->value() == i);
	}

	++n_finished_threads;
}

static void
test()
{
	Maid maid;

	// Check basic single-threaded correctness
	{
		assert(n_junk == 0);
		Maid::managed_ptr<Junk> a = maid.make_managed<Junk>(1U);
		assert(n_junk == 1);
		Maid::managed_ptr<Junk> b = maid.make_managed<Junk>(2U);
		assert(n_junk == 2);
	}

	maid.dispose(nullptr);  // Mustn't crash

	// All referenes dropped, but deletion deferred
	assert(n_junk == 2);

	// Trigger actual deletion
	maid.cleanup();
	assert(n_junk == 0);
	assert(maid.empty());

	// Create some threads to produce garbage
	std::vector<std::thread> litterers;
	for (size_t i = 0; i < n_threads; ++i) {
		litterers.emplace_back(litter, &maid);
	}

	// Wait for some garbage to show up if necessary (unlikely)
	size_t initial_n_junk = n_junk;
	while (maid.empty()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		initial_n_junk = n_junk;
	}

	printf("Starting with %zu initial bits of junk\n", initial_n_junk);

	// Ensure we're actually cleaning things up concurrently
	maid.cleanup();
	assert(n_junk != initial_n_junk);

	// Continue cleaning up as long as threads are running
	size_t n_cleanup_calls = 1;
	while (n_finished_threads < n_threads) {
		maid.cleanup();
		++n_cleanup_calls;
	}

	printf("Called cleanup %zu times\n", n_cleanup_calls);

	// Join litterer threads
	for (auto& t : litterers) {
		t.join();
	}

	// Clean up any leftover garbage (unlikely/impossible?)
	maid.cleanup();
	assert(n_junk == 0);

	// Allocate a new object, then let it and the Maid go out of scope
	Maid::managed_ptr<Junk> c = maid.make_managed<Junk>(5U);
	assert(n_junk == 1);
}

int
main()
{
	assert(n_junk == 0);
	test();
	assert(n_junk == 0);
	return 0;
}
