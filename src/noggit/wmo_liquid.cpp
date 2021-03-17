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

namespace
{

  enum liquid_basic_types
  {
    liquid_basic_types_water = 0,
    liquid_basic_types_ocean = 1,
    liquid_basic_types_magma = 2,
    liquid_basic_types_slime = 3,

    liquid_basic_types_MASK = 3,
  };
  enum liquid_types
  {
    LIQUID_WMO_Water = 13,
    LIQUID_WMO_Ocean = 14,
    LIQUID_Green_Lava = 15,
    LIQUID_WMO_Magma = 19,
    LIQUID_WMO_Slime = 20,

    LIQUID_END_BASIC_LIQUIDS = 20,
    LIQUID_FIRST_NONBASIC_LIQUID_TYPE = 21,

    LIQUID_NAXX_SLIME = 21,
  };

  liquid_types to_wmo_liquid(int x, bool ocean)
  {
    liquid_basic_types const basic(static_cast<liquid_basic_types>(x & liquid_basic_types_MASK));
    switch (basic)
    {
      default:
      case liquid_basic_types_water:
        return ocean ? LIQUID_WMO_Ocean : LIQUID_WMO_Water;
      case liquid_basic_types_ocean:
        return LIQUID_WMO_Ocean;
      case liquid_basic_types_magma:
        return LIQUID_WMO_Magma;
      case liquid_basic_types_slime:
        return LIQUID_WMO_Slime;
    }
  }
}

// todo: use material
wmo_liquid::wmo_liquid(MPQFile* f, WMOLiquidHeader const& header, WMOMaterial const&, int group_liquid, bool use_dbc_type, bool is_ocean)
  : pos(math::vector_3d(header.pos.x, header.pos.z, -header.pos.y))
  , xtiles(header.A)
  , ytiles(header.B)
{
  int liquid = initGeometry(f);

  // see: https://wowdev.wiki/WMO#how_to_determine_LiquidTypeRec_to_use
  if (use_dbc_type)
  {
    if (group_liquid < LIQUID_FIRST_NONBASIC_LIQUID_TYPE)
    {
      _liquid_id = to_wmo_liquid(group_liquid - 1, is_ocean);
    }
    else
    {
      _liquid_id = group_liquid;
    }
  }
  else
  {
    if (group_liquid == LIQUID_Green_Lava)
    {
      // todo: investigage
      // This method is most likely wrong since "liquid" is the last SMOLTile's liquid value
      // and it can vary from one liquid tile to another.
      _liquid_id = to_wmo_liquid(liquid, is_ocean);
    }
    else
    {
      if (group_liquid < LIQUID_END_BASIC_LIQUIDS)
      {
        _liquid_id = to_wmo_liquid(group_liquid, is_ocean);
      }
      else
      {
        _liquid_id = group_liquid + 1;
      }
    }
  }
}

wmo_liquid::wmo_liquid(wmo_liquid const& other)
  : pos(other.pos)
  , mTransparency(other.mTransparency)
  , xtiles(other.xtiles)
  , ytiles(other.ytiles)
  , _liquid_id(other._liquid_id)
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
  SMOLTile const* tiles = reinterpret_cast<SMOLTile const*>(f->getPointer() + (xtiles + 1)*(ytiles + 1) * sizeof(LiquidVertex));
  int last_liquid_id = 0;

  // generate vertices
  std::vector<math::vector_3d> lVertices ((xtiles + 1)*(ytiles + 1));

  for (int j = 0; j<ytiles + 1; j++)
  {
    for (int i = 0; i<xtiles + 1; ++i)
    {
      size_t p = j*(xtiles + 1) + i;
      lVertices[p] = math::vector_3d( pos.x + UNITSIZE * i
                                    , map[p].height
                                    , pos.z - UNITSIZE * j
                                    );
    }
  }

  std::uint16_t index (0);

  for (int j = 0; j<ytiles; j++)
  {
    for (int i = 0; i<xtiles; ++i)
    {
      SMOLTile const& tile = tiles[j*xtiles + i];

      // it seems that if (liquid & 8) != 0 => do not render
      if (!(tile.liquid & 0x8))
      {
        last_liquid_id = tile.liquid;

        size_t p = j*(xtiles + 1) + i;

        if (!(tile.liquid & 2))
        {
          depths.emplace_back(static_cast<float>(map[p].water_vertex.flow1) / 255.0f);
          tex_coords.emplace_back(i, j);

          depths.emplace_back(static_cast<float>(map[p + 1].water_vertex.flow1) / 255.0f);
          tex_coords.emplace_back(i+1, j);

          depths.emplace_back(static_cast<float>(map[p + xtiles + 1 + 1].water_vertex.flow1) / 255.0f);
          tex_coords.emplace_back(i+1, j+1);

          depths.emplace_back(static_cast<float>(map[p + xtiles + 1].water_vertex.flow1) / 255.0f);
          tex_coords.emplace_back(i, j+1);
        }
        else
        {
          // todo handle that properly
          // depth isn't used for lava, it's just to fill up the buffer
          depths.emplace_back(1.f);
          depths.emplace_back(1.f);
          depths.emplace_back(1.f);
          depths.emplace_back(1.f);

          tex_coords.emplace_back( static_cast<float>(map[p].magma_vertex.s) / 255.f
                                 , static_cast<float>(map[p].magma_vertex.t) / 255.f
                                 );
          tex_coords.emplace_back( static_cast<float>(map[p + 1].magma_vertex.s) / 255.f 
                                 , static_cast<float>(map[p + 1].magma_vertex.t) / 255.f
                                 );
          tex_coords.emplace_back( static_cast<float>(map[p + xtiles + 1 + 1].magma_vertex.s) / 255.f 
                                 , static_cast<float>(map[p + xtiles + 1 + 1].magma_vertex.t) / 255.f 
                                 );
          tex_coords.emplace_back( static_cast<float>(map[p + xtiles + 1].magma_vertex.s) / 255.f
                                 , static_cast<float>(map[p + xtiles + 1].magma_vertex.t) / 255.f 
                                 );
        }

        vertices.emplace_back (lVertices[p]);
        vertices.emplace_back (lVertices[p + 1]);
        vertices.emplace_back (lVertices[p + xtiles + 1 + 1]);
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

  return last_liquid_id;
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
    
    water_shader.attrib(_, "position", _vertices_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    water_shader.attrib(_, "tex_coord", _tex_coord_buffer, 2, GL_FLOAT, GL_FALSE, 0, 0);
    water_shader.attrib(_, "depth", _depth_buffer, 1, GL_FLOAT, GL_FALSE, 0, 0);

    indices_binder.bind();
  }

  _uploaded = true;
}

void wmo_liquid::draw ( math::matrix_4x4 const& transform
                      , liquid_render& render
                      , int animtime
                      )
{
  opengl::scoped::use_program water_shader(render.shader_program());

  if (!_uploaded)
  {
    upload(water_shader);
  }

  opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;

  water_shader.uniform ("transform", transform);

  opengl::scoped::vao_binder const _ (_vao);

  render.force_texture_update();
  render.prepare_draw (water_shader, _liquid_id, animtime);

  gl.drawElements (GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, opengl::index_buffer_is_already_bound{});
}
