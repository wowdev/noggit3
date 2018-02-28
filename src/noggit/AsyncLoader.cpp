// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/AsyncLoader.h>
#include <noggit/AsyncObject.h>

#include <list>

void AsyncLoader::process()
{
  AsyncObject* object;

  while (!_stop)
  {
    {    
      std::unique_lock<std::mutex> lock (_guard);

      _state_changed.wait 
      ( lock
       , [&]
      {
        return !_to_load.empty() || _stop;
      }
      );

      if (_stop)
      {
        return;
      }

      object = _to_load.front();
      _currently_loading.emplace_back (object);
      _to_load.pop_front();
    }

    try
    {
      object->finishLoading();

      {
        std::lock_guard<std::mutex> const lock (_guard);
        _currently_loading.remove (object);
        _state_changed.notify_all();
      }
    }
    catch (...)
    {
      queue_for_load (object);
    }
  }
}

void AsyncLoader::queue_for_load (AsyncObject* object)
{
  std::lock_guard<std::mutex> const lock (_guard);
  _to_load.push_back (object);
  _state_changed.notify_one();
}
void AsyncLoader::ensure_deletable (AsyncObject* object)
{
  std::unique_lock<std::mutex> lock (_guard);
  _state_changed.wait 
  ( lock
   , [&]
  {
    return std::find (_to_load.begin(), _to_load.end(), object) == _to_load.end()
      && std::find (_currently_loading.begin(), _currently_loading.end(), object) == _currently_loading.end()
      ;
  }
  );
}

AsyncLoader::AsyncLoader(int numThreads)
  : _stop (false)
{
  for (int i = 0; i < numThreads; ++i)
  {
    _threads.emplace_back (&AsyncLoader::process, this);
  }
}

AsyncLoader::~AsyncLoader()
{
  _stop = true;
  _state_changed.notify_all();

  for (auto& thread : _threads)
  {
    thread.join();
  }
}
