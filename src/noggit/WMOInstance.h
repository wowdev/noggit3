// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <math/vector_3d.hpp> // math::vector_3d
#include <noggit/WMO.h>

#include <cstdint>
#include <set>

class MPQFile;
struct ENTRY_MODF;

class WMOInstance
{
public:
  scoped_wmo_reference wmo;
  math::vector_3d pos;
  math::vector_3d  extents[2];
  math::vector_3d  dir;
  unsigned int mUniqueID;
  uint16_t mFlags;
  uint16_t mUnknown;
  uint16_t mNameset; 

  uint16_t doodadset() const { return _doodadset; }
  void change_doodadset(uint16_t doodad_set);

private:
  void update_doodads();
  
  uint16_t _doodadset;

  std::map<uint32_t, std::vector<wmo_doodad_instance>> _doodads_per_group;

public:
  WMOInstance(std::string const& filename, ENTRY_MODF const* d);
  explicit WMOInstance(std::string const& filename);

  WMOInstance(WMOInstance const& other) = default;
  WMOInstance& operator=(WMOInstance const& other) = default;

  WMOInstance (WMOInstance&& other)
    : wmo (std::move (other.wmo))
    , pos (other.pos)
    , dir (other.dir)
    , mUniqueID (other.mUniqueID)
    , mFlags (other.mFlags)
    , mUnknown (other.mUnknown)
    , mNameset (other.mNameset)
    , _doodadset (other._doodadset)
    , _doodads_per_group(other._doodads_per_group)

  {
    std::swap (extents, other.extents);
  }

  WMOInstance& operator= (WMOInstance&& other)
  {
    std::swap(wmo, other.wmo);
    std::swap(pos, other.pos);
    std::swap(extents, other.extents);
    std::swap(dir, other.dir);
    std::swap(mUniqueID, other.mUniqueID);
    std::swap(mFlags, other.mFlags);
    std::swap(mUnknown, other.mUnknown);
    std::swap(mNameset, other.mNameset);
    std::swap(_doodadset, other._doodadset);
    std::swap(_doodads_per_group, other._doodads_per_group);
    return *this;
  }

  void draw ( math::frustum const&
            , const float&
            , const math::vector_3d&
            , bool force_box
            , bool draw_doodads
            , bool draw_fog
            , math::vector_4d const& ocean_color_light
            , math::vector_4d const& ocean_color_dark
            , math::vector_4d const& river_color_light
            , math::vector_4d const& river_color_dark
            , liquid_render& render
            , boost::optional<selection_type> selection
            , int animtime
            , std::function<void (bool)> setup_outdoor_lights
            , bool world_has_skies
            , std::function<void (bool)> setup_fog
            );
  void intersect (math::ray const&, selection_result*);

  void recalcExtents();
  void resetDirection();

  bool isInsideRect(math::vector_3d rect[2]) const;

  std::vector<wmo_doodad_instance*> get_visible_doodads( math::frustum const& frustum
                                                       , const float& cull_distance
                                                       , const math::vector_3d& camera
                                                       );
};
