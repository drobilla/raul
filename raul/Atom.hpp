/* This file is part of Raul.
 * Copyright (C) 2007-2009 David Robillard <http://drobilla.net>
 *
 * Raul is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Raul is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RAUL_ATOM_HPP
#define RAUL_ATOM_HPP

#include <stdint.h>
#include <glib.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <map>
#include <ostream>
#include <string>

namespace Raul {

class URI;

/** A piece of data with some type.
 *
 * Atoms can be of various primitive types (integer, float, etc) as well as
 * a string or primitive.  The primitive types are entirely contained within
 * the Atom, i.e. the Atom is POD.  String, URI, and blob atoms are not POD.
 *
 * \ingroup raul
 */
class Atom {
public:
	enum Type {
		NIL,
		INT,
		FLOAT,
		BOOL,
		URI,
		STRING,
		BLOB,
		DICT
	};

	Atom()                : _type(NIL),    _blob_val(0)             {}
	Atom(int32_t val)     : _type(INT),    _int_val(val)            {}
	Atom(float val)       : _type(FLOAT),  _float_val(val)          {}
	Atom(bool val)        : _type(BOOL),   _bool_val(val)           {}
	Atom(const char* val) : _type(STRING), _string_val(strdup(val)) {}

	Atom(const std::string& val) : _type(STRING), _string_val(strdup(val.c_str())) {}

	/** URI constructor (@a t must be URI) */
	Atom(Type t, const std::string& val) : _type(t), _string_val(g_intern_string(val.c_str())) {
		assert(t == URI);
	}

	Atom(const char* type_uri, size_t size, void* val)
		: _type(BLOB), _blob_val(new BlobValue(type_uri, size, val)) {}

	typedef std::map<Raul::Atom, Raul::Atom> DictValue;
	Atom(const DictValue& dict) : _type(DICT), _dict_val(new DictValue(dict)) {}

	~Atom() { dealloc(); }

	Atom(const Atom& copy)
		: _type(copy._type)
	{
		switch (_type) {
		case NIL:    _blob_val   = 0;                              break;
		case INT:    _int_val    = copy._int_val;                  break;
		case FLOAT:  _float_val  = copy._float_val;                break;
		case BOOL:   _bool_val   = copy._bool_val;                 break;
		case URI:    _string_val = copy._string_val;               break;
		case STRING: _string_val = strdup(copy._string_val);       break;
		case BLOB:   _blob_val   = new BlobValue(*copy._blob_val); break;
		case DICT:   _dict_val   = new DictValue(*copy._dict_val); break;
		}
	}

	Atom& operator=(const Atom& other) {
		dealloc();
		_type = other._type;

		switch (_type) {
		case NIL:    _blob_val   = 0;                               break;
		case INT:    _int_val    = other._int_val;                  break;
		case FLOAT:  _float_val  = other._float_val;                break;
		case BOOL:   _bool_val   = other._bool_val;                 break;
		case URI:    _string_val = other._string_val;               break;
		case STRING: _string_val = strdup(other._string_val);       break;
		case BLOB:   _blob_val   = new BlobValue(*other._blob_val); break;
		case DICT:   _dict_val   = new DictValue(*other._dict_val); break;
		}
		return *this;
	}

	inline bool operator==(const Atom& other) const {
		if (_type == other.type()) {
			switch (_type) {
			case NIL:    return true;
			case INT:    return _int_val    == other._int_val;
			case FLOAT:  return _float_val  == other._float_val;
			case BOOL:   return _bool_val   == other._bool_val;
			case URI:    return _string_val == other._string_val;
			case STRING: return strcmp(_string_val, other._string_val) == 0;
			case BLOB:   return _blob_val == other._blob_val;
			case DICT:   return *_dict_val == *other._dict_val;
			}
		}
		return false;
	}

	inline bool operator!=(const Atom& other) const { return ! operator==(other); }

	inline bool operator<(const Atom& other) const {
		if (_type == other.type()) {
			switch (_type) {
			case NIL:    return true;
			case INT:    return _int_val    < other._int_val;
			case FLOAT:  return _float_val  < other._float_val;
			case BOOL:   return _bool_val   < other._bool_val;
			case URI:
				if (_string_val == other._string_val) {
					return false;
				} // else fall through to STRING
			case STRING: return strcmp(_string_val, other._string_val) < 0;
			case BLOB:   return _blob_val   < other._blob_val;
			case DICT:   return *_dict_val  < *other._dict_val;
			}
		}
		return _type < other.type();
	}

	inline size_t data_size() const {
		switch (_type) {
		case NIL:    return 0;
		case INT:    return sizeof(uint32_t);
		case FLOAT:  return sizeof(float);
		case BOOL:   return sizeof(bool);
		case URI:
		case STRING: return strlen(_string_val) + 1;
		case BLOB:   return _blob_val->size();
		case DICT:   return 0; // FIXME ?
		}
		return 0;
	}

	inline bool is_valid() const { return (_type != NIL); }

	/** Type of this atom.  Always check this before attempting to get the
	 * value - attempting to get the incorrectly typed value is a fatal error.
	 */
	Type type() const { return _type; }

	inline int32_t     get_int32()  const { assert(_type == INT);    return _int_val; }
	inline float       get_float()  const { assert(_type == FLOAT);  return _float_val; }
	inline bool        get_bool()   const { assert(_type == BOOL);   return _bool_val; }
	inline const char* get_string() const { assert(_type == STRING); return _string_val; }
	inline const char* get_uri()    const { assert(_type == URI);    return _string_val; }

	inline const char* get_blob_type() const { assert(_type == BLOB); return _blob_val->type(); }
	inline const void* get_blob()      const { assert(_type == BLOB); return _blob_val->data(); }

	inline const DictValue& get_dict() const { assert(_type == DICT); return *_dict_val; }

private:
	Type _type;

	friend class Raul::URI;
	Atom(const char* str, uint32_t magic) : _type(URI), _string_val(str) {
		assert(magic == 12345);
		assert(g_intern_string(str) == str);
	}

	inline void dealloc() {
		switch (_type) {
		case STRING:
			free(const_cast<char*>(_string_val));
			break;
		case BLOB:
			delete _blob_val;
		default:
			break;
		}
	}

	class BlobValue {
	public:
		BlobValue(const char* type, size_t size, void* data)
			: _type_length(strlen(type) + 1) // + 1 for \0
			, _size(size)
			, _buf(malloc(_type_length + _size))
		{
			memcpy(_buf, type, _type_length);
			memcpy(static_cast<char*>(_buf) + _type_length, data, size);
		}

		BlobValue(const BlobValue& copy)
			: _type_length(copy._type_length)
			, _size(copy._size)
			, _buf(malloc(_type_length + _size))
		{
			_type_length = copy._type_length;
			memcpy(_buf, copy._buf, _type_length + _size);
		}

		~BlobValue() { free(_buf); }

		inline const char* type() const { return static_cast<const char*>(_buf); }
		inline const void* data() const { return static_cast<const char*>(_buf) + _type_length; }
		inline size_t      size() const { return _size; }
	private:
		size_t _type_length; ///< Length of type string (first part of buffer, inc. \0)
		size_t _size;        ///< Length of data not including (after) type string
		void*  _buf;         ///< Type string followed by data
	};

	union {
		int32_t          _int_val;
		float            _float_val;
		bool             _bool_val;
		const char*      _string_val;
		BlobValue*       _blob_val;
		const DictValue* _dict_val;
	};
};


} // namespace Raul

static inline std::ostream& operator<<(std::ostream& os, const Raul::Atom& atom)
{
	switch (atom.type()) {
	case Raul::Atom::NIL:    return os << "(nil)";
	case Raul::Atom::INT:    return os << atom.get_int32();
	case Raul::Atom::FLOAT:  return os << atom.get_float();
	case Raul::Atom::BOOL:   return os << (atom.get_bool() ? "true" : "false");
	case Raul::Atom::URI:    return os << "<" << atom.get_uri() << ">";
	case Raul::Atom::STRING: return os << atom.get_string();
	case Raul::Atom::BLOB:   return os << atom.get_blob();
	case Raul::Atom::DICT:
		os << "{";
		for (Raul::Atom::DictValue::const_iterator i = atom.get_dict().begin();
				i != atom.get_dict().end(); ++i) {
			os << " " << i->first << " " << i->second << ";";
		}
		os << " }";
		return os;
	}
	return os;
}

static inline std::ostream& operator<<(std::ostream& os, Raul::Atom::Type type)
{
	switch (type) {
	case Raul::Atom::NIL:    return os << "Nil";
	case Raul::Atom::INT:    return os << "Int";
	case Raul::Atom::FLOAT:  return os << "Float";
	case Raul::Atom::BOOL:   return os << "Bool";
	case Raul::Atom::URI:    return os << "URI";
	case Raul::Atom::STRING: return os << "String";
	case Raul::Atom::BLOB:   return os << "Blob";
	case Raul::Atom::DICT:   return os << "Dict";
	}
	return os;
}

#endif // RAUL_ATOM_HPP
