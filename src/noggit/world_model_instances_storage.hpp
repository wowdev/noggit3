// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/tile_index.hpp>
#include <noggit/WMOInstance.h>


#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_map>

class World;

using m2_instance_umap = std::unordered_map<std::uint32_t, ModelInstance>;
using wmo_instance_umap = std::unordered_map<std::uint32_t, WMOInstance>;

namespace noggit
{
  class world_model_instances_storage
  {
  public:
    world_model_instances_storage(World* world);

    world_model_instances_storage() = delete;
    world_model_instances_storage(world_model_instances_storage const&) = delete;
    world_model_instances_storage(world_model_instances_storage&&) = delete;
    world_model_instances_storage& operator= (world_model_instances_storage const&) = delete;
    world_model_instances_storage& operator= (world_model_instances_storage&&) = delete;

    // perform uid duplicate check, return the uid of the stored instance
    std::uint32_t add_model_instance(ModelInstance instance, bool from_reloading);
    // perform uid duplicate check, return the uid of the stored instance
    std::uint32_t add_wmo_instance(WMOInstance instance, bool from_reloading);

    boost::optional<ModelInstance*> get_model_instance(std::uint32_t uid);
    boost::optional<WMOInstance*> get_wmo_instance(std::uint32_t uid);
    boost::optional<selection_type> get_instance(std::uint32_t uid);

    void delete_instances_from_tile(tile_index const& tile);
    void delete_instances(std::vector<selection_type> const& instances);
    void delete_instance(std::uint32_t uid);
    void unload_instance_and_remove_from_selection_if_necessary(std::uint32_t uid);

    void clear();

    void clear_duplicates();

    bool uid_duplicates_found() const
    {
      return _uid_duplicates_found.load();
    }

  private: // private functions aren't thread safe
    inline bool unsafe_uid_is_used(std::uint32_t uid) const;

    std::uint32_t unsafe_add_model_instance_no_world_upd(ModelInstance instance);
    std::uint32_t unsafe_add_wmo_instance_no_world_upd(WMOInstance instance);
    boost::optional<ModelInstance*> unsafe_get_model_instance(std::uint32_t uid);
    boost::optional<WMOInstance*> unsafe_get_wmo_instance(std::uint32_t uid);

  public:
    template<typename Fun>
      void for_each_wmo_instance(Fun&& function)
    {
      std::unique_lock<std::mutex> const lock (_mutex);

      for (auto& it : _wmos)
      {
        function(it.second);
      }
    }

    template<typename Fun, typename Stop>
      void for_each_wmo_instance(Fun&& function, Stop&& stop_cond)
    {
      std::unique_lock<std::mutex> const lock (_mutex);

      for (auto& it : _wmos)
      {
        function(it.second);

        if (stop_cond())
        {
          break;
        }
      }
    }

    template<typename Fun>
      void for_each_m2_instance(Fun&& function)
    {
      std::unique_lock<std::mutex> const lock (_mutex);

      for (auto& it : _m2s)
      {
        function(it.second);
      }
    }

  private:
    World* _world;
    std::mutex _mutex;
    std::atomic<bool> _uid_duplicates_found = {false};

    m2_instance_umap _m2s;
    wmo_instance_umap _wmos;

    std::unordered_map<std::uint32_t, int> _instance_count_per_uid;
  };
}
