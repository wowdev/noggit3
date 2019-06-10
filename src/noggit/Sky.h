// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/DBCFile.h>
#include <noggit/MPQ.h>
#include <noggit/ModelManager.h>
#include <opengl/scoped.hpp>
#include <opengl/shader.fwd.hpp>

#include <memory>
#include <string>
#include <vector>

class Model;

struct SkyColor {
  math::vector_3d color;
  int time;

  SkyColor(int t, int col);
};

class Sky {
public:
  Model *alt_sky;

  math::vector_3d pos;
  float r1, r2;

  explicit Sky(DBCFile::Iterator data);

  std::vector<SkyColor> colorRows[36];
  int mmin[36];

  char name[32];

  math::vector_3d colorFor(int r, int t) const;

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
  OCEAN_COLOR_LIGHT,
  OCEAN_COLOR_DARK,
  RIVER_COLOR_LIGHT,
  RIVER_COLOR_DARK,
  NUM_SkyColorNames,
};

class Skies 
{
private:
  int numSkies = 0;
  int cs = -1;
  scoped_model_reference stars;

  int _last_time = -1;
  math::vector_3d _last_pos;

public:
  std::vector<Sky> skies;
  std::vector<math::vector_3d> color_set = std::vector<math::vector_3d>(NUM_SkyColorNames);

  explicit Skies(unsigned int mapid);

  void findSkyWeights(math::vector_3d pos);
  void update_sky_colors(math::vector_3d pos, int time);

  bool draw ( math::matrix_4x4 const& mvp
            , math::vector_3d const& pos
            , float night_intensity
            , bool draw_fog
            , int animtime
            );
  bool hasSkies() { return numSkies > 0; }

private:
  bool _uploaded = false;
  bool _need_color_buffer_update = true;
  bool _need_vao_update = true;

  int _indices_count;

  void upload();
  void update_color_buffer();
  void update_vao(opengl::scoped::use_program& shader);

  opengl::scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
  opengl::scoped::deferred_upload_buffers<3> _buffers;
  GLuint const& _vertices_vbo = _buffers[0];
  GLuint const& _colors_vbo = _buffers[1];
  GLuint const& _indices_vbo = _buffers[2];

  std::unique_ptr<opengl::program> _program;
};


/*
It seems that lighting info is also stored in lights.lit, so I
wonder what the heck is in Dnc.db. Maybe just light directions and/or
sun/moon positions...?
*/
struct OutdoorLightStats {
  int time; // converted from hour:min to the 2880 half-minute ticks thing used in the other Sky thing

  float dayIntensity, nightIntensity, ambientIntensity, fogIntensity, fogDepth;
  math::vector_3d dayColor, nightColor, ambientColor, fogColor, dayDir, nightDir;

  void init(MPQFile* f);

  void interpolate(OutdoorLightStats *a, OutdoorLightStats *b, float r);
  void setupLighting();
  // void setupFog(); //! \todo  add fog maybe?

};


class OutdoorLighting {

  std::vector<OutdoorLightStats> lightStats;

public:
  explicit OutdoorLighting(const std::string& fname);

  OutdoorLightStats getLightStats(int time);

};
