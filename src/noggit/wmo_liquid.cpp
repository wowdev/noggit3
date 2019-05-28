// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/World.h>
#include <noggit/wmo_liquid.hpp>
#include <opengl/context.hpp>
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

wmo_liquid::wmo_liquid(wmo_liquid const& other)
  : pos(other.pos)
  , texRepeats(other.texRepeats)
  , mTransparency(other.mTransparency)
  , xtiles(other.xtiles)
  , ytiles(other.ytiles)
  , depths(other.depths)
  , tex_coords(other.tex_coords)
  , vertices(other.vertices)
  , indices(other.indices)
  , _uploaded(false)
{

}


int wmo_liquid::initGeometry(MPQFile* f)
{
  LiquidVertex const* map = reinterpret_cast<LiquidVertex const*>(f->getPointer());
  std::uint8_t const* flags = reinterpret_cast<std::uint8_t const*>(f->getPointer() + (xtiles + 1)*(ytiles + 1) * sizeof(LiquidVertex));
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
      std::uint8_t flag = flags[j*xtiles + i];

      if (!(flag & 8))
      {
        last_flag = flag;
        // 15 seems to be "don't draw"
        size_t p = j*(xtiles + 1) + i;

        depths.emplace_back (static_cast<float>(map[p].c[0]) / 255.0f);
        tex_coords.emplace_back (i + 0.f, j + 0.f);
        vertices.emplace_back (lVertices[p]);        

        depths.emplace_back (static_cast<float>(map[p + 1].c[0]) / 255.0f);
        tex_coords.emplace_back (i + 1.f, j + 0.f);
        vertices.emplace_back (lVertices[p + 1]);

        depths.emplace_back (static_cast<float>(map[p + xtiles + 1 + 1].c[0]) / 255.0f);
        tex_coords.emplace_back (i + 1.f, j + 1.f);
        vertices.emplace_back (lVertices[p + xtiles + 1 + 1]);

        depths.emplace_back (static_cast<float>(map[p + xtiles + 1].c[0]) / 255.0f);
        tex_coords.emplace_back (i + 0.f, j + 1.f);
        vertices.emplace_back (lVertices[p + xtiles + 1]);

        indices.emplace_back(index);
        indices.emplace_back(index+1);
        indices.emplace_back(index+2);

        indices.emplace_back(index+2);
        indices.emplace_back(index+3);
        indices.emplace_back(index);

        index += 4;
      }
    }
  }

  _indices_count = indices.size();

  return last_flag;
}

void wmo_liquid::upload(opengl::scoped::use_program& water_shader)
{
  _buffer.upload();
  _vertex_array.upload();

  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(_indices_buffer, indices, GL_STATIC_DRAW);

  gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>(_vertices_buffer, vertices, GL_STATIC_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER, math::vector_2d>(_tex_coord_buffer, tex_coords, GL_STATIC_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER, float>(_depth_buffer, depths, GL_STATIC_DRAW);

  opengl::scoped::index_buffer_manual_binder indices_binder (_indices_buffer);

  {
    opengl::scoped::vao_binder const _ (_vao);
    
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices_binder (_vertices_buffer);
    water_shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const tex_coord_binder(_tex_coord_buffer);
    water_shader.attrib("tex_coord", 2, GL_FLOAT, GL_FALSE, 0, 0);

    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const depth_binder(_depth_buffer);
    water_shader.attrib("depth", 1, GL_FLOAT, GL_FALSE, 0, 0);

    indices_binder.bind();
  }

  _uploaded = true;
}

void wmo_liquid::draw ( math::matrix_4x4 const& model_view
                      , math::matrix_4x4 const& projection
                      , math::matrix_4x4 const& transform
                      , math::vector_4d const& ocean_color_light
                      , math::vector_4d const& ocean_color_dark
                      , math::vector_4d const& river_color_light
                      , math::vector_4d const& river_color_dark
                      , liquid_render& render
                      , int animtime
                      )
{
  opengl::scoped::use_program water_shader(render.shader_program());

  if (!_uploaded)
  {
    upload(water_shader);
  }

  opengl::scoped::bool_setter<GL_CULL_FACE, FALSE> const cull;  

  water_shader.uniform ("model_view", model_view);
  water_shader.uniform ("projection", projection);
  water_shader.uniform ("transform", transform);

  water_shader.uniform ("use_transform", 1);

  water_shader.uniform ("ocean_color_light", ocean_color_light);
  water_shader.uniform ("ocean_color_dark", ocean_color_dark);
  water_shader.uniform ("river_color_light", river_color_light);
  water_shader.uniform ("river_color_dark", river_color_dark);

  water_shader.uniform("tex_repeat", texRepeats);

  opengl::scoped::vao_binder const _ (_vao);

  render.prepare_draw (water_shader, 13, animtime, true);

  gl.drawElements (GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr);
}
