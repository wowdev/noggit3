// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/World.h>
#include <noggit/wmo_liquid.hpp>
#include <opengl/context.hpp>
#include <opengl/matrix.hpp>
#include <opengl/shader.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>



wmo_liquid::wmo_liquid(MPQFile* f, WMOLiquidHeader const& header, WMOMaterial const&, bool /*indoor*/)
  : pos(math::vector_3d(header.pos.x, header.pos.z, -header.pos.y))
  , texRepeats(4.0f)
  , xtiles(header.A)
  , ytiles(header.B)
{
  int flag = initGeometry (f);
  std::string texture;

  // value for the last drawn tile
  if (flag & 1)
  {
    // "XTEXTURES\\SLIME\\slime.%d.blp"
    texture = "XTextures\\river\\lake_a.%d.blp";
    texRepeats = 2.0f;
    mTransparency = false;
  }
  else if (flag & 2)
  {
    // "XTEXTURES\\LAVA\\lava.%d.blp"
    texture = "XTextures\\river\\lake_a.%d.blp";
    mTransparency = false;
  }
  else
  {
    // "XTEXTURES\\river\\lake_a.%d.blp"
    texture = "XTextures\\river\\lake_a.%d.blp";
    mTransparency = true;
  }
}

int wmo_liquid::initGeometry(MPQFile* f)
{
  LiquidVertex const* map = reinterpret_cast<LiquidVertex const*>(f->getPointer());
  unsigned char const* flags = reinterpret_cast<unsigned char const*>(f->getPointer() + (xtiles + 1)*(ytiles + 1) * sizeof(LiquidVertex));
  int last_flag = 0;

  // generate vertices
  std::vector<math::vector_3d> lVertices ((xtiles + 1)*(ytiles + 1));

  for (int j = 0; j<ytiles + 1; j++)
  {
    for (int i = 0; i<xtiles + 1; ++i)
    {
      size_t p = j*(xtiles + 1) + i;
      lVertices[p] = math::vector_3d( pos.x + UNITSIZE * i
                                    , map[p].h
                                    , pos.z + -1.0f * UNITSIZE * j
                                    );
    }
  }

  std::uint16_t index (0);

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

        depths.emplace_back (static_cast<float>(map[p].c[0]) / 255.0f);
        tex_coords.emplace_back (i + 0.f, j + 0.f);
        vertices.emplace_back (lVertices[p]);
        indices.emplace_back (index++);

        depths.emplace_back (static_cast<float>(map[p + 1].c[0]) / 255.0f);
        tex_coords.emplace_back (i + 1.f, j + 0.f);
        vertices.emplace_back (lVertices[p + 1]);
        indices.emplace_back (index++);

        depths.emplace_back (static_cast<float>(map[p + xtiles + 1 + 1].c[0]) / 255.0f);
        tex_coords.emplace_back (i + 1.f, j + 1.f);
        vertices.emplace_back (lVertices[p + xtiles + 1 + 1]);
        indices.emplace_back (index++);

        depths.emplace_back (static_cast<float>(map[p + xtiles + 1].c[0]) / 255.0f);
        tex_coords.emplace_back (i + 0.f, j + 1.f);
        vertices.emplace_back (lVertices[p + xtiles + 1]);
        indices.emplace_back (index++);
      }
    }
  }

  _indices_count = indices.size();

  opengl::scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (_index_buffer[0]);
  gl.bufferData (GL_ELEMENT_ARRAY_BUFFER
    , _indices_count * sizeof (indices[0])
    , indices.data()
    , GL_STATIC_DRAW
  );

  return last_flag;
}

void wmo_liquid::draw ( math::vector_4d const& ocean_color_light
          , math::vector_4d const& ocean_color_dark
          , math::vector_4d const& river_color_light
          , math::vector_4d const& river_color_dark
          , liquid_render& render
          , int animtime
          )
{
  opengl::scoped::bool_setter<GL_COLOR_MATERIAL, FALSE> const color_mat;
  opengl::scoped::bool_setter<GL_LIGHTING, FALSE> const lighting;
  opengl::scoped::bool_setter<GL_CULL_FACE, FALSE> const cull;

  opengl::scoped::use_program water_shader (render.shader_program());

  water_shader.uniform ("model_view", opengl::matrix::model_view());
  water_shader.uniform ("projection", opengl::matrix::projection());

  water_shader.uniform ("ocean_color_light", ocean_color_light);
  water_shader.uniform ("ocean_color_dark", ocean_color_dark);
  water_shader.uniform ("river_color_light", river_color_light);
  water_shader.uniform ("river_color_dark", river_color_dark);

  render.prepare_draw (water_shader, 13, animtime, true);

  opengl::scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (_index_buffer[0]);

  water_shader.attrib ("position", vertices);
  water_shader.attrib ("tex_coord", tex_coords);
  water_shader.attrib ("depth", depths);
  water_shader.uniform ("tex_repeat", texRepeats);


  gl.drawElements (GL_QUADS, indices.size(), GL_UNSIGNED_SHORT, nullptr);
}
