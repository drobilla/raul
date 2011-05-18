/* This file is part of Raul.
 * Copyright 2007-2011 David Robillard <http://drobilla.net>
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

#ifndef RAUL_ATOM_RDF_HPP
#define RAUL_ATOM_RDF_HPP

#include <cmath>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>

#include "raul/Atom.hpp"
#include "raul/log.hpp"

#include "sord/sordmm.hpp"

#define CUC(x) ((const unsigned char*)(x))

namespace Raul {

/** Conversion between Raul Atoms and Sord RDF nodes.
 * This code (in header raul/AtomRDF.hpp) depends on sord, only apps
 * which directly depend on both raul and sord should include it.
 */
namespace AtomRDF {

/** Convert a Sord::Node to a Raul::Atom */
inline Atom
node_to_atom(Sord::Model& model, const Sord::Node& node)
{
	if (node.is_bool()) {
		return Atom(bool(node.to_bool()));
	} else if (node.is_uri()) {
		return Atom(Atom::URI, node.to_c_string());
	} else if (node.is_float()) {
		return Atom(node.to_float());
	} else if (node.is_int()) {
		return Atom(node.to_int());
	} else if (node.is_blank()) {
		Atom::DictValue dict;
		Sord::Node nil;
		for (Sord::Iter i = model.find(node, nil, nil); !i.end(); ++i) {
			Sord::Node predicate = i.get_predicate();
			Sord::Node object    = i.get_object();
			dict.insert(std::make_pair(node_to_atom(model, predicate),
			                           node_to_atom(model, object)));
		}
		return Atom(dict);
	} else {
		return Atom(node.to_c_string());
	}
}

#define RAUL_NS_XSD "http://www.w3.org/2001/XMLSchema#"

/** Convert a Raul::Atom to a Sord::Node
 * Note that not all Atoms are serialisable, the returned node should
 * be checked (can be treated as a bool) before use. */
inline Sord::Node
atom_to_node(Sord::Model& model, const Atom& atom)
{
	Sord::World& world = model.world();

	std::ostringstream os;
	std::string        str;
	SordNode*          type = NULL;
	SordNode*          node = NULL;

	switch (atom.type()) {
	case Atom::INT:
		os << atom.get_int32();
		str = os.str();
		// xsd:integer -> pretty integer literals in Turtle
		type = sord_new_uri(world.world(), CUC(RAUL_NS_XSD "integer"));
		break;
	case Atom::FLOAT:
		if (std::isnan(atom.get_float()) || std::isinf(atom.get_float()))
			break;
		os.precision(8);
		os << atom.get_float();
		str = os.str();
		if (str.find(".") == std::string::npos)
			str += ".0";
		// xsd:decimal -> pretty decimal (float) literals in Turtle
		type = sord_new_uri(world.world(), CUC(RAUL_NS_XSD "decimal"));
		break;
	case Atom::BOOL:
		// xsd:boolean -> pretty boolean literals in Turtle
		if (atom.get_bool())
			str = "true";
		else
			str = "false";
		type = sord_new_uri(world.world(), CUC(RAUL_NS_XSD "boolean"));
		break;
	case Atom::URI:
		str = atom.get_uri();
		node = sord_new_uri(world.world(), CUC(str.c_str()));
		break;
	case Atom::STRING:
		str = atom.get_string();
		break;
	case Atom::DICT: {
		Sord::Node blank = Sord::Node::blank_id(model.world());
		for (Atom::DictValue::const_iterator i = atom.get_dict().begin();
				i != atom.get_dict().end(); ++i) {
			model.add_statement(blank,
			                    atom_to_node(model, i->first),
			                    atom_to_node(model, i->second));
		}
		node = blank.c_obj();
		break;
	}
	case Atom::BLOB:
	case Atom::NIL:
	default:
		warn << "Unserializable Atom" << std::endl;
		break;
	}

	if (!node && str != "")
		node = sord_new_literal(world.world(), type, CUC(str.c_str()), NULL);

	return Sord::Node(world, node);
}

} // namespace AtomRDF
} // namespace Raul

#endif // RAUL_ATOM_RDF_HPP

