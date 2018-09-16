/*
  This file is part of Raul.
  Copyright 2007-2017 David Robillard <http://drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAUL_MAID_HPP
#define RAUL_MAID_HPP

#include "raul/Deletable.hpp"
#include "raul/Noncopyable.hpp"

#include <atomic>
#include <memory>

namespace Raul {

/** Explicit garbage-collector.
 *
 * This allows objects to be disposed of in a real-time thread, but actually
 * deleted later by another thread which calls cleanup().  Disposable objects
 * may be explicitly disposed by calling dispose(), or automatically managed
 * with a managed_ptr which can safely be dropped in any thread, including
 * real-time threads.
 *
 * \ingroup raul
 */
class Maid : public Noncopyable
{
public:
	/** An object that can be disposed via Maid::dispose(). */
	class Disposable : public Deletable {
	public:
		Disposable() = default;

	private:
		friend class Maid;
		Disposable* _maid_next{};
	};

	/** Disposable wrapper for any type. */
	template<typename T>
	class Managed : public Raul::Maid::Disposable, public T
	{
	public:
		template<typename... Args>
		explicit Managed(Args&&... args)
			: T(std::forward<Args>(args)...)
		{}
	};

	/** Deleter for Disposable objects. */
	template<typename T>
	class Disposer {
	public:
		explicit Disposer(Maid* maid) : _maid(maid) {}
		Disposer()                    : _maid(nullptr) {}

		void operator()(T* obj) {
			if (_maid) { _maid->dispose(obj); }
		}

		Maid* _maid{nullptr};
	};

	/** A managed pointer that automatically disposes of its contents.
	 *
	 * This is a unique_ptr so that it is possible to statically verify that
	 * code is real-time safe.
	 */
	template<typename T> using managed_ptr = std::unique_ptr<T, Disposer<T>>;

	Maid() : _disposed(nullptr) {}

	inline ~Maid() { cleanup(); }

	/** Return false iff there is currently no garbage. */
	inline bool empty() const {
		return !_disposed.load(std::memory_order_relaxed);
	}

	/** Enqueue an object for deletion when cleanup() is called next.
	 *
	 * This is thread-safe, and real-time safe assuming reasonably low
	 * contention.
	 */
	inline void dispose(Disposable* obj) {
		if (obj) {
			// Atomically add obj to the head of the disposed list
			do {
				obj->_maid_next = _disposed.load(std::memory_order_relaxed);
			} while (!_disposed.compare_exchange_weak(
				         obj->_maid_next, obj,
				         std::memory_order_release,
				         std::memory_order_relaxed));
		}
	}

	/** Delete all disposed objects immediately.
	 *
	 * Obviously not real-time safe, but may be called while other threads are
	 * calling dispose().
	 */
	inline void cleanup() {
		// Atomically get the head of the disposed list
		Disposable* const disposed = _disposed.exchange(
			nullptr, std::memory_order_acquire);

		// Free the disposed list
		for (Disposable* obj = disposed; obj;) {
			Disposable* const next = obj->_maid_next;
			delete obj;
			obj = next;
		}
	}

	/** Make a unique_ptr that will dispose its object when dropped. */
	template<class T, class... Args>
	managed_ptr<T> make_managed(Args&&... args) {
		return std::unique_ptr<T, Disposer<T> >(
			new T(std::forward<Args>(args)...),
			Disposer<T>(this));
	}

private:
	std::atomic<Disposable*> _disposed;
};

template<typename T>
using managed_ptr = Maid::managed_ptr<T>;

} // namespace Raul

#endif // RAUL_MAID_HPP
