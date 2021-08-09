// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_vert.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>

#include <sol/sol.hpp>

#include <vector>

// maximum amount of texunits that can be closest to a vertex
#define MAX_TEXUNITS_PER_VERT 36
// total amount of vertices in each pair of two rows
#define VERTS_PER_TWO_ROWS 17
// maximum vertex index on odd rows
#define VERTS_ON_ODD_ROWS 8

#include <noggit/scripting/script_vert-script_texture_index.ipp>

namespace noggit
{
  namespace scripting
  {
    vert::vert(script_context * ctx, MapChunk* chunk, int index)
      : script_object(ctx)
      , _chunk(chunk)
      , _index(index)
    {
    }

    void vert::set_height(float value)
    {
      _chunk->mVertices[_index].y = value;
    }

    void vert::add_height(float value)
    {
      _chunk->mVertices[_index].y += value;
    }

    void vert::sub_height(float value)
    {
      _chunk->mVertices[_index].y -= value;
    }

    void vert::set_color(float r, float g, float b)
    {
      _chunk->maybe_create_mccv();
      _chunk->mccv[_index] = math::vector_3d(r, g, b);
    }

    math::vector_3d vert::get_color()
    {
      if (!_chunk->hasColors())
      {
        return math::vector_3d(1, 1, 1);
      }
      else
      {
        return math::vector_3d(_chunk->mccv[_index]);
      }
    }

    void vert::set_water(int type, float height)
    {
      if (is_water_aligned())
      {
        return;
      }

      // TODO: Extremely inefficient
      _chunk->liquid_chunk()->paintLiquid(get_pos(), 1, type, true, math::radians(0), math::radians(0), true, math::vector_3d(0, height, 0), true, true, _chunk, 1);
    }

    void vert::set_hole(bool add)
    {
      _chunk->setHole(get_pos(), false, add);
    }

    math::vector_3d vert::get_pos()
    {
      return _chunk->mVertices[_index];
    }

    void vert::set_alpha(int index, float alpha)
    {
      if(index<0||index>3)
      {
        throw script_exception(
            "vert::set_alpha",
            std::string("invalid texture layer: ")
          + std::to_string(index));
      }
      if (index == 0)
      {
        return;
      }
      auto& ts = _chunk->texture_set;
      ts->create_temporary_alphamaps_if_needed();

      auto tex_indices = texture_index[_index];

      for ( auto iter = std::begin(tex_indices.indices)
          ; iter != std::end(tex_indices.indices)
          ; ++iter
          )
      {
        if (*iter == -1)
          break;
        ts->tmp_edit_values.get()[index][*iter] = alpha;
      }
    }

    float vert::get_alpha(int index)
    {
      if(index<0||index>3)
      {
        throw script_exception(
          "vert::get_alpha",
          std::string("invalid texture layer: ")
          + std::to_string(index));
      }
      if (index == 0)
      {
        return 1;
      }
      auto& ts = _chunk->texture_set;
      ts->create_temporary_alphamaps_if_needed();
      auto tex_indices = texture_index[_index];

      float sum = 0;
      int ctr = 0;
      for (auto iter = std::begin(tex_indices.indices)
          ; iter != std::end(tex_indices.indices)
          ; ++iter
          )
      {
        if (*iter == -1)
          break;
        sum += ts->tmp_edit_values.get()[index][*iter];
        ++ctr;
      }
      return sum / float(ctr);
    }

    bool vert::is_water_aligned()
    {
      return (_index % VERTS_PER_TWO_ROWS) > VERTS_ON_ODD_ROWS;
    }

    sol::as_table_t<std::vector<tex>> vert::textures()
    {
      std::vector<tex> texVec;
      for (int i = 0; i < MAX_TEXUNITS_PER_VERT; ++i)
      {
        if (texture_index[_index].indices[i] == -1) break;
        texVec.emplace_back(state(), _chunk, texture_index[_index].indices[i]);
      }
      return sol::as_table(texVec);
    }

    void register_vert(script_context * state)
    {
      state->new_usertype<vert>("vert"
        , "get_pos", &vert::get_pos
        , "set_height", &vert::set_height
        , "add_height", &vert::add_height
        , "sub_height", &vert::sub_height
        , "set_color", &vert::set_color
        , "get_color", &vert::get_color
        , "set_water", &vert::set_water
        , "set_hole", &vert::set_hole
        , "set_alpha", &vert::set_alpha
        , "get_alpha", &vert::get_alpha
        , "tex", &vert::textures
        , "is_water_aligned", &vert::is_water_aligned
      );
    }
  } // namespace scripting
} // namespace noggit
