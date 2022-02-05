// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Log.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

enum class async_priority : int
{
  high,
  medium,
  low,
  count
};

class AsyncObject
{
private: 
  bool _loading_failed = false;
protected:
  std::atomic<bool> finished = {false};
  std::mutex _mutex;
  std::condition_variable _state_changed;

  AsyncObject(std::string filename_) : filename(filename_) {}

public:
  std::string const filename;

  AsyncObject() = delete;
  virtual ~AsyncObject() = default;

  virtual bool finishedLoading() const
  {
    return finished.load();
  }

  bool loading_failed() const
  {
    return _loading_failed;
  }

  void wait_until_loaded()
  {
    if (finished.load())
    {
      return;
    }

    std::unique_lock<std::mutex> lock (_mutex);

    _state_changed.wait 
    ( lock
    , [&]
      {
        return finished.load();
      }
    );
  }

  void error_on_loading()
  {
    LogError << filename << " could not be loaded" << std::endl;
    _loading_failed = true;
    finished = true;
    _state_changed.notify_all();
  }

  virtual bool is_required_when_saving() const
  {
    return false;
  }

  virtual async_priority loading_priority() const
  {
    return async_priority::medium;
  }

  virtual void finishLoading() = 0;
};
