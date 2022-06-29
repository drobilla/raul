// Copyright 2007-2017 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAUL_MAID_HPP
#define RAUL_MAID_HPP

#include "raul/Deletable.hpp"
#include "raul/Noncopyable.hpp"

#include <atomic>
#include <memory>
#include <utility>

namespace raul {

/**
   Explicit garbage collector.

   This allows objects to be disposed of in a real-time thread, but actually
   deleted later by another thread which calls cleanup().  Disposable objects
   may be explicitly disposed by calling dispose(), or automatically managed
   with a managed_ptr which can safely be dropped in any thread, including
   real-time threads.

   @ingroup raul
*/
class Maid : public Noncopyable
{
public:
  /// An object that can be disposed via Maid::dispose()
  class Disposable : public Deletable
  {
  public:
    Disposable() = default;

    Disposable(const Disposable&) = delete;
    Disposable& operator=(const Disposable&) = delete;

    Disposable(Disposable&&) = delete;
    Disposable& operator=(Disposable&&) = delete;

    ~Disposable() override = default;

  private:
    friend class Maid;
    Disposable* _maid_next{};
  };

  /// Disposable wrapper for any type
  template<typename T>
  class Managed
    : public raul::Maid::Disposable
    , public T
  {
  public:
    template<typename... Args>
    explicit Managed(Args&&... args)
      : T(std::forward<Args>(args)...)
    {}
  };

  /// Deleter for Disposable objects
  template<typename T>
  class Disposer
  {
  public:
    explicit Disposer(Maid* maid)
      : _maid(maid)
    {}

    Disposer() = default;

    void operator()(T* obj)
    {
      if (_maid) {
        _maid->dispose(obj);
      }
    }

  private:
    Maid* _maid{nullptr};
  };

  /**
     A managed pointer that automatically disposes of its contents.

     This is a unique_ptr so that it is possible to statically verify that
     code is real-time safe.
  */
  template<typename T>
  using managed_ptr = std::unique_ptr<T, Disposer<T>>;

  Maid()
    : _disposed(nullptr)
  {}

  Maid(const Maid&) = delete;
  Maid& operator=(const Maid&) = delete;

  Maid(Maid&&)  = delete;
  Maid& operator=(Maid&&) = delete;

  ~Maid() { cleanup(); }

  /// Return false iff there is currently no garbage
  bool empty() const { return !_disposed.load(std::memory_order_relaxed); }

  /**
     Enqueue an object for deletion when cleanup() is called next.

     This is thread-safe, and real-time safe assuming reasonably low
     contention.
  */
  void dispose(Disposable* obj)
  {
    if (obj) {
      // Atomically add obj to the head of the disposed list
      do {
        obj->_maid_next = _disposed.load(std::memory_order_relaxed);
      } while (!_disposed.compare_exchange_weak(obj->_maid_next,
                                                obj,
                                                std::memory_order_release,
                                                std::memory_order_relaxed));
    }
  }

  /**
     Delete all disposed objects immediately.

     Obviously not real-time safe, but may be called while other threads are
     calling dispose().
  */
  void cleanup()
  {
    // Atomically get the head of the disposed list
    Disposable* const disposed =
      _disposed.exchange(nullptr, std::memory_order_acquire);

    // Free the disposed list
    for (Disposable* obj = disposed; obj;) {
      Disposable* const next = obj->_maid_next;
      delete obj;
      obj = next;
    }
  }

  /// Make a unique_ptr that will dispose its object when dropped
  template<class T, class... Args>
  managed_ptr<T> make_managed(Args&&... args)
  {
    return std::unique_ptr<T, Disposer<T>>(new T(std::forward<Args>(args)...),
                                           Disposer<T>(this));
  }

private:
  std::atomic<Disposable*> _disposed;
};

template<typename T>
using managed_ptr = Maid::managed_ptr<T>;

} // namespace raul

#endif // RAUL_MAID_HPP
