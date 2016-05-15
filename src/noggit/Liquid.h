// Liquid.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

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
#include <noggit/TextureManager.h>
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

struct LiquidVertex {
  unsigned char c[4];
  float h;
};

// handle liquids like oceans, lakes, rivers, slime, magma
class Liquid
{
public:
  Liquid (int x, int y, ::math::vector_3d base, float ptilesize = LQ_DEFAULT_TILESIZE);
  ~Liquid();

  void initFromWMO (noggit::mpq::file*, WMOMaterial const&, bool indoor);
  void initFromMH2O (MH2O_Tile const&);

  void draw (Skies const*) const;

private:
  int mShaderType;
  int mLiquidType;
  bool mTransparency;

  int xtiles, ytiles;
  std::unique_ptr<opengl::call_list> mDrawList;

  ::math::vector_3d pos;

  float tilesize;
  float ydir;
  float texRepeats;

  void initGeometry(noggit::mpq::file* f);

  template<int pFirst, int pLast>
  void initTextures( const std::string& pFilename );

  int type;
  std::vector<noggit::scoped_blp_texture_reference> _textures;
  ::math::vector_3d col;
  int tmpflag;
  bool trans;

  MH2O_Tile mTileData;
};

#endif
