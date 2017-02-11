// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/liquid_layer.hpp>

#include <vector>
#include <set>

class MPQFile;
class sExtendableArray;
class MapChunk;

class ChunkWater
{
public:
  ChunkWater(float x, float z);

  void fromFile(MPQFile &f, size_t basePos);
  void save(sExtendableArray& adt, int base_pos, int& header_pos, int& current_pos);

  void draw (opengl::scoped::use_program& water_shader);

  void autoGen(MapChunk* chunk, float factor);
  void CropWater(MapChunk* chunkTerrain);

  void setType(int type, size_t layer);
  int getType(size_t layer) const;
  bool hasData(size_t layer) const;

  void paintLiquid(math::vector_3d const& pos
    , float radius
    , int liquid_id
    , bool add
    , math::radians const& angle
    , math::radians const& orientation
    , bool lock
    , math::vector_3d const& origin
  );


  float xbase, zbase;

private:
  // remove empty layers
  void cleanup();
  // update every layer's render
  void update_layers();


  MH2O_Render Render;

  std::vector<liquid_layer> _layers;
};
