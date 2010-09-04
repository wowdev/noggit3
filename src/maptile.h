#ifndef MAPTILE_H
#define MAPTILE_H

#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)
#define UNITSIZE (CHUNKSIZE / 8.0f)
#define	MINICHUNKSIZE (CHUNKSIZE / 4.0f)
#define ZEROPOINT (32.0f * (TILESIZE))

#include <vector>
#include <string>

#include "liquid.h"
#include "mapheaders.h"
#include "quaternion.h"

class MapTile;
class MapChunk;

class World;
class brush;

enum MCNKFlags
{
  FLAG_SHADOW = 1,
  FLAG_IMPASS = 2,
  FLAG_LQ_RIVER = 4,
  FLAG_LQ_OCEAN = 8,
  FLAG_LQ_MAGMA = 0x10,
  FLAG_LQ_SLIME = 0x20,
  FLAG_MCCV = 0x40,
  FLAG_TBC = 0x8000
};

const int mapbufsize = 9*9 + 8*8;

class MapNode {
public:

	MapNode(int x, int y, int s):px(x),py(y),size(s) {}

	int px, py, size;

	Vec3D vmin, vmax, vcenter;

	MapNode *children[4];
	MapTile *mt;

	virtual void draw();
	virtual void drawSelect();
	virtual void drawColor();
	void setup(MapTile *t);
	void cleanup();

};

class MapChunk : public MapNode {
public:
	MapChunkHeader header;
	bool Changed;
	int nTextures;

	float xbase, ybase, zbase;
	float r;

	bool mBigAlpha;

	unsigned int nameID;

	unsigned int Flags;
	
	unsigned int areaID;

	bool haswater;

	bool hasholes;
	int holes;

	float waterlevel[2];

	int				tex[4];
	GLuint		textures[4];
	unsigned int	texFlags[4];
	unsigned int	effectID[4];
	unsigned int	MCALoffset[4];
	unsigned char amap[3][64*64];
	unsigned char mShadowMap[8*64];
	GLuint alphamaps[3];
	GLuint shadow;

	int animated[4];

	GLuint vertices, normals, minimap, minishadows;

	short *strip;
	int striplen;

	MapChunk( ) : MapNode( 0, 0, 0 ) { };
	~MapChunk( );

	void init(MapTile* mt, MPQFile &f,bool bigAlpha);
	void destroy();
	void initStrip();

	void draw();
	void drawContour();
	void drawColor();
	void drawSelect();
	void drawSelect2();
	void drawNoDetail();
	void drawPass(int anim);
	void drawLines();

	void drawTextures();

	void recalcNorms();

	Vec3D tn[mapbufsize], tv[mapbufsize], tm[mapbufsize];
	Vec4D ts[mapbufsize];

	void getSelectionCoord(float *x,float *z);
	float getSelectionHeight();

	Vec3D GetSelectionPosition( );

	void changeTerrain(float x, float z, float change, float radius, int BrushType);
	void flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);
	void blurTerrain(float x, float z, float remain, float radius, int BrushType);

	bool paintTexture(float x, float z, brush *Brush, float strength, float pressure, int texture);
	int addTexture(GLuint texture);
	void eraseTextures();

	bool isHole(int i,int j);
	void addHole(int i,int j);
	void removeHole(int i,int j);

	void setAreaID(int ID);

	bool GetVertex(float x,float z, Vec3D *V);

	void loadTextures();
//	char getAlpha(float x,float y);


	//float getTerrainHeight(float x, float z);
};

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

	std::string fname;
	
	MPQFile	*theFile;

	bool	chunksLoaded;
	int		nextChunk;
	size_t mcnk_offsets[256], mcnk_sizes[256];
	void	loadChunk();
	void	finishChunkLoad();

	void loadTexture();
	void finishTextureLoad();

	char	*modelBuffer;
	char	*modelPos;
	size_t	modelSize;
	int		curModelID;
	
	uint32_t	modelNum;
	ENTRY_MDDF	*modelInstances;
	
public:
	void loadModel();
	bool	modelsLoaded;
private:
	void loadModelInstances(int id);


	char	*wmoBuffer;
	char	*wmoPos;
	size_t	wmoSize;
	
	uint32_t	wmoNum;
	ENTRY_MODF	*wmoInstances;
	
public:
	void loadWMO();
	bool	wmosLoaded;
private:
	void loadWMOInstances();

public:
	void finishLoading();
	bool isLoaded(){return mTexturesLoaded&modelsLoaded&wmosLoaded;};
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
		else if(!wmosLoaded)
			loadWMO();
		else if(!modelsLoaded)
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
