#include <iostream>
#include <unistd.h>
#include "raul/Thread.hpp"
#include "raul/Semaphore.hpp"

using namespace std;
using namespace Raul;

class Waiter : public Raul::Thread {
public:
	Waiter(Semaphore& sem) : _sem(sem) {
		Thread::set_name("Waiter");
	}

private:
	void _run() {
		while (true) {
			if (_sem.timed_wait(250)) {
				cout << "[Waiter] Received signal" << endl;
				break;
			} else {
				cout << "[Waiter] Timed out" << endl;
			}
		}
		cout << "[Waiter] Exiting" << endl;
	}

	Semaphore& _sem;
};

int
main()
{
	Thread& this_thread = Thread::get();
	this_thread.set_name("Main");

	Semaphore sem(0);
	Waiter waiter(sem);
	waiter.start();

	sleep(1);
	
	cout << "[Main] Signalling..." << endl;
	sem.post();

	waiter.join();
	cout << "[Main] Exiting" << endl;

	return 0;
}
