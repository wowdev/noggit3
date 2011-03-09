// #define USEBLSFILES

#ifndef LIQUID_H
#define LIQUID_H

class Liquid;

#include "video.h"
#include "mpq.h"
#ifdef USEBLSFILES
	#include "shaders.h"
#endif
#include "maptile.h"
#include "wmo.h"

//#include "Log.h"

class OpenGL::CallList;

//static int lCount = 0;

void loadWaterShader();

const float LQ_DEFAULT_TILESIZE = CHUNKSIZE / 8.0f;

struct MH2O_Tile
{
	int mLiquidType;
	int mFlags;
	float mMinimum;
	float mMaximum;
	bool mRender[8][8];
	float mHeightmap[9][9];
	float mDepth[9][9];

	MH2O_Tile()
	{
		mLiquidType = 0;
		mFlags = 0;
		mMinimum = 0.0f;
		mMaximum = 0.0f;
		for( int i = 0; i < 8; ++i )
			for( int j = 0; j < 8; j++ )
				mRender[i][j] = false;
		
		for( int i = 0; i < 9; ++i )
			for( int j = 0; j < 9; j++ )
			{
				mHeightmap[i][j] = 0.0f;
				mDepth[i][j] = 255.0f;
			}
	}
};

// handle liquids like oceans, lakes, rivers, slime, magma
class Liquid 
{
	int pType;

	int mShaderType;
	int mLiquidType;
	bool mTransparency;

	int xtiles, ytiles;
	OpenGL::CallList* mDrawList;

	Vec3D pos;

	float tilesize;
	float ydir;
	float texRepeats;

	void initGeometry(MPQFile &f);

	template<int pFirst, int pLast>
	void initTextures( const std::string& pFilename );

	int type;
	std::vector<GLuint> textures;
	Vec3D col;
	int tmpflag;
	bool trans;
	
	unsigned char	*color;
	unsigned char	*waterFlags;

	//int number;

private:
	MH2O_Tile mTileData;

public:


	Liquid(int x, int y, Vec3D base, float ptilesize = LQ_DEFAULT_TILESIZE):
		xtiles(x), ytiles(y), pos(base), tilesize(ptilesize)
	{
	//	number = ++lCount;
	//	LogDebug << "Created Liquid of Number: " << number << std::endl;
		ydir = 1.0f;
		mDrawList = NULL;
	}
	~Liquid();
	

	//void init(MPQFile &f);
	void initFromTerrain(MPQFile &f, int flags);
	void initFromWMO(MPQFile &f, WMOMaterial &mat, bool indoor);
	void initFromMH2O( MH2O_Information *info, MH2O_HeightMask *HeightMap, MH2O_Render *render );
	void initFromMH2O();

	MH2O_Tile getMH2OData();
	void setMH2OData(MH2O_Tile pTileInfo);

	int getWidth();
	int getHeight();
	int getXOffset();
	int getYOffset();

	bool isNotEmpty();

	bool isRendered(int i, int j);
	void setRender(int i, int j);
	void unsetRender(int i, int j);

	void draw();

private:
	void recalcSize();


};

#endif
