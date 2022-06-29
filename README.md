Raul
====

Raul (Realtime Audio Utility Library) is a header-only C++ utility library for
real-time audio applications.

Components
----------

  * `Array`: A disposable array with a runtime size.
  * `DoubleBuffer`: A realtime-safe double buffer.
  * `Maid`: A simple explicit garbage collector.
  * `Path`: A restricted path of symbols.
  * `Process`: A child process.
  * `RingBuffer`: A lock-free ring buffer.
  * `Semaphore`: A process-local counting semaphore.
  * `Socket`: A UNIX or TCP socket.
  * `Symbol`: A valid C identifier string and path component.

Dependencies
------------

None, except the C++ standard library.

Documentation
-------------

Public interfaces are well-documented in the [headers](include/raul/).  There
is no external documentation at this time.

  * [Installation Instructions](INSTALL.md)

 -- David Robillard <d@drobilla.net>
