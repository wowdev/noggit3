// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <vector>
#include <string>

#include <math/vector_3d.h>

#include <noggit/DBCFile.h>
#include <noggit/ModelManager.h>

namespace noggit
{
  namespace mpq
  {
    class file;
  }
}

class Model;
class World;

struct SkyColor {
  ::math::vector_3d color;
  int time;

  SkyColor( int t, int col );
};

class Sky {
public:
  Model *alt_sky;

  ::math::vector_3d pos;
  float r1, r2;

  explicit Sky( DBCFile::Iterator data );

  std::vector<SkyColor> colorRows[36];
  int mmin[36];

  char name[32];

  ::math::vector_3d colorFor(int r, int t) const;

  float weight;
  bool global;

    bool operator<(const Sky& s) const
  {
    if (global) return false;
    else if (s.global) return true;
    else return r2 < s.r2;
  }
};

enum SkyColorNames {
  LIGHT_GLOBAL_DIFFUSE,
  LIGHT_GLOBAL_AMBIENT,
  SKY_COLOR_0,
  SKY_COLOR_1,
  SKY_COLOR_2,
  SKY_COLOR_3,
  SKY_COLOR_4,
  FOG_COLOR,
  SKY_UNKNOWN_1,
  SUN_COLOR,
  SUN_HALO_COLOR,
  SKY_UNKNOWN_2,
  CLOUD_COLOR,
  SKY_UNKNOWN_3,
  SKY_UNKNOWN_4,
  WATER_COLOR_DARK,
  WATER_COLOR_LIGHT,
  SHADOW_COLOR,
  NUM_SkyColorNames
};

class Skies {

  int numSkies;
  int cs;
  noggit::scoped_model_reference stars;
  char skyname[128];

public:
  std::vector<Sky> skies;
  ::math::vector_3d colorSet[NUM_SkyColorNames];

  explicit Skies( unsigned int mapid );

  void findSkyWeights(::math::vector_3d pos);
  void initSky(::math::vector_3d pos, int t);

  void draw() const;

  bool drawSky (World* world, const ::math::vector_3d &pos) const;
  bool hasSkies() { return numSkies > 0; }

  void setupLighting();

  void debugDraw(unsigned int *buf, int dim);
};


/*
  It seems that lighting info is also stored in lights.lit, so I
  wonder what the heck is in Dnc.db. Maybe just light directions and/or
  sun/moon positions...?
*/
struct OutdoorLightStats {
  int time; // converted from hour:min to the 2880 half-minute ticks thing used in the other Sky thing

  float dayIntensity, nightIntensity, ambientIntensity, fogIntensity, fogDepth;
  ::math::vector_3d dayColor, nightColor, ambientColor, fogColor, dayDir, nightDir;

  void init(noggit::mpq::file* f);

  void interpolate(OutdoorLightStats *a, OutdoorLightStats *b, float r);
  void setupLighting();
    // void setupFog(); //! \todo  add fog maybe?

};


class OutdoorLighting {

  std::vector<OutdoorLightStats> lightStats;

public:
  explicit OutdoorLighting( const std::string& fname );

  OutdoorLightStats getLightStats(int time);

};
