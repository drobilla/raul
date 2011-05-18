#include "raul/Quantizer.hpp"
#include <iostream>

using namespace std;
using namespace Raul;

int
main()
{
	TimeStamp q(TimeUnit(TimeUnit::BEATS, 19200), 0.25);

	for (double in = 0.0; in < 32; in += 0.23) {
		TimeStamp beats(TimeUnit(TimeUnit::BEATS, 19200), in);

		/*cout << "Q(" << in << ", 1/4) = "
			<< Quantizer::quantize(q, beats) << endl;*/

		if (Quantizer::quantize(q, beats).subticks() % (19200/4) != 0)
			return 1;
	}

	return 0;
}

