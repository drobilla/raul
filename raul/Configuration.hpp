/* This file is part of Raul.
 * Copyright 2009-2011 David Robillard <http://drobilla.net>
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

#ifndef RAUL_CONFIGURATION_HPP
#define RAUL_CONFIGURATION_HPP

#include <exception>
#include <list>
#include <map>
#include <ostream>
#include <string>

#include "raul/Atom.hpp"

namespace Raul {

/** Program configuration (command line options and/or configuration file).
 *
 * \ingroup raul
 */
class Configuration {
public:
	Configuration(const std::string& shortdesc, const std::string& desc)
		: _shortdesc(shortdesc)
		, _desc(desc)
		, _max_name_length(0)
	{}

	Configuration& add(
			const std::string& name,
			char               letter,
			const std::string& desc,
			const Atom::Type   type,
			const Atom&        value);

	void print_usage(const std::string& program, std::ostream& os);

	struct CommandLineError : public std::exception {
		explicit CommandLineError(const std::string& m) : msg(m) {}
		~CommandLineError() throw() {}
		const char* what() const throw() { return msg.c_str(); }
		std::string msg;
	};

	void parse(int argc, char** argv) throw (CommandLineError);

	void print(std::ostream& os, const std::string mime_type="text/plain") const;

	const Raul::Atom& option(const std::string& long_name);
	const std::list<std::string>& files() const { return _files; }

private:
	struct Option {
	public:
		Option(const std::string& n, char l, const std::string& d,
		       const Atom::Type type, const Raul::Atom& def)
			: name(n), letter(l), desc(d), type(type), default_value(def), value(def)
		{}

		std::string name;
		char        letter;
		std::string desc;
		Atom::Type  type;
		Raul::Atom  default_value;
		Raul::Atom  value;
	};

	struct OptionNameOrder {
		inline bool operator()(const Option& a, const Option& b) {
			return a.name < b.name;
		}
	};

	typedef std::map<std::string, Option> Options;
	typedef std::map<char, std::string>   ShortNames;
	typedef std::list<std::string>        Files;

	int set_value_from_string(Configuration::Option& option, const std::string& value)
			throw (Configuration::CommandLineError);

	const std::string _shortdesc;
	const std::string _desc;
	Options           _options;
	ShortNames        _short_names;
	Files             _files;
	size_t            _max_name_length;
};

} // namespace Raul

#endif // RAUL_CONFIGURATION_HPP

