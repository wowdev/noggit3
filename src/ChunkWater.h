#pragma once

#include "MapHeaders.h"

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
  void writeInfo(sExtendableArray &lADTFile, MH2O_Header *header, size_t basePos, int &lCurrentPosition);
  void writeData(MH2O_Header *header, MH2O_Information *info, sExtendableArray &lADTFile, size_t basePos, int &lCurrentPosition);

  void autoGen(MapChunk* chunk, int factor);

  void setHeight(float height);
  void setHeight(size_t x, size_t y, float height);
  float getHeight();
  float getHeight(size_t x, size_t y);

  void setTrans(unsigned char trans);
  void setTrans(size_t x, size_t y, unsigned char trans);
  unsigned char getTrans();
  unsigned char getTrans(size_t x, size_t y);

  void setType(int type);
  int getType();

  void addLayer();
  void addLayer(size_t x, size_t y);

  void deleteLayer();
  void deleteLayer(size_t x, size_t y);

  bool hasData();

private:

  bool hasLayer(size_t x, size_t y);

  MH2O_Header Header;
  MH2O_Information Info[5];
  MH2O_HeightMask HeightData[5];
  MH2O_Render Render[5];

  Liquid * Liquids[5];

  bool existsTable[5][8][8];

  uint8_t InfoMask[8];

  float x, y;
};
