// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/MPQ.h>
#include <noggit/MapHeaders.h>
#include <noggit/MapTile.h>

class ChunkWater;
class sExtendableArray;

class TileWater
{
public:
  TileWater(MapTile *pTile, float pXbase, float pZbase);
  ~TileWater(void);

  ChunkWater* getChunk(int x, int z);

  void readFromFile(MPQFile &theFile, size_t basePos);
  void saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition);

  void draw();
  bool hasData(size_t layer);
  float HaveWater(int x, int z);
  void CropMiniChunk(int x, int z, MapChunk* chunkTerrain);

  void autoGen(int factor);

  void setHeight(float height, size_t layer);
  void setHeight(int x, int z, float height, size_t layer);
  float getHeight(size_t layer);
  float getHeightChunk(int x, int z, size_t layer);

  void setTrans(unsigned char opacity, size_t layer);
  unsigned char getOpacity(size_t layer);

  void setType(int type, size_t layer);
  int getType(size_t layer);

  void addLayer(size_t layer);
  void addLayer(int x, int z, size_t layer);
  void addLayer(float height, unsigned char trans, size_t layer);
  void addLayer(int x, int z, float height, unsigned char trans, size_t layer);

  void deleteLayer(size_t layer);
  void deleteLayer(int x, int z, size_t layer);

private:
  void reload();

  MapTile *tile;
  ChunkWater *chunks[16][16];

  float xbase;
  float zbase;
};
