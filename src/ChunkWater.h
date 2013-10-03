#pragma once

#include "MapHeaders.h"

class MPQFile;
class Liquid;
class sExtendableArray;

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

private:
  MH2O_Header Header;
  MH2O_Information Info[5];
  MH2O_HeightMask HeightData[5];
  char *InfoMask[5];
  MH2O_Render Render[5];
  Liquid * Liquids[5];

  float x, y;
};
