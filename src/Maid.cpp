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

#include "raul/Maid.hpp"
#include "raul/Deletable.hpp"
#include "raul/SharedPtr.hpp"

namespace Raul {

Maid::Maid(size_t size)
	: _objects(size)
{
}

Maid::~Maid()
{
	cleanup();
}

/** Manage a SharedPtr.  NOT realtime safe.
 *
 * @a ptr is guaranteed to be deleted in the context that calls cleanup()
 */
void
Maid::manage(SharedPtr<Raul::Deletable> ptr)
{
	if (ptr)
		_managed.push_back(new Raul::List<SharedPtr<Raul::Deletable> >::Node(ptr));
}

/** Free all the objects in the queue (passed by push()).
 */
void
Maid::cleanup()
{
	Raul::Deletable* obj = NULL;

	while (!_objects.empty()) {
		obj = _objects.front();
		_objects.pop();
		delete obj;
	}

	for (Managed::iterator i = _managed.begin(); i != _managed.end() ; ) {
		Managed::iterator next = i;
		++next;

		if ((*i).unique())
			_managed.erase(i);

		i = next;
	}

}

} // namespace Raul
