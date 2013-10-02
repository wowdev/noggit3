#ifndef TILEWATER_H
#define TILEWATER_H

#include "MPQ.h"
#include "MapHeaders.h"
#include "MapTile.h"
#include "Vec3D.h"

class ChunkWater;
class sExtendableArray;

class TileWater
{

public:
  TileWater(float pXbase, float pZbase);
  ~TileWater(void);

  void readFromFile(MPQFile &theFile, size_t basePos);
  void saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition);

  void draw();

  void setLevel(int waterLevel);
  int getLevel();
  void setOpercity(int waterOpercity);
  int getOpercity();
  void setType(int waterType);
  int getType();

private:
  void reload();

  ChunkWater *chunks[16][16];

  bool hasWater;
  float xbase;
  float zbase;
};

#endif
