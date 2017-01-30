// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/wmo_liquid.hpp>
#include <noggit/Log.h>
#include <noggit/World.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>



wmo_liquid::wmo_liquid(MPQFile* f, WMOLiquidHeader const& header, WMOMaterial const& mat, bool indoor)
  : xtiles(header.A)
  , ytiles(header.B)
  , pos(math::vector_3d(header.pos.x, header.pos.z, -header.pos.y))
  , tilesize(LQ_DEFAULT_TILESIZE)
  , ydir(-1.0f)
  , texRepeats(4.0f)
  , trans(false)
{
  opengl::call_list* draw_list = new opengl::call_list();
  int flag = initGeometry(f, draw_list);
  std::string texture;
    
  // value for the last drawn tile
  if (flag & 1)
  {
    // "XTEXTURES\\SLIME\\slime.%d.blp"
    texture = "XTextures\\river\\lake_a.%d.blp";
    type = 0;
    texRepeats = 2.0f;
    mTransparency = false;
  }
  else if (flag & 2) {
    // "XTEXTURES\\LAVA\\lava.%d.blp"
    texture = "XTextures\\river\\lake_a.%d.blp";
    type = 0;
    mTransparency = false;
  }
  else {
    // "XTEXTURES\\river\\lake_a.%d.blp"
    texture = "XTextures\\river\\lake_a.%d.blp";

    if (indoor) {
      trans = true;
      type = 1;
      col = math::vector_3d(((mat.col2 & 0xFF0000) >> 16) / 255.0f, ((mat.col2 & 0xFF00) >> 8) / 255.0f, (mat.col2 & 0xFF) / 255.0f);
    }
    else {
      trans = true;
      type = 2; // outdoor water (...?)
    }
    mTransparency = true;
  }

  render.reset(new liquid_render(col, mTransparency, draw_list, texture));
}

int wmo_liquid::initGeometry(MPQFile* f, opengl::call_list* draw_list)
{
  LiquidVertex *map = reinterpret_cast<LiquidVertex*>(f->getPointer());
  unsigned char *flags = reinterpret_cast<unsigned char*>(f->getPointer() + (xtiles + 1)*(ytiles + 1) * sizeof(LiquidVertex));
  int last_flag;

  // generate vertices
  math::vector_3d * lVertices = new math::vector_3d[(xtiles + 1)*(ytiles + 1)];

  for (int j = 0; j<ytiles + 1; j++)
  {
    for (int i = 0; i<xtiles + 1; ++i)
    {
      size_t p = j*(xtiles + 1) + i;
      float h = map[p].h;
      if (h > 100000)
      {
        h = pos.y;
      }
      lVertices[p] = math::vector_3d(pos.x + tilesize * i, h, pos.z + ydir * tilesize * j);
    }
  }

  draw_list->start_recording();

  //! \todo  handle light/dark liquid colors

  glBegin(GL_QUADS);
  glNormal3f(0, 1, 0);

  // draw tiles
  for (int j = 0; j<ytiles; j++)
  {
    for (int i = 0; i<xtiles; ++i)
    {
      unsigned char flag = flags[j*xtiles + i];
      if (!(flag & 8))
      {
        last_flag = flag;
        // 15 seems to be "don't draw"
        size_t p = j*(xtiles + 1) + i;
        float c;

        c = static_cast<float>(map[p].c[0]) / 255.0f;
        glMultiTexCoord2f(GL_TEXTURE1, c, c);
        gl.texCoord2f(i / texRepeats, j / texRepeats);
        gl.vertex3fv(lVertices[p]);

        c = static_cast<float>(map[p + 1].c[0]) / 255.0f;
        glMultiTexCoord2f(GL_TEXTURE1, c, c);
        gl.texCoord2f((i + 1) / texRepeats, j / texRepeats);
        gl.vertex3fv(lVertices[p + 1]);

        c = static_cast<float>(map[p + xtiles + 1 + 1].c[0]) / 255.0f;
        glMultiTexCoord2f(GL_TEXTURE1, c, c);
        gl.texCoord2f((i + 1) / texRepeats, (j + 1) / texRepeats);
        gl.vertex3fv(lVertices[p + xtiles + 1 + 1]);

        c = static_cast<float>(map[p + xtiles + 1].c[0]) / 255.0f;
        glMultiTexCoord2f(GL_TEXTURE1, c, c);
        gl.texCoord2f(i / texRepeats, (j + 1) / texRepeats);
        gl.vertex3fv(lVertices[p + xtiles + 1]);
      }
    }
  }
  glEnd();

  draw_list->end_recording();

  if (lVertices)
  {
    delete[] lVertices;
    lVertices = nullptr;
  }

  return last_flag;
}
