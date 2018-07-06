// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/AsyncLoader.h>
#include <noggit/errorHandling.h>

#include <list>

void AsyncLoader::process()
{
  noggit::RegisterErrorHandlers();
  AsyncObject* object;

  while (!_stop)
  {
    {    
      std::unique_lock<std::mutex> lock (_guard);

      _state_changed.wait 
      ( lock
      , [&]
      {
        for (auto const& to_load : _to_load)
        {
          if (!to_load.second.empty())
          {
            return true;
          }
        }
        
        return !!_stop;
      }
      );

      if (_stop)
      {
        return;
      }

      for (auto& to_load : _to_load)
      {
        if (to_load.second.empty())
        {
          continue;
        }

        object = to_load.second.front();
        _currently_loading.emplace_back (object);
        to_load.second.pop_front();

        break;
      }
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
      object->error_on_loading();

      if (object->is_required_when_saving())
      {
        _important_object_failed_loading = true;
      }
    }
  }
}

void AsyncLoader::queue_for_load (AsyncObject* object)
{
  std::lock_guard<std::mutex> const lock (_guard);
  _to_load[object->loading_priority()].push_back (object);
  _state_changed.notify_one();
}

void AsyncLoader::ensure_loaded (AsyncObject* object)
{
  std::unique_lock<std::mutex> lock (_guard);
  _state_changed.wait 
  ( lock
  , [&]
    {
      auto const& to_load = _to_load[object->loading_priority()];
      return std::find (to_load.begin(), to_load.end(), object) == to_load.end()
        && std::find (_currently_loading.begin(), _currently_loading.end(), object) == _currently_loading.end()
        ;
    }
  );
}

void AsyncLoader::ensure_deletable (AsyncObject* object)
{
  std::unique_lock<std::mutex> lock (_guard);
  _state_changed.wait
  ( lock
  , [&]
    {
      auto& to_load = _to_load[object->loading_priority()];
      auto const& it = std::find (to_load.begin(), to_load.end(), object);
      
      // don't load it if it's just to delete it afterward
      if (it != to_load.end())
      {
        to_load.erase(it);
        return true;
      }
      else
      {
        return std::find (_currently_loading.begin(), _currently_loading.end(), object) == _currently_loading.end();
      }
    }
  );
}

AsyncLoader::AsyncLoader(int numThreads)
  : _stop (false)
{
  // to create the lists in the right order
  for (auto priority : { async_priority::high, async_priority::medium, async_priority::low })
  {
    _to_load[priority].clear();
  }

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
