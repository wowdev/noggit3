// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/world_model_instances_storage.hpp>

#include <noggit/World.h>

namespace noggit
{
  world_model_instances_storage::world_model_instances_storage(World* world)
    : _world(world)
  {

  }

  std::uint32_t world_model_instances_storage::add_model_instance(ModelInstance instance, bool from_reloading)
  {
    std::uint32_t uid = instance.uid;
    std::uint32_t uid_after;

    {
      std::lock_guard<std::mutex> const lock (_mutex);
      uid_after = unsafe_add_model_instance_no_world_upd(std::move(instance));
    }

    if (from_reloading || uid_after != uid)
    {
      _world->updateTilesModel(&_m2s.at(uid_after), model_update::add);
    }

    return instance.uid;
  }
  std::uint32_t world_model_instances_storage::unsafe_add_model_instance_no_world_upd(ModelInstance instance)
  {
    std::uint32_t uid = instance.uid;
    auto existing_instance = unsafe_get_model_instance(uid);

    if (existing_instance)
    {
      // instance already loaded
      if (existing_instance.get()->is_a_duplicate_of(instance))
      {
        _instance_count_per_uid[uid]++;
        return uid;
      }
    }
    else if(!unsafe_uid_is_used(uid))
    {
      _m2s.emplace(uid, instance);
      _instance_count_per_uid[uid] = 1;
      return uid;
    }

    // the uid is already used for another model/wmo, use a new one
    _uid_duplicates_found = true;
    instance.uid = _world->mapIndex.newGUID();

    return unsafe_add_model_instance_no_world_upd(std::move(instance));
  }

  
  std::uint32_t world_model_instances_storage::add_wmo_instance(WMOInstance instance, bool from_reloading)
  {
    std::uint32_t uid = instance.mUniqueID;
    std::uint32_t uid_after;

    {
      std::lock_guard<std::mutex> const lock(_mutex);
      uid_after = unsafe_add_wmo_instance_no_world_upd(std::move(instance));
    }

    if (from_reloading || uid_after != uid)
    {
      _world->updateTilesWMO(&_wmos.at(uid_after), model_update::add);
    }

    return instance.mUniqueID;
  }
  std::uint32_t world_model_instances_storage::unsafe_add_wmo_instance_no_world_upd(WMOInstance instance)
  {
    std::uint32_t uid = instance.mUniqueID;
    auto existing_instance = unsafe_get_wmo_instance(uid);

    if (existing_instance)
    {
      // instance already loaded
      if (existing_instance.get()->is_a_duplicate_of(instance))
      {
        _instance_count_per_uid[uid]++;

        return uid;
      }
    }
    else if (!unsafe_uid_is_used(uid))
    {
      _wmos.emplace(uid, instance);
      _instance_count_per_uid[uid] = 1;
      return uid;
    }

    // the uid is already used for another model/wmo, use a new one
    _uid_duplicates_found = true;
    instance.mUniqueID = _world->mapIndex.newGUID();

    return unsafe_add_wmo_instance_no_world_upd(std::move(instance));
  }

  void world_model_instances_storage::delete_instances_from_tile(tile_index const& tile)
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    for (auto it = _m2s.begin(); it != _m2s.end();)
    {
      if (tile_index(it->second.pos) == tile)
      {
        _world->updateTilesModel(&it->second, model_update::remove);
        _instance_count_per_uid.erase(it->first);
        it = _m2s.erase(it);
      }
      else
      {
        it++;
      }
    }
    for (auto it = _wmos.begin(); it != _wmos.end();)
    {
      if (tile_index(it->second.pos) == tile)
      {
        _world->updateTilesWMO(&it->second, model_update::remove);
        _instance_count_per_uid.erase(it->first);
        it = _wmos.erase(it);
      }
      else
      {
        it++;
      }
    }
  }

  void world_model_instances_storage::delete_instances(std::vector<selection_type> const& instances)
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    for (auto& it : instances)
    {
      if (it.which() == eEntry_Model)
      {
        auto& instance = boost::get<selected_model_type>(it);
        
        _world->updateTilesModel(instance, model_update::remove);

        _instance_count_per_uid.erase(instance->uid);
        _m2s.erase(instance->uid);
      }
      else if (it.which() == eEntry_WMO)
      {
        auto& instance = boost::get<selected_wmo_type>(it);

        _world->updateTilesWMO(instance, model_update::remove);

        _instance_count_per_uid.erase(instance->mUniqueID);
        _wmos.erase(instance->mUniqueID);
      }
    }
  }

  void world_model_instances_storage::delete_instance(std::uint32_t uid)
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    _instance_count_per_uid.erase(uid);
    _m2s.erase(uid);
    _wmos.erase(uid);
  }

  void world_model_instances_storage::unload_instance_and_remove_from_selection_if_necessary(std::uint32_t uid)
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    if (!unsafe_uid_is_used(uid))
    {
      LogError << "Trying to unload an instance that wasn't stored" << std::endl;
      return;
    }

    if (--_instance_count_per_uid.at(uid) == 0)
    {
      _world->remove_from_selection(uid);

      _instance_count_per_uid.erase(uid);
      _m2s.erase(uid);
      _wmos.erase(uid);
    }
  }

  void world_model_instances_storage::clear()
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    _instance_count_per_uid.clear();
    _m2s.clear();
    _wmos.clear();
  }

  boost::optional<ModelInstance*> world_model_instances_storage::get_model_instance(std::uint32_t uid)
  {
    std::unique_lock<std::mutex> const lock (_mutex);
    return unsafe_get_model_instance(uid);
  }
  boost::optional<ModelInstance*> world_model_instances_storage::unsafe_get_model_instance(std::uint32_t uid)
  {
    auto it = _m2s.find(uid);

    if (it != _m2s.end())
    {
      return &it->second;
    }
    else
    {
      return boost::none;
    }
  }

  boost::optional<WMOInstance*> world_model_instances_storage::get_wmo_instance(std::uint32_t uid)
  {
    std::unique_lock<std::mutex> const lock (_mutex);
    return unsafe_get_wmo_instance(uid);
  }
  boost::optional<WMOInstance*> world_model_instances_storage::unsafe_get_wmo_instance(std::uint32_t uid)
  {
    auto it = _wmos.find(uid);

    if (it != _wmos.end())
    {
      return &it->second;
    }
    else
    {
      return boost::none;
    }
  }

  boost::optional<selection_type> world_model_instances_storage::get_instance(std::uint32_t uid)
  {
    std::unique_lock<std::mutex> const lock (_mutex);
    
    auto wmo_it = _wmos.find(uid);

    if (wmo_it != _wmos.end())
    {
      return selection_type {&wmo_it->second};
    }
    else
    {
      auto m2_it = _m2s.find(uid);

      if (m2_it != _m2s.end())
      {
        return selection_type {&m2_it->second};
      }
      else
      {
        return boost::none;
      }
    }
  }

  bool world_model_instances_storage::unsafe_uid_is_used(std::uint32_t uid) const
  {
    return _instance_count_per_uid.find(uid) != _instance_count_per_uid.end();
  }

  void world_model_instances_storage::clear_duplicates()
  {
    std::unique_lock<std::mutex> const lock (_mutex);

    int deleted_uids = 0;

    for (auto lhs(_wmos.begin()); lhs != _wmos.end(); ++lhs)
    {
      for (auto rhs(std::next(lhs)); rhs != _wmos.end();)
      {
        assert(lhs->first != rhs->first);

        if (lhs->second.is_a_duplicate_of(rhs->second))
        {
          _world->updateTilesWMO(&rhs->second, model_update::remove);

          _instance_count_per_uid.erase(rhs->second.mUniqueID);
          rhs = _wmos.erase(rhs);
          deleted_uids++;
        }
        else
        {
          rhs++;
        }
      }
    }

    for (auto lhs(_m2s.begin()); lhs != _m2s.end(); ++lhs)
    {
      for (auto rhs(std::next(lhs)); rhs != _m2s.end();)
      {
        assert(lhs->first != rhs->first);

        if (lhs->second.is_a_duplicate_of(rhs->second))
        {
          _world->updateTilesModel(&rhs->second, model_update::remove);

          _instance_count_per_uid.erase(rhs->second.uid);
          rhs = _m2s.erase(rhs);
          deleted_uids++;
        }
        else
        {
          rhs++;
        }
      }
    }

    NOGGIT_LOG << "Deleted " << deleted_uids << " duplicate Model/WMO" << std::endl;
  }
}
