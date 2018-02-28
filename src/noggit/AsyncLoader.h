// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <memory>
#include <thread>

class AsyncObject;

class AsyncLoader
{
public:
  static AsyncLoader& instance()
  {
    static AsyncLoader async_loader(2);
    return async_loader;
  }

  //! Ownership is _not_ transferred. Call ensure_deletable to ensure 
  //! that a previously enqueued object can be destroyed.
  void queue_for_load (AsyncObject*);
  //! \todo Make part of ~AsyncObject?
  void ensure_deletable (AsyncObject*);

  AsyncLoader(int numThreads);
  ~AsyncLoader();

private:
  void process();

  std::mutex _guard;
  std::condition_variable _state_changed;
  std::atomic<bool> _stop;
  std::list<AsyncObject*> _to_load;
  std::list<AsyncObject*> _currently_loading;
  std::list<std::thread> _threads;
};
