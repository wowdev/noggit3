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
	float HaveWater(int i, int j);
	void CropMiniChunk(int i, int j, MapChunk* chunkTerrain);

	void autoGen(int factor);

	void setHeight(float height);
	void setHeight(int i, int j, float height);
	float getHeight();
	float getHeightChunk(int i, int j);

	void setTrans(unsigned char opacity);
	unsigned char getOpacity();

	void setType(int type);
	int getType();

	void addLayer();
	void addLayer(int i, int j);
	void addLayer(float height, unsigned char trans);
	void addLayer(int i, int j, float height, unsigned char trans);

	void deleteLayer();
	void deleteLayer(int i, int j);

private:
	void reload();

	MapTile *tile;
	ChunkWater *chunks[16][16];

	float xbase;
	float zbase;
};


#endif
