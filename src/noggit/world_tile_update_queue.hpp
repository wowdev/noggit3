// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/map_enums.hpp>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

class ModelInstance;
class WMOInstance;
class World;

namespace noggit
{
  struct instance_update;

  class world_tile_update_queue
  {
  public:
    world_tile_update_queue(World* world);
    ~world_tile_update_queue();

    world_tile_update_queue() = delete;
    world_tile_update_queue(world_tile_update_queue const&) = delete;
    world_tile_update_queue(world_tile_update_queue&&) = delete;
    world_tile_update_queue& operator= (world_tile_update_queue const&) = delete;
    world_tile_update_queue& operator= (world_tile_update_queue&&) = delete;

    void wait_for_all_update();

    void queue_update(ModelInstance* instance, model_update type);
    void queue_update(WMOInstance* instance, model_update type);

  private:
    void process_queue();
  private:
    World* _world;

    std::atomic<bool> _stop = {false};
    std::mutex _mutex;
    std::condition_variable _state_changed;

    // only use one thread
    std::unique_ptr<std::thread> _thread;

    std::queue<std::unique_ptr<instance_update>> _update_queue;
  };
}
