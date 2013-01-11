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

#ifndef RAUL_MAID_HPP
#define RAUL_MAID_HPP

#include <atomic>

#include "raul/Disposable.hpp"
#include "raul/Manageable.hpp"
#include "raul/Noncopyable.hpp"
#include "raul/SharedPtr.hpp"

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
				if (_disposed.compare_exchange_strong(obj->_maid_next, obj)) {
					return;
				}
			}
		}
	}

	/** Manage an object held by a SharedPtr.
	 *
	 * This will hold a reference to @p ptr ensuring it will not be deleted
	 * except by cleanup().  This is mainly useful to allow dropping SharedPtr
	 * references in real-time threads without causing a deletion.
	 *
	 * This is not thread-safe.
	 *
	 * Note this mechanism scales linearly.  If a very large number of objects
	 * are managed cleanup() will become very expensive.
	 */
	inline void manage(SharedPtr<Manageable> ptr) {
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
			if (_disposed.compare_exchange_strong(disposed, NULL)) {
				break;
			}
		}

		// Free the disposed list
		for (Disposable* obj = _disposed.load(); obj;) {
			Disposable* const next = obj->_maid_next;
			delete obj;
			obj = next;
		}

		// Free the managed list
		SharedPtr<Manageable> managed = _managed;
		_managed.reset();
		for (SharedPtr<Manageable> obj = managed; obj;) {
			const SharedPtr<Manageable> next = obj->_maid_next;
			obj->_maid_next.reset();
			obj = next;
		}
	}

private:
	std::atomic<Disposable*> _disposed;
	SharedPtr<Manageable>    _managed;
};

} // namespace Raul

#endif // RAUL_MAID_HPP
