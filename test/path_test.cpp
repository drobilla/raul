#include <iostream>
#include <list>
#include "raul/log.hpp"
#include "raul/Path.hpp"

using namespace std;
using namespace Raul;

int
main()
{
#define CHECK(cond) \
	do { if (!(cond)) { \
		error << "Test failed: " << (cond) << endl; \
		return 1; \
	} } while (0)

	list<string> names;
	names.push_back("Dry/Wet Balance");
	names.push_back("foo+1bar(baz)");
	names.push_back("ThisCRAR");
	names.push_back("NAME");
	names.push_back("thing with a bunch of spaces");
	names.push_back("thing-with-a-bunch-of-dashes");
	names.push_back("CamelCaseABC");
	names.push_back("Signal Level [dB]");
	names.push_back("Gain dB");
	names.push_back("Dry/Wet Balance");
	names.push_back("Phaser1 - Similar to CSound's phaser1 by Sean Costello");

	for (list<string>::iterator i = names.begin(); i != names.end(); ++i) {
		CHECK(Symbol::is_valid(Path::nameify(*i)));
		CHECK(Symbol::is_valid(Symbol::symbolify(*i)));
	}

	CHECK(Path("/foo/bar").parent() == Path("/foo"));
	CHECK(Path("/foo").parent() == Path("/"));
	CHECK(Path("/").parent() == Path("/"));

	CHECK(Path("/").is_parent_of(Path("/foo")));
	CHECK(Path("/foo").is_parent_of(Path("/foo/bar")));
	CHECK(!(Path("/foo").is_parent_of(Path("/foo2"))));

	CHECK(!Path::is_valid(""));
	CHECK(!Path::is_valid("hello"));
	CHECK(!Path::is_valid("/foo/bar/"));
	CHECK(!Path::is_valid("/foo//bar"));
	CHECK(!Path::is_valid("/foo/bar/d*s"));
	CHECK(Path::is_valid("/"));
	CHECK(!Path::is_valid("/foo/3foo/bar"));

	CHECK(Path::descendant_comparator("/", "/foo"));
	CHECK(Path::descendant_comparator("/foo", "/foo/bar"));
	CHECK(Path::descendant_comparator("/foo", "/foo"));
	CHECK(Path::descendant_comparator("/", "/"));
	CHECK(!Path::descendant_comparator("/baz", "/"));
	CHECK(!Path::descendant_comparator("/foo", "/bar"));
	CHECK(!Path::descendant_comparator("/foo/bar", "/foo"));

	CHECK(!Symbol::is_valid(""));
	CHECK(!Symbol::is_valid("/I/have/slashes"));
	CHECK(!Symbol::is_valid("!illegalchar"));
	CHECK(!Symbol::is_valid("0illegalleadingdigit"));
	CHECK(Symbol::is_valid(Symbol::symbolify("")));
	CHECK(Symbol::is_valid(Symbol::symbolify("1hello")));

	return 0;
}
