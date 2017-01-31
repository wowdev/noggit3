// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/liquid_render.hpp>
#include <noggit/MapHeaders.h>

class MapChunk;
class sExtendableArray;

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


  void save(sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const;

  void draw() { render->draw(); }
  void updateRender();
  void changeLiquidID(int id);
  
  void crop(MapChunk* chunk);
  void updateTransparency(MapChunk* chunk, float factor);


  float min() const { return _minimum; }
  float max() const { return _maximum; }
  int liquidID() const { return _liquid_id; }

  bool hasSubchunk(int x, int z) const;
  void setSubchunk(int x, int z, bool water);

private:
  int _liquid_id;
  int _flags;
  float _minimum;
  float _maximum;
  std::uint64_t _subchunks;
  std::vector<math::vector_3d> _vertices;
  std::vector<float> _depth;

private:
  bool mTransparency;  
  int xtiles, ytiles;
  math::vector_3d pos;
  float texRepeats;

  std::unique_ptr<liquid_render> render;
};
