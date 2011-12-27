#ifndef WMOINSTANCE_H
#define WMOINSTANCE_H

#include <set>
#include <stdint.h>

#include <noggit/Vec3D.h> // Vec3D

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
  Vec3D pos;
  Vec3D  extents[2];
  Vec3D  dir;
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

  void draw (bool draw_doodads, bool draw_fog) const;
  void drawSelect (bool draw_doodads);

  void resetDirection();

private:
  World* _world;
};


#endif // WMOINSTANCE_H
