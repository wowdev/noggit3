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
  uint16_t doodadset;

public:
  WMOInstance(std::string const& filename, MPQFile* _file);
  WMOInstance(std::string const& filename, ENTRY_MODF* d);
  explicit WMOInstance(std::string const& filename);

  WMOInstance(WMOInstance const& other) = default;
  WMOInstance& operator=(WMOInstance const& other) = default;

  WMOInstance (WMOInstance&& other)
    : wmo (std::move (other.wmo))
    , pos (other.pos)
    // , extents (other.extents)
    , dir (other.dir)
    , mUniqueID (other.mUniqueID)
    , mFlags (other.mFlags)
    , mUnknown (other.mUnknown)
    , mNameset (other.mNameset)
    , doodadset (other.doodadset)

  {
  //  std::move(std::begin(other.extents), std::end(other.extents), extents);
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
    std::swap(doodadset, other.doodadset);
    return *this;
  }

  void draw (math::frustum const&, bool force_box, bool draw_doodads);
  void intersect (math::ray const&, selection_result*);

  void recalcExtents();
  void resetDirection();

  bool isInsideRect(math::vector_3d rect[2]);
};
