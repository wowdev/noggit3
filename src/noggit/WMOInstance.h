// WMOInstance.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef WMOINSTANCE_H
#define WMOINSTANCE_H

#include <set>
#include <stdint.h>

#include <math/vector_3d.h>

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
  WMO* wmo;
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
  WMOInstance( World*, WMO* _wmo, noggit::mpq::file* _file );
  WMOInstance( World*, WMO* _wmo, ENTRY_MODF* d );
  explicit WMOInstance( World*, WMO* _wmo );
  ~WMOInstance();

  void draw (bool draw_doodads, bool draw_fog, bool hasSkies, const float animtime, const float culldistance) const;
  void drawSelect (bool draw_doodads, const float animtime, const float culldistance);

  void resetDirection();

private:
  World* _world;
};


#endif // WMOINSTANCE_H
