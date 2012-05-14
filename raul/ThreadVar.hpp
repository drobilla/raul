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

#ifndef RAUL_THREADVAR_HPP
#define RAUL_THREADVAR_HPP

#include <stdlib.h>
#include <pthread.h>

namespace Raul {

struct ThreadVarImpl;

/** Thread-specific variable.
 */
template<typename T>
class ThreadVar
{
public:
	ThreadVar(const T& default_value)
		: _default_value(default_value)
	{
		pthread_key_create(&_key, free);
	}

	~ThreadVar() {
		pthread_key_delete(_key);
	}

	ThreadVar& operator=(const T& value) {
		T* val = (T*)pthread_getspecific(_key);
		if (val) {
			*val = value;
		} else {
			val  = (T*)malloc(sizeof(value));
			*val = value;
			pthread_setspecific(_key, val);
		}
		return *this;
	}
	
	operator T() const {
		T* val = (T*)pthread_getspecific(_key);
		return val ? *val : _default_value;
	}

private:
	ThreadVar(const ThreadVar& noncopyable);
	ThreadVar& operator=(const ThreadVar& noncopyable);

	const T       _default_value;
	pthread_key_t _key;
};

} // namespace Raul

#endif // RAUL_THREADVAR_HPP
