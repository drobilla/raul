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

#ifndef RAUL_ATOM_LIBLO_HPP
#define RAUL_ATOM_LIBLO_HPP

#include <lo/lo.h>

#include "raul/log.hpp"
#include "raul/Atom.hpp"

namespace Raul {

/** Conversion between Raul Atoms and Liblo OSC arguments.
 * This code (in header raul/AtomLiblo.hpp) depends on liblo, only apps which
 * directly depend on both raul and liblo should include it.
 */
namespace AtomLiblo {

/** Append a Raul Atom as a parameter to a liblo message */
inline void
lo_message_add_atom(lo_message m, const Atom& atom)
{
	switch (atom.type()) {
	case Atom::INT:
		lo_message_add_int32(m, atom.get_int32());
		break;
	case Atom::FLOAT:
		lo_message_add_float(m, atom.get_float());
		break;
	case Atom::STRING:
		lo_message_add_string(m, atom.get_string());
		break;
	case Atom::URI:
		lo_message_add_symbol(m, atom.get_uri());
		break;
	case Atom::BOOL:
		if (atom.get_bool())
			lo_message_add_true(m);
		else
			lo_message_add_false(m);
		break;
	case Atom::BLOB:
		if (atom.data_size() > 0)
			lo_message_add_blob(
				m, lo_blob_new(atom.data_size(), atom.get_blob()));
		else
			lo_message_add_nil(m);
		break;
	case Atom::NIL:
	default:
		lo_message_add_nil(m);
		break;
	}
}

/** Convert a liblo argument to a Raul::Atom */
inline Atom
lo_arg_to_atom(Raul::Forge& forge, char type, lo_arg* arg)
{
	switch (type) {
	case 'i':
		return forge.make(arg->i);
	case 'f':
		return forge.make(arg->f);
	case 's':
		return forge.make(&arg->s);
	case 'S':
		return forge.alloc(Atom::URI, &arg->S);
	case 'T':
		return forge.make((bool)true);
	case 'F':
		return forge.make((bool)false);
	default:
		warn << "Unable to convert OSC type '"
		     << type << "' to Atom" << std::endl;
		return Atom();
	}
}

} // namespace AtomLiblo
} // namespace Raul

#endif // RAUL_ATOM_LIBLO_HPP
