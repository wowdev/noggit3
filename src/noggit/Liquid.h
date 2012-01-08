// #define USEBLSFILES

#ifndef LIQUID_H
#define LIQUID_H

class Liquid;

#include <string>
#include <vector>

#ifdef USEBLSFILES
  #include <noggit/Shaders.h>
#endif
#include <noggit/MapTile.h>
#include <noggit/WMO.h>

class Skies;

namespace opengl
{
  class call_list;
  class texture;
};

namespace noggit
{
  namespace mpq
  {
    class file;
  }
}

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
  opengl::call_list* mDrawList;

  ::math::vector_3d pos;

  float tilesize;
  float ydir;
  float texRepeats;

  void initGeometry(noggit::mpq::file* f);

  template<int pFirst, int pLast>
  void initTextures( const std::string& pFilename );

  int type;
  std::vector<opengl::texture*> _textures;
  std::vector<std::string> _textureFilenames;
  ::math::vector_3d col;
  int tmpflag;
  bool trans;

  unsigned char  *waterFlags;

  //int number;

private:
  MH2O_Tile mTileData;

public:


  Liquid (int x, int y, ::math::vector_3d base, float ptilesize = LQ_DEFAULT_TILESIZE)
    : xtiles (x)
    , ytiles (y)
    , pos (base)
    , tilesize (ptilesize)
  {
  //  number = ++lCount;
  //  LogDebug << "Created Liquid of Number: " << number << std::endl;
    ydir = 1.0f;
    mDrawList = NULL;
  }
  ~Liquid();


  //void init(noggit::mpq::file &f);
  void initFromTerrain(noggit::mpq::file* f, int flags);
  void initFromWMO(noggit::mpq::file* f, const WMOMaterial &mat, bool indoor);
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

  void draw (const float&, const Skies* = NULL) const;

private:
  void recalcSize();
};

#endif
