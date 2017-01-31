// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/MapHeaders.h>

#include <vector>

class MPQFile;
class Liquid;
class sExtendableArray;
class MapChunk;

class ChunkWater
{
public:
  ChunkWater(float pX, float pY);
  ~ChunkWater();

  void fromFile(MPQFile &f, size_t basePos);
  void reloadRendering();
  void draw();

  void writeHeader(sExtendableArray &lADTFile, int &lCurrentPosition);
  void writeInfo(sExtendableArray &lADTFile, size_t basePos, int &lCurrentPosition);
  void writeData(size_t offHeader, sExtendableArray &lADTFile, size_t basePos, int &lCurrentPosition);

  void autoGen(MapChunk* chunk, int factor);

  void setHeight(float height, size_t layer);
  void setHeight(size_t x, size_t y, float height, size_t layer);
  float getHeight(size_t layer);
  float getHeight(size_t x, size_t y, size_t layer);
  void CropWater(MapChunk* chunkTerrain);

  void setTrans(unsigned char trans, size_t layer);
  void setTrans(size_t x, size_t y, unsigned char trans, size_t layer);
  unsigned char getTrans(size_t layer);
  unsigned char getTrans(size_t x, size_t y, size_t layer);

  void setType(int type, size_t layer);
  int getType(size_t layer);

  void addLayer(size_t layer);
  void addLayer(size_t x, size_t y, size_t layer);
  void cleanLayer(size_t layer);

  void deleteLayer(size_t layer);
  void deleteLayer(size_t x, size_t y, size_t layer);

  bool hasData(size_t layer);

private:

  bool subchunkHasWater(size_t x, size_t y, size_t layer);

  MH2O_Render Render;

  std::vector<Liquid> _liquids;
  

  float x, y;
};
