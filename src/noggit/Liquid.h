// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/liquid_render.hpp>
#include <noggit/MapHeaders.h>


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
    {
      for (int j = 0; j < 8; j++)
      {
        mRender[i][j] = false;
      }        
    }

    for (int i = 0; i < 9; ++i)
    {
      for (int j = 0; j < 9; j++)
      {
        mHeightmap[i][j] = 0.0f;
        mDepth[i][j] = 255.0f;
      }
    }      
  }
};



// handle liquids like oceans, lakes, rivers, slime, magma
class Liquid
{
public:

  Liquid(math::vector_3d const& base, MH2O_Tile const& tile_info);

  void draw() { render->draw(); }

  void changeLiquidID(int id);
  void updateRender();

  float min() const { return _minimum; }

private:
  int _liquid_id;
  int _flags;
  float _minimum;
  float _maximum;
  std::vector<bool> _subchunks;
  std::vector<math::vector_3d> _vertices;
  std::vector<float> _depth;

private:
  bool mTransparency;  
  int xtiles, ytiles;
  math::vector_3d pos;
  float texRepeats;

  std::unique_ptr<liquid_render> render;
};
