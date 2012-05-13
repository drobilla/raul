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

#include <algorithm>
#include <string>

#include "raul/Configuration.hpp"

using std::endl;
using std::string;

namespace Raul {

/** Add a configuration option.
 *
 * @param name Long name (without leading "--")
 * @param letter Short name (without leading "-")
 * @param desc Description
 * @param type Type
 * @param value Default value
 */
Configuration&
Configuration::add(
		const std::string& name,
		char               letter,
		const std::string& desc,
		const OptionType   type,
		const Value&       value)
{
	assert(value.type() == type || value.type() == 0);
	_max_name_length = std::max(_max_name_length, name.length());
	_options.insert(make_pair(name, Option(name, letter, desc, type, value)));
	if (letter != '\0') {
		_short_names.insert(make_pair(letter, name));
	}
	return *this;
}

void
Configuration::print_usage(const std::string& program, std::ostream& os)
{
	os << "Usage: " << program << " [OPTION]..." << endl;
	os << _shortdesc << endl << endl;
	os << _desc << endl << endl;
	os << "Options:" << endl;
	for (Options::iterator o = _options.begin(); o != _options.end(); ++o) {
		Option& option = o->second;
			os << "  ";
		if (option.letter != '\0')
			os << "-" << option.letter << ", ";
		else
			os << "    ";
		os.width(_max_name_length + 4);
		os << std::left << (string("--") + o->first);
		os << option.desc << endl;
	}
}

int
Configuration::set_value_from_string(Configuration::Option& option,
                                     const std::string&     value)
		throw (Configuration::CommandLineError)
{
	int   intval = 0;
	char* endptr = NULL;
	switch (option.type) {
	case INT:
		intval = static_cast<int>(strtol(value.c_str(), &endptr, 10));
		if (endptr && *endptr == '\0') {
			option.value = Value(intval);
		} else {
			throw CommandLineError("option `" + option.name
					+ "' has non-integer value `" + value + "'");
		}
		break;
	case STRING:
		option.value = Value(value.c_str());
		assert(option.value.type() == STRING);
		break;
	default:
		throw CommandLineError(string("bad option type `--") + option.name + "'");
	}
	return EXIT_SUCCESS;
}

/** Parse command line arguments. */
void
Configuration::parse(int argc, char** argv) throw (Configuration::CommandLineError)
{
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] != '-' || !strcmp(argv[i], "-")) {
			_files.push_back(argv[i]);
		} else if (argv[i][1] == '-') {
			const string name = string(argv[i]).substr(2);
			Options::iterator o = _options.find(name);
			if (o == _options.end()) {
				throw CommandLineError(string("unrecognized option `--") + name + "'");
			}
			if (o->second.type == BOOL) {
				o->second.value = Value(true);
			} else {
				if (++i >= argc)
					throw CommandLineError("missing value for `--" + name + "'");
				set_value_from_string(o->second, argv[i]);
			}
		} else {
			const size_t len = strlen(argv[i]);
			for (size_t j = 1; j < len; ++j) {
				char letter = argv[i][j];
				ShortNames::iterator n = _short_names.find(letter);
				if (n == _short_names.end())
					throw CommandLineError(string("unrecognized option `-") + letter + "'");
				Options::iterator o = _options.find(n->second);
				if (j < len - 1) {
					if (o->second.type != BOOL)
						throw CommandLineError(string("missing value for `-") + letter + "'");
					o->second.value = Value(true);
				} else {
					if (o->second.type == BOOL) {
						o->second.value = Value(true);
					} else {
						if (++i >= argc)
							throw CommandLineError(string("missing value for `-") + letter + "'");
						set_value_from_string(o->second, argv[i]);
					}
				}
			}
		}
	}
}

void
Configuration::print(std::ostream& os, const std::string mime_type) const
{
	for (Options::const_iterator o = _options.begin(); o != _options.end(); ++o) {
		const Option& option = o->second;
		os << o->first << " = " << option.value << endl;
	}
}

const Raul::Configuration::Value&
Configuration::option(const std::string& long_name)
{
	static const Value nil;
	Options::iterator o = _options.find(long_name);
	if (o == _options.end())
		return nil;
	else
		return o->second.value;
}

} // namespace Raul

