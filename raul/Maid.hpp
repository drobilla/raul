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

#ifndef RAUL_MAID_HPP
#define RAUL_MAID_HPP

#include <atomic>
#include <memory>

#include "raul/Deletable.hpp"
#include "raul/Noncopyable.hpp"

namespace Raul {

/** Explicit quasi-garbage-collector.
 *
 * This allows objects to be disposed of in a real-time thread, but actually
 * deleted later by another thread which calls cleanup().
 *
 * \ingroup raul
 */
class Maid : public Noncopyable
{
public:
	/** An object that can be managed by the maid using shared_ptr. */
	class Manageable : public Deletable {
	public:
		Manageable() {}

	private:
		friend class Maid;
		std::shared_ptr<Manageable> _maid_next;
	};

	/** An object that can be disposed via Maid::dispose(). */
	class Disposable : public Deletable {
	public:
		Disposable() : _maid_next(NULL) {}

	private:
		friend class Maid;
		Disposable* _maid_next;
	};

	Maid() : _disposed(NULL) {}

	inline ~Maid() {
		cleanup();
	}

	/** Dispose of an object when cleanup() is called next.
	 *
	 * This is thread safe, and real-time safe assuming reasonably low
	 * contention.  If real-time threads need to push, do not call this very
	 * rapidly from many threads.
	 */
	inline void dispose(Disposable* obj) {
		if (obj) {
			while (true) {
				obj->_maid_next = _disposed.load();
				if (_disposed.compare_exchange_weak(obj->_maid_next, obj)) {
					return;
				}
			}
		}
	}

	/** Manage an object held by a shared pointer.
	 *
	 * This will hold a reference to `ptr` ensuring it will not be deleted
	 * except by cleanup().  This is mainly useful to allow dropping references
	 * in real-time threads without causing a deletion.
	 *
	 * This is not thread-safe.
	 *
	 * Note this mechanism scales linearly.  If a very large number of objects
	 * are managed cleanup() will become very expensive.
	 */
	inline void manage(std::shared_ptr<Manageable> ptr) {
		ptr->_maid_next = _managed;
		_managed        = ptr;
	}

	/** Free all dead and managed objects immediately.
	 *
	 * Obviously not real-time safe, but may be called while other threads are
	 * calling dispose().
	 */
	inline void cleanup() {
		// Atomically get the head of the disposed list
		Disposable* disposed;
		while (true) {
			disposed = _disposed.load();
			if (_disposed.compare_exchange_weak(disposed, NULL)) {
				break;
			}
		}

		// Free the disposed list
		for (Disposable* obj = disposed; obj;) {
			Disposable* const next = obj->_maid_next;
			delete obj;
			obj = next;
		}

		// Free the managed list
		std::shared_ptr<Manageable> managed = _managed;
		_managed.reset();
		for (std::shared_ptr<Manageable> obj = managed; obj;) {
			const std::shared_ptr<Manageable> next = obj->_maid_next;
			obj->_maid_next.reset();
			obj = next;
		}
	}

private:
	std::atomic<Disposable*>    _disposed;
	std::shared_ptr<Manageable> _managed;
};

} // namespace Raul

#endif // RAUL_MAID_HPP
