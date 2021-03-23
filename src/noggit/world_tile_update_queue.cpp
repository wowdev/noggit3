// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/world_tile_update_queue.hpp>

#include <noggit/Log.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/World.h>


namespace noggit
{
  struct instance_update
  {
    virtual void apply(World* const) = 0;
    virtual ~instance_update() = default;
  };

  struct model_instance_update : public instance_update
  {
    model_instance_update() = delete;
    model_instance_update(model_instance_update const&) = delete;
    model_instance_update(model_instance_update&&) = default;
    model_instance_update& operator= (model_instance_update const&) = delete;
    model_instance_update& operator= (model_instance_update&&) = default;

    model_instance_update(ModelInstance* m2, model_update type)
      : instance(m2)
      , update_type(type)
    {

    }

    virtual void apply(World* const world) override
    {
      instance->model->wait_until_loaded();
      auto const& extents(instance->extents());
      tile_index start(extents[0]), end(extents[1]);

      for (int z = start.z; z <= end.z; ++z)
      {
        for (int x = start.x; x <= end.x; ++x)
        {
          world->mapIndex.update_model_tile(tile_index(x, z), update_type, instance->uid);
        }
      }
    }

    ModelInstance* instance;
    model_update update_type;
  };

  struct wmo_instance_update : public instance_update
  {
    wmo_instance_update() = delete;
    wmo_instance_update(wmo_instance_update const&) = delete;
    wmo_instance_update(wmo_instance_update&&) = default;
    wmo_instance_update& operator= (wmo_instance_update const&) = delete;
    wmo_instance_update& operator= (wmo_instance_update&&) = default;

    wmo_instance_update(WMOInstance* wmo, model_update type)
      : start(wmo->extents[0])
      , end(wmo->extents[1])
      , uid(wmo->mUniqueID)
      , update_type(type)
    {

    }

    virtual void apply(World* const world) override
    {
      for (int z = start.z; z <= end.z; ++z)
      {
        for (int x = start.x; x <= end.x; ++x)
        {
          world->mapIndex.update_model_tile(tile_index(x, z), update_type, uid);
        }
      }
    }

    tile_index start;
    tile_index end;
    std::uint32_t uid;
    model_update update_type;
  };

  world_tile_update_queue::world_tile_update_queue(World* world)
    : _world(world)
  {
    _thread = std::make_unique<std::thread>(&world_tile_update_queue::process_queue, this);
  }

  world_tile_update_queue::~world_tile_update_queue()
  {
    _stop = true;
    _state_changed.notify_all();

    _thread->join();

    if (!_update_queue.empty())
    {
      LogError << "Update queue deleted with some update pending !" << std::endl;
    }
  }

  void world_tile_update_queue::wait_for_all_update()
  {
    std::unique_lock<std::mutex> lock (_mutex);

    _state_changed.wait
    ( lock
    , [&]
      {
        return _update_queue.empty();
      }
    );
  }

  void world_tile_update_queue::queue_update(ModelInstance* instance, model_update type)
  {
    {
      std::lock_guard<std::mutex> const lock(_mutex);

      _update_queue.emplace(new model_instance_update(instance, type));
      _state_changed.notify_one();
    }
    // make sure deletion are done here
    // otherwise the instance get deleted
    if (type == model_update::remove)
    {
      // wait for all update to make sure they are done in the right order
      wait_for_all_update();
    }
  }
  void world_tile_update_queue::queue_update(WMOInstance* instance, model_update type)
  {
    std::lock_guard<std::mutex> const lock (_mutex);

    _update_queue.emplace(new wmo_instance_update(instance, type));
    _state_changed.notify_one();
  }

  void world_tile_update_queue::process_queue()
  {
    instance_update* update;

    while(!_stop.load())
    {
      {
        std::unique_lock<std::mutex> lock(_mutex);

        _state_changed.wait
        ( lock
        , [&]
          {
            return _stop.load() || !_update_queue.empty();
          }
        );

        if (_stop.load())
        {
          return;
        }

        update = _update_queue.front().get();
      }

      update->apply(_world);

      {
        std::lock_guard<std::mutex> const lock (_mutex);
        _update_queue.pop();
        _state_changed.notify_all();
      }
    }
  }
}
