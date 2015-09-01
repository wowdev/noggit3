// #define USEBLSFILES

#ifndef LIQUID_H
#define LIQUID_H

class Liquid;

#include <string>
#include <vector>

#include "Video.h"
#include "MPQ.h"
#ifdef USEBLSFILES
#include "Shaders.h"
#endif
#include "MapTile.h"
#include "WMO.h"

//#include "Log.h"

namespace OpenGL
{
	class CallList;
	class Texture;
};

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
		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < 8; j++)
				mRender[i][j] = false;

		for (int i = 0; i < 9; ++i)
			for (int j = 0; j < 9; j++)
			{
			mHeightmap[i][j] = 0.0f;
			mDepth[i][j] = 255.0f;
			}
	}
};

struct LiquidVertex {
	unsigned char c[4];
	float h;
};

// handle liquids like oceans, lakes, rivers, slime, magma
class Liquid
{
public:

	Liquid(int x, int y, Vec3D base, float ptilesize = LQ_DEFAULT_TILESIZE);
	~Liquid();

	void initFromWMO(MPQFile* f, const WMOMaterial &mat, bool indoor);
	void initFromMH2O(MH2O_Tile &pTileInfo);

	void draw();

private:
	void delTextures();

	int mShaderType;
	int mLiquidType;
	bool mTransparency;

	int xtiles, ytiles;
	OpenGL::CallList* mDrawList;

	Vec3D pos;

	float tilesize;
	float ydir;
	float texRepeats;

	void initGeometry(MPQFile* f);

	template<int pFirst, int pLast>
	void initTextures(const std::string& pFilename);

	int type;
	std::vector<OpenGL::Texture*> textures;
	Vec3D col;
	int tmpflag;
	bool trans;

	//int number;
	MH2O_Tile mTileData;

};

#endif
