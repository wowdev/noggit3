// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/ChunkWater.hpp>
#include <noggit/MPQ.h>
#include <noggit/MapHeaders.h>
#include <noggit/tool_enums.hpp>
#include <util/sExtendableArray.hpp>

#include <memory>

class MapTile;

class TileWater
{
public:
  TileWater(MapTile *pTile, float pXbase, float pZbase, bool use_mclq_green_lava);

  ChunkWater* getChunk(int x, int z);

  void readFromFile(MPQFile &theFile, size_t basePos);
  void saveToFile(util::sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition);

  void draw ( math::frustum const& frustum
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool camera_moved
            , liquid_render& render
            , opengl::scoped::use_program& water_shader
            , int animtime
            , int layer
            , display_mode display
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
