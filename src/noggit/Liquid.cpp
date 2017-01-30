// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Liquid.h>
#include <noggit/Log.h>
#include <opengl/call_list.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>



Liquid::Liquid(int x, int y, math::vector_3d base, float ptilesize)
  : xtiles(x)
  , ytiles(y)
  , pos(base)
  , tilesize(ptilesize)
  , ydir(1.0f)
{

}


void Liquid::initFromMH2O(MH2O_Tile &pTileInfo)
{
  texRepeats = 4.0f;
  ydir = 1.0f;
  std::string texture;

  try
  {
    DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID(pTileInfo.mLiquidType);
    texture = lLiquidTypeRow.getString(LiquidTypeDB::TextureFilenames - 1);
    mLiquidType = lLiquidTypeRow.getInt(LiquidTypeDB::Type);
    mShaderType = lLiquidTypeRow.getInt(LiquidTypeDB::ShaderType);
    //! \todo  Get texRepeats too.
  }
  catch (...)
  {
    // Fallback, when there is no information.
    texture = "XTextures\\river\\lake_a.%d.blp";
    mLiquidType = 0;
    mShaderType = 1;
  }

  mTransparency = mShaderType & 1;

  // generate vertices
  math::vector_3d lVertices[9][9];
  for (int j = 0; j < 9; ++j)
  {
    for (int i = 0; i < 9; ++i)
    {
      lVertices[j][i] = math::vector_3d(pos.x + tilesize * i, pTileInfo.mHeightmap[j][i], pos.z + ydir * tilesize * j);
    }
  }

  opengl::call_list* draw_list = new opengl::call_list();
  draw_list->start_recording();

  gl.begin(GL_QUADS);

  gl.normal3f(0.0f, 1.0f, 0.0f);

  // draw tiles
  for (int j = 0; j < 8; ++j)
  {
    for (int i = 0; i < 8; ++i)
    {
      if (!pTileInfo.mRender[j][i])
      {
        continue;
      }

      float c;
      c = pTileInfo.mDepth[j][i];// / 255.0f;
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f(i / texRepeats, j / texRepeats);
      gl.vertex3fv(lVertices[j][i]);

      c = pTileInfo.mDepth[j][i + 1];// / 255.0f;
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f((i + 1) / texRepeats, j / texRepeats);
      gl.vertex3fv(lVertices[j][i + 1]);

      c = pTileInfo.mDepth[j + 1][i + 1];// / 255.0f;
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f((i + 1) / texRepeats, (j + 1) / texRepeats);
      gl.vertex3fv(lVertices[j + 1][i + 1]);

      c = pTileInfo.mDepth[j + 1][i];// / 255.0f;
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f(i / texRepeats, (j + 1) / texRepeats);
      gl.vertex3fv(lVertices[j + 1][i]);
    }
  }

  gl.end();

  draw_list->end_recording();

  render.reset(new liquid_render(col, mTransparency, draw_list, texture));
}
