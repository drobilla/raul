#include "raul/Atom.hpp"

using namespace std;
using namespace Raul;

int
main()
{
	static const size_t buf_size = 9;
	char buf[buf_size] = "atomtest";


	Atom nil_atom();
	Atom int_atom(42);
	Atom float_atom(42.0f);
	Atom bool_atom(true);
	Atom string_atom("hello");
	Atom blob_atom("http://example.org/atomtype", buf_size, buf);

	return 0;
}

