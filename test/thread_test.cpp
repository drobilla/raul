#include <iostream>
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
		cout << "[Waiter] Waiting for signal..." << endl;
		_sem.wait();
		cout << "[Waiter] Received signal, exiting" << endl;
	}

	Semaphore& _sem;
};
		
int
main()
{
	Thread::create_for_this_thread();

	Thread& this_thread = Thread::get();
	this_thread.set_name("Main");
	
	Semaphore sem(0);
	Waiter waiter(sem);
	waiter.start();

	cout << "[Main] Signaling..." << endl;
	sem.post();

	cout << "[Main] Waiting for waiter..." << endl;
	waiter.join();

	cout << "[Main] Exiting" << endl;

	return 0;
}
