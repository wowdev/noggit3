#ifndef MAPTILE_H
#define MAPTILE_H

#include <vector>
#include <map>
#include <string>

#include "MapHeaders.h"
#include "Video.h" // GLfloat, GLshort, ...

class Vec3D;
class TileWater;
class MapChunk;

class MapTile
{

public:
	MapTile(int x0, int z0, const std::string& pFilename, bool pBigAlpha);
	~MapTile();
	//! \brief Get the maximum height of terrain on this map tile.
	float getMaxHeight();

	//! \brief Get chunk for sub offset x,z.
	MapChunk* getChunk(unsigned int x, unsigned int z);

	int modelCount;
	int mPositionX;
	int mPositionZ;
	float xbase, zbase;

	int changed;

	void draw();
	void drawSelect();
	void drawLines();
	void drawWater();
	void drawTextures();
	void drawMFBO();

	bool GetVertex(float x, float z, Vec3D *V);

	void saveTile();
	void FixGapt();
	void FixAllGapt(MapTile *next, bool a);
	void CropWater();
	void ClearShader();

	bool isTile(int pX, int pZ);
	void clearAllModels();
	void addChunksLiquid(TileWater *lq);

	bool canWaterSave();

	void getAlpha(size_t id, unsigned char *amap);

	TileWater * Water;
private:

	// MFBO:
	GLfloat mMinimumValues[3 * 3 * 3];
	GLfloat mMaximumValues[3 * 3 * 3];

	// MHDR:
	int mFlags;
	bool mBigAlpha;

	// Data to be loaded and later unloaded.
	std::vector<std::string> mTextureFilenames;
	std::vector<std::string> mModelFilenames;
	std::vector<std::string> mWMOFilenames;

	std::string mFilename;

	MapChunk* mChunks[16][16];
	std::vector<TileWater*> mLiquids;
	std::vector<TileWater*> chunksLiquids; //map chunks liquids for old style water render!!! (Not MH2O)

	friend class MapChunk;
	friend class TextureSet;
};

int indexMapBuf(int x, int y);

//! \todo get stripify related functions somewhere else.
// unused
// 8x8x2 version with triangle strips, size = 8*18 + 7*2
/*const int stripsize = 8*18 + 7*2;
template <class V>
void stripify(V *in, V *out)
{
for (int row=0; row<8; row++) {
V *thisrow = &in[indexMapBuf(0,row*2)];
V *nextrow = &in[indexMapBuf(0,(row+1)*2)];

if (row>0) *out++ = thisrow[0];
for (int col=0; col<9; col++) {
*out++ = thisrow[col];
*out++ = nextrow[col];
}
if (row<7) *out++ = nextrow[8];
}
}*/

// high res version, size = 16*18 + 7*2 + 8*2
const int stripsize2 = 16 * 18 + 7 * 2 + 8 * 2;
template <class V>
void stripify2(V *in, V *out)
{
	for (int row = 0; row<8; row++) {
		V *thisrow = &in[indexMapBuf(0, row * 2)];
		V *nextrow = &in[indexMapBuf(0, row * 2 + 1)];
		V *overrow = &in[indexMapBuf(0, (row + 1) * 2)];

		if (row>0) *out++ = thisrow[0];// jump end
		for (int col = 0; col<8; col++) {
			*out++ = thisrow[col];
			*out++ = nextrow[col];
		}
		*out++ = thisrow[8];
		*out++ = overrow[8];
		*out++ = overrow[8];// jump start
		*out++ = thisrow[0];// jump end
		*out++ = thisrow[0];
		for (int col = 0; col<8; col++) {
			*out++ = overrow[col];
			*out++ = nextrow[col];
		}
		if (row<8) *out++ = overrow[8];
		if (row<7) *out++ = overrow[8];// jump start
	}
}



#endif
