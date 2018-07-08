// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/Log.h>

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
  bool _loading_failed;
protected:
  bool finished;  

  AsyncObject(std::string filename) : filename(filename), finished(false), _loading_failed(false) {}

public:
  std::string const filename;

  AsyncObject() = delete;
  virtual ~AsyncObject() = default;

  virtual bool finishedLoading() const
  {
    return finished;
  }

  bool loading_failed() const
  {
    return _loading_failed;
  }

  void error_on_loading()
  {
    LogError << filename << " could not be loaded" << std::endl;
    _loading_failed = true;
    finished = true;
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
