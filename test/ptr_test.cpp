#include "raul/SharedPtr.hpp"
#include "raul/WeakPtr.hpp"

int
main()
{
	/* TODO: Actually test functionality... */

	SharedPtr<int> iptr;
	WeakPtr<int> wptr;
	wptr = iptr;

	return 0;
}
