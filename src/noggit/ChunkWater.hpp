// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/MapHeaders.h>

#include <vector>
#include <set>

class MPQFile;
class Liquid;
class sExtendableArray;
class MapChunk;

class ChunkWater
{
public:
  ChunkWater(float x, float z);
  ~ChunkWater();

  void fromFile(MPQFile &f, size_t basePos);
  void reloadRendering();
  void draw();


  void save(sExtendableArray& adt, int base_pos, int& header_pos, int& current_pos);

  void autoGen(MapChunk* chunk, float factor);
  void CropWater(MapChunk* chunkTerrain);

  void setType(int type, size_t layer);
  int getType(size_t layer) const;

  bool hasData(size_t layer) const;

  float xbase, zbase;

private:

  MH2O_Render Render;

  std::vector<Liquid> _liquids;
};
