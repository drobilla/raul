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

#ifndef RAUL_ATOM_HPP
#define RAUL_ATOM_HPP

#include <stdint.h>
#include <glib.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

namespace Raul {

class Forge;

/** A piece of data with some type.
 *
 * An Atom is either a primitive type (int, float, etc.) or a blob.  Primitives
 * are contained entirely within this struct so everything is realtime safe.
 * Blob creation/copying/destruction allocates and is not realtime safe.
 *
 * \ingroup raul
 */
class Atom {
public:
	Atom() : _size(0), _type(0) { _val._blob = NULL; }
	~Atom() { dealloc(); }

	typedef uint32_t TypeID;

	Atom(const Atom& copy)
		: _size(copy._size)
		, _type(copy._type)
	{
		if (is_reference()) {
			_val._blob = malloc(_size);
			memcpy(_val._blob, copy._val._blob, _size);
		} else {
			memcpy(&_val, &copy._val, _size);
		}
	}

	Atom& operator=(const Atom& other) {
		dealloc();
		_size = other._size;
		_type = other._type;
		if (is_reference()) {
			_val._blob = malloc(_size);
			memcpy(_val._blob, other._val._blob, _size);
		} else {
			memcpy(&_val, &other._val, _size);
		}
		return *this;
	}

	inline bool operator==(const Atom& other) const {
		if (_type == other.type()) {
			if (is_reference()) {
				return !memcmp(_val._blob, other._val._blob, _size);
			} else {
				return !memcmp(&_val, &other._val, _size);
			}
		}
		return false;
	}

	inline bool operator!=(const Atom& other) const { return ! operator==(other); }

	inline bool operator<(const Atom& other) const {
		if (_type == other.type()) {
			if (is_reference()) {
				return memcmp(_val._blob, other._val._blob, _size) < 0;
			} else {
				return memcmp(&_val, &other._val, _size) < 0;
			}
		}
		return _type < other.type();
	}

	inline uint32_t size()     const { return _size; }
	inline bool     is_valid() const { return _type; }
	inline TypeID   type()     const { return _type; }

	inline const void* get_body() const {
		return is_reference() ? _val._blob : &_val;
	}

	inline int32_t     get_int32()  const { return _val._int; }
	inline float       get_float()  const { return _val._float; }
	inline bool        get_bool()   const { return _val._bool; }
	inline const char* get_uri()    const { return (const char*)get_body(); }
	inline const char* get_string() const { return (const char*)get_body(); }

	typedef std::map<Raul::Atom, Raul::Atom> DictValue;

	inline const DictValue& get_dict() const { return *(DictValue*)_val._blob; }

private:
	friend class Forge;

	Atom(uint32_t s, TypeID t, const void* v)
		: _size(s)
		, _type(t)
	{
		if (is_reference()) {
			_val._blob = malloc(s);
			memcpy(_val._blob, v, s);
		} else {
			memcpy(&_val, v, s);
		}
	}

	inline void dealloc() {
		if (is_reference()) {
			free(_val._blob);
		}
	}

	inline bool is_reference() const {
		return _size > sizeof(_val);
	}

	uint32_t _size;
	TypeID   _type;

	union {
		int32_t _int;
		float   _float;
		int32_t _bool;
		void*   _blob;
	} _val;
};

class Forge {
public:
	Forge()
		: Int(1)
		, Float(2)
		, Bool(3)
		, URI(4)
		, URID(5)
		, String(6)
		, Dict(7)
	{}

	virtual ~Forge() {}

	Atom make()          { return Atom(); }
	Atom make(int32_t v) { return Atom(sizeof(int32_t), Int, &v); }
	Atom make(float v)   { return Atom(sizeof(float), Float, &v); }
	Atom make(bool v) {
		const int32_t iv = v ? 1 : 0;
		return Atom(sizeof(int32_t), Bool, &iv);
	}

	Atom make_urid(int32_t v) { return Atom(sizeof(int32_t), URID, &v); }

	Atom alloc(uint32_t size, uint32_t type, const void* val) {
		return Atom(size, type, val);
	}

	Atom alloc(const char* v) {
		const size_t len = strlen(v);
		return Atom(len + 1, String, v);
	}

	Atom alloc(const std::string& v) {
		return Atom(v.length() + 1, String, v.c_str());
	}

	Atom alloc(const Atom::DictValue& v) {
		std::cerr << "FIXME: Atom dict" << std::endl;
		return Atom();
	}

	Atom alloc_uri(const char* v) {
		const size_t len = strlen(v);
		return Atom(len + 1, URI, v);
	}

	Atom alloc_uri(const std::string& v) {
		return Atom(v.length() + 1, URI, v.c_str());
	}

	Atom::TypeID Int;
	Atom::TypeID Float;
	Atom::TypeID Bool;
	Atom::TypeID URI;
	Atom::TypeID URID;
	Atom::TypeID String;
	Atom::TypeID Dict;
};

} // namespace Raul

#endif // RAUL_ATOM_HPP
