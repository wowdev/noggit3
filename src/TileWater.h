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
  bool hasData(size_t layer);
	float HaveWater(int i, int j);
	void CropMiniChunk(int i, int j, MapChunk* chunkTerrain);

	void autoGen(int factor);

	void setHeight(float height, size_t layer);
	void setHeight(int i, int j, float height, size_t layer);
	float getHeight(size_t layer);
	float getHeightChunk(int i, int j, size_t layer);

	void setTrans(unsigned char opacity, size_t layer);
	unsigned char getOpacity(size_t layer);

	void setType(int type, size_t layer);
	int getType(size_t layer);

	void addLayer(size_t layer);
	void addLayer(int i, int j, size_t layer);
	void addLayer(float height, unsigned char trans, size_t layer);
	void addLayer(int i, int j, float height, unsigned char trans, size_t layer);

	void deleteLayer(size_t layer);
	void deleteLayer(int i, int j, size_t layer);

private:
	void reload();

	MapTile *tile;
	ChunkWater *chunks[16][16];

	float xbase;
	float zbase;
};


#endif
