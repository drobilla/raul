#include "raul/Atom.hpp"

using namespace std;
using namespace Raul;

int
main()
{
	//static const size_t buf_size = 9;
	//char buf[buf_size] = "atomtest";

	Forge forge;

	Atom nil_atom = forge.make();
	Atom int_atom = forge.make(42);
	Atom float_atom = forge.make(42.0f);
	Atom bool_atom = forge.make(true);
	Atom string_atom = forge.alloc("hello");
	//Atom blob_atom = forge.alloc("http://example.org/atomtype", buf_size, buf);

	return 0;
}

