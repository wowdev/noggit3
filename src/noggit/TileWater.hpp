// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/ChunkWater.hpp>
#include <noggit/MPQ.h>
#include <noggit/MapHeaders.h>

#include <memory>

class MapTile;
class sExtendableArray;

class TileWater
{
public:
  TileWater(MapTile *pTile, float pXbase, float pZbase);

  ChunkWater* getChunk(int x, int z);

  void readFromFile(MPQFile &theFile, size_t basePos);
  void saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition);

  void draw ( opengl::scoped::use_program& water_shader
            , math::vector_3d water_color_light
            , math::vector_3d water_color_dark
            , int animtime
            , int layer
            );
  bool hasData(size_t layer);
  void CropMiniChunk(int x, int z, MapChunk* chunkTerrain);

  void autoGen(float factor);

  void setType(int type, size_t layer);
  int getType(size_t layer);

private:

  MapTile *tile;
  std::unique_ptr<ChunkWater> chunks[16][16];

  float xbase;
  float zbase;
};
