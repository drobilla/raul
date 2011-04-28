#include <iostream>
#include <cstring>
#include <cstdlib>

#include "raul/log.hpp"
#include "raul/RingBuffer.hpp"

using namespace std;
using namespace Raul;

int
main()
{
	static const int n_tests = 32;
	for (int i = 0; i < n_tests; ++i) {
		const int size = (rand() % 2000) + 8;
		RingBuffer rb(size);

		for (int j = 0; j < size * 32; ++j) {
			char ev1[] = { 'a', 'b', 'c' };
			rb.write(3, ev1);

			char buf[3];
			uint32_t read = rb.read(3, buf);
			if (read != 3 || strncmp(buf, "abc", 3)) {
				error << "Corrupt event " << i << ".1: "
				      << buf[0] << buf[1] << buf[2] << endl;
				return 1;
			}
	
			char ev2[] = { 'd', 'e', 'f' };
			if (!rb.write(3, ev2)) {
				error << "Failed write " << i << ".2" << endl;
				return 1;
			}

			char ev3[] = { 'g', 'h' };
			if (!rb.write(2, ev3)) {
				error << "Failed write " << i << ".3" << endl;
				return 1;
			}

			if (rb.skip(1) != 1) {
				error << "Failed skip " << i << endl;
				return 1;
			}

			uint32_t n_read = rb.read(2, buf);
			if (n_read != 2 || strncmp(buf, "ef", 2)) {
				error << "Corrupt event " << i << ".4: "
				      << buf[0] << buf[1] << buf[2] << endl;
				return 1;
			}

			n_read = rb.peek(2, buf);
			if (n_read != 2 || strncmp(buf, "gh", 2)) {
				error << "Corrupt peek event " << i << ".5: "
				      << buf[0] << buf[1] << endl;
				return 1;
			}

			n_read = rb.read(2, buf);
			if (n_read != 2 || strncmp(buf, "gh", 2)) {
				error << "Corrupt event " << i << ".6: "
				      << buf[0] << buf[1] << endl;
				return 1;
			}
		}
	}

	info << "Successfully ran " << n_tests << " tests." << endl;
	return 0;
}

