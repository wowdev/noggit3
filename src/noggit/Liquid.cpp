// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Liquid.h>
#include <noggit/Log.h>
#include <opengl/call_list.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>



Liquid::Liquid(int x, int y, math::vector_3d base, MH2O_Tile const& tile_info)
  : xtiles(x)
  , ytiles(y)
  , pos(base)
  , texRepeats(4.0f)
  , _flags(tile_info.mFlags)
  , _minimum(tile_info.mMinimum)
  , _maximum(tile_info.mMaximum)
  , _liquid_id(tile_info.mLiquidType)
  , render(new liquid_render(true))
{
  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      _subchunks.emplace_back(tile_info.mRender[z][x]);
    }
  }

  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      _depth.emplace_back(tile_info.mDepth[z][x]);
      _vertices.emplace_back( pos.x + LQ_DEFAULT_TILESIZE * x
                            , tile_info.mDepth[z][x]
                            , pos.z + LQ_DEFAULT_TILESIZE * z
                            );
    }
  }
  
  changeLiquidID(_liquid_id);
  updateRender();
}


void Liquid::changeLiquidID(int id)
{
  _liquid_id = id;

  try
  {
    DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID(_liquid_id);
    render->setTextures(lLiquidTypeRow.getString(LiquidTypeDB::TextureFilenames - 1));
    mTransparency = lLiquidTypeRow.getInt(LiquidTypeDB::ShaderType) & 1;
    //! \todo  Get texRepeats too.
  }
  catch (...)
  {
    // Fallback, when there is no information.
    render->setTextures("XTextures\\river\\lake_a.%d.blp");
    mTransparency = true;
  }
}

void Liquid::updateRender()
{
  opengl::call_list* draw_list = new opengl::call_list();
  draw_list->start_recording();

  gl.begin(GL_QUADS);

  gl.normal3f(0.0f, 1.0f, 0.0f);

  // draw tiles
  for (int j = 0; j < 8; ++j)
  {
    for (int i = 0; i < 8; ++i)
    {
      
      if (!_subchunks[j * 8 + i])
      {
        continue;
      }

      int index = j * 9 + i;

      float c;
      c = _depth[index];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f(i / texRepeats, j / texRepeats);
      gl.vertex3fv(_vertices[index]);

      c = _depth[index+1];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f((i + 1) / texRepeats, j / texRepeats);
      gl.vertex3fv(_vertices[index + 1]);

      c = _depth[index + 10];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f((i + 1) / texRepeats, (j + 1) / texRepeats);
      gl.vertex3fv(_vertices[index + 10]);

      c = _depth[index + 9];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f(i / texRepeats, (j + 1) / texRepeats);
      gl.vertex3fv(_vertices[index + 9]);
    }
  }

  gl.end();

  draw_list->end_recording();

  render->changeDrawList(draw_list);
}
