#ifndef MAPTILE_H
#define MAPTILE_H

#include <vector>

#include "mapheaders.h"
#include "MapNode.h"
#include "video.h" // GLfloat, GLshort, ...

class Liquid;
class MapChunk;
class MPQFile;

class MapTile 
{
public:
	std::vector<Liquid*> mLiquids;

private:
	// MFBO:
	GLfloat mMinimumValues[3*3*3], mMaximumValues[3*3*3];
	GLshort lIndices[18];

	// MHDR:
	int mFlags;

	// Data to be loaded:
	bool mTexturesLoaded;
	std::vector<std::string> mTextureFilenames;
  
  bool mModelsLoaded;
	std::vector<std::string> mModelFilenames;
  std::vector<ENTRY_MDDF> mModelInstances;
  
  bool mWMOsLoaded;
	std::vector<std::string> mWMOFilenames;
  std::vector<ENTRY_MODF> mWMOInstances;
  

	std::string fname;
	
	MPQFile	*theFile;

	bool	chunksLoaded;
	int		nextChunk;
	size_t mcnk_offsets[256], mcnk_sizes[256];
	void	loadChunk();
	void	finishChunkLoad();

	void loadTexture();
	void finishTextureLoad();
	
	
public:
	void loadModel();
	void loadWMO();

	void finishLoading();
	bool isLoaded(){return mTexturesLoaded&&mModelsLoaded&&mWMOsLoaded;};
	void partialLoad(){
		if( !mTexturesLoaded )
		{
				loadTexture();
		}		
		else if(!chunksLoaded)
		{
			//loadChunk();
			loadChunk();
		}
		else if(!mWMOsLoaded)
			loadWMO();
		else if(!mModelsLoaded)
			loadModel();		
	};
	std::vector<std::string> textures;
	std::vector<std::string> wmos;
	std::vector<std::string> models;

	int x, z;
	bool ok;

	bool mBigAlpha;

	//World *world;

	float xbase, zbase;

	MapChunk * chunks[16][16];

	MapNode topnode;

	MapTile(int x0, int z0, char* filename,bool bigAlpha);
	~MapTile();

	void draw();
	void drawSelect();
	void drawLines();
	void drawWater();
	void drawSky();
	//void drawPortals();
	//void drawModelsMapTile();
	void drawTextures();
	void drawMFBO();

	bool GetVertex(float x,float z, Vec3D *V);
	

	void saveTile();

	/// Get chunk for sub offset x,z
	MapChunk *getChunk(unsigned int x, unsigned int z);
};

int indexMapBuf(int x, int y);

//! \todo get stripify related functions somewhere else.

// 8x8x2 version with triangle strips, size = 8*18 + 7*2
const int stripsize = 8*18 + 7*2;
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
}

// high res version, size = 16*18 + 7*2 + 8*2
const int stripsize2 = 16*18 + 7*2 + 8*2;
template <class V>
void stripify2(V *in, V *out)
{
	for (int row=0; row<8; row++) { 
		V *thisrow = &in[indexMapBuf(0,row*2)];
		V *nextrow = &in[indexMapBuf(0,row*2+1)];
		V *overrow = &in[indexMapBuf(0,(row+1)*2)];

		if (row>0) *out++ = thisrow[0];// jump end
		for (int col=0; col<8; col++) {
			*out++ = thisrow[col];
			*out++ = nextrow[col];
		}
		*out++ = thisrow[8];
		*out++ = overrow[8];
		*out++ = overrow[8];// jump start
		*out++ = thisrow[0];// jump end
		*out++ = thisrow[0];
		for (int col=0; col<8; col++) {
			*out++ = overrow[col];
			*out++ = nextrow[col];
		}
		if (row<8) *out++ = overrow[8];
		if (row<7) *out++ = overrow[8];// jump start
	}
}



#endif
