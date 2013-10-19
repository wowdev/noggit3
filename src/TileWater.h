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
  TileWater(MapTile *pTile, float pXbase, float pZbase);
  ~TileWater(void);

  ChunkWater* getChunk(int x, int y);

  void readFromFile(MPQFile &theFile, size_t basePos);
  void saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition);

  void draw();
  bool hasData();

  void autoGen();

  void setHeight(float height);
  float getHeight();

  void setTrans(unsigned char waterOpercity);
  unsigned char getOpercity();

  void setType(int type);
  int getType();

  void addLayer();
  void addLayer(float height, unsigned char trans);

  void deleteLayer();

private:
  void reload();

  MapTile *tile;
  ChunkWater *chunks[16][16];

  float xbase;
  float zbase;
};


#endif
