#pragma once

#include "MapHeaders.h"

class MPQFile;
class Liquid;

class ChunkWater
{
public:
  ChunkWater(size_t pX, size_t pY);
  ~ChunkWater();

  void fromFile(MPQFile &f, size_t basePos);
  void reloadRendering();
  void draw();

private:
  MH2O_Header Header;
  MH2O_Information Info[5];
  MH2O_HeightMask HeightData[5];
  char *InfoMask[5];
  MH2O_Render Render[5];
  Liquid * Liquids[5];

  size_t x, y;
};
