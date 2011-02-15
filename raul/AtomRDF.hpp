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

#ifndef RAUL_ATOM_RDF_HPP
#define RAUL_ATOM_RDF_HPP

#include <cmath>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>

#include "raul/Atom.hpp"
#include "raul/log.hpp"
#include "redlandmm/Model.hpp"
#include "redlandmm/Node.hpp"
#include "redlandmm/World.hpp"

#define CUC(x) ((const unsigned char*)(x))

namespace Raul {

/** Conversion between Raul Atoms and Redlandmm RDF nodes.
 * This code (in header raul/AtomRDF.hpp) depends on redlandmm, only apps
 * which directly depend on both raul and redlandmm should include it.
 */
namespace AtomRDF {

/** Convert a Redland::Node to a Raul::Atom */
inline Atom
node_to_atom(Redland::Model& model, const Redland::Node& node)
{
	if (node.is_bool()) {
		return Atom(bool(node.to_bool()));
	} else if (node.is_resource()) {
		return Atom(Atom::URI, node.to_c_string());
	} else if (node.is_float()) {
		return Atom(node.to_float());
	} else if (node.is_int()) {
		return Atom(node.to_int());
	} else if (node.is_blank()) {
		Atom::DictValue dict;
		librdf_statement* pattern = librdf_new_statement_from_nodes(
				model.world().c_obj(),
				const_cast<librdf_node*>(node.c_obj()),
				NULL,
				NULL);
		librdf_stream* results = librdf_model_find_statements(
				const_cast<librdf_model*>(model.c_obj()),
				pattern);
		while (!librdf_stream_end(results)) {
			librdf_statement* s = librdf_stream_get_object(results);
			Redland::Node predicate(model.world(), librdf_statement_get_predicate(s));
			Redland::Node object(model.world(), librdf_statement_get_object(s));
			dict.insert(std::make_pair(node_to_atom(model, predicate), node_to_atom(model, object)));
			librdf_stream_next(results);
		}
		return Atom(dict);
	} else {
		return Atom(node.to_c_string());
	}
}


/** Convert a Raul::Atom to a Redland::Node
 * Note that not all Atoms are serialisable, the returned node should
 * be checked (can be treated as a bool) before use. */
inline Redland::Node
atom_to_node(Redland::Model& model, const Atom& atom)
{
	Redland::World& world = model.world();

	std::ostringstream os;
	std::string        str;
	librdf_uri*        type = NULL;
	librdf_node*       node = NULL;

	switch (atom.type()) {
	case Atom::INT:
		os << atom.get_int32();
		str = os.str();
		// xsd:integer -> pretty integer literals in Turtle
		type = librdf_new_uri(world.world(), CUC("http://www.w3.org/2001/XMLSchema#integer"));
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
		type = librdf_new_uri(world.world(), CUC("http://www.w3.org/2001/XMLSchema#decimal"));
		break;
	case Atom::BOOL:
		// xsd:boolean -> pretty boolean literals in Turtle
		if (atom.get_bool())
			str = "true";
		else
			str = "false";
		type = librdf_new_uri(world.world(), CUC("http://www.w3.org/2001/XMLSchema#boolean"));
		break;
	case Atom::URI:
		str = atom.get_uri();
		node = librdf_new_node_from_uri_string(world.world(), CUC(str.c_str()));
		break;
	case Atom::STRING:
		str = atom.get_string();
		break;
	case Atom::DICT:
		node = librdf_new_node(world.world());
		for (Atom::DictValue::const_iterator i = atom.get_dict().begin();
				i != atom.get_dict().end(); ++i) {
			model.add_statement(Redland::Node(world, node),
					atom_to_node(model, i->first),
					atom_to_node(model, i->second));
		}
		break;
	case Atom::BLOB:
	case Atom::NIL:
	default:
		warn << "Unserializable Atom" << std::endl;
		break;
	}

	if (!node && str != "")
		node = librdf_new_node_from_typed_literal(world.world(), CUC(str.c_str()), NULL, type);

	return Redland::Node(world, node);
}


} // namespace AtomRDF
} // namespace Raul

#endif // RAUL_ATOM_RDF_HPP

