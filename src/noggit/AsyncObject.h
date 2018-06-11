// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

enum class async_priority : int
{
  high,
  medium,
  low
};

class AsyncObject
{
protected:
  bool finished;

  AsyncObject(std::string filename) : filename(filename) {}

public:
  std::string const filename;

  AsyncObject() = delete;
  virtual ~AsyncObject() = default;

  virtual bool finishedLoading() const
  {
    return finished;
  }

  virtual async_priority loading_priority() const
  {
    return async_priority::medium;
  }

  virtual void finishLoading() = 0;
};
