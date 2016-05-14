// WMOInstance.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef WMOINSTANCE_H
#define WMOINSTANCE_H

#include <set>
#include <stdint.h>

#include <boost/optional.hpp>

#include <math/vector_3d.h>

#include <noggit/Selection.h>
#include <noggit/WMO.h>

class Frustum;
class WMO;
class World;

struct ENTRY_MODF;

namespace noggit
{
  namespace mpq
  {
    class file;
  }
}

class WMOInstance
{
public:
  scoped_wmo_reference wmo;
  ::math::vector_3d pos;
  ::math::vector_3d  extents[2];
  ::math::vector_3d  dir;
  int mUniqueID;
  uint16_t mFlags;
  uint16_t mUnknown;
  uint16_t mNameset;
  uint16_t doodadset;

private:
  unsigned int mSelectionID;

public:
  WMOInstance( World*, std::string const& path, noggit::mpq::file* _file );
  WMOInstance( World*, std::string const& path, ENTRY_MODF* d );
  explicit WMOInstance( World*, std::string const& path );
  ~WMOInstance();

  void draw ( bool draw_doodads
            , bool draw_fog
            , bool hasSkies
            , const float culldistance
            , const float& fog_distance
            , const Frustum& frustum
            , const ::math::vector_3d& camera
            , const boost::optional<selection_type>& selected_item
            ) const;
  void drawSelect ( bool draw_doodads
                  , const float culldistance
                  , const Frustum& frustum
                  , const ::math::vector_3d& camera
                  ) const;

  void resetDirection();

  void recalc_extents();

private:
  World* _world;
};


#endif // WMOINSTANCE_H
