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

    bool vert::is_tex_done()
    {
      return _tex_index >= MAX_TEXUNITS_PER_VERT 
        || texture_index[_index].indices[_tex_index] == -1;
    }

    void vert::reset_tex()
    {
      _tex_index = -1;
    }

    bool vert::next_tex()
    {
      ++_tex_index;
      return !is_tex_done();
    }

    tex vert::get_tex()
    {
      if(is_tex_done())
      {
        throw script_exception(
          "vert::get_tex",
          "accessing invalid texture unit: iterator is done");
      }
      return tex(state(), _chunk, texture_index[_index].indices[_tex_index]);
    }

    vert_iterator::vert_iterator(
      script_context * ctx
      , std::shared_ptr<std::vector<MapChunk*>> chunks
      , math::vector_3d const& min
      , math::vector_3d const& max)
      : script_object(ctx)
      , _chunks(chunks)
      , _chunk_iter(_chunks->begin())
      , _min(min)
      , _max(max)
      {}

    bool vert_iterator::next()
    {
      if(_chunk_iter==_chunks->end())
      {
        return false;
      }

      ++_vert_iter;

      // skip verts outside of the selection
      while(_vert_iter < 145)
      {
        auto& vert = (*_chunk_iter)->mVertices[_vert_iter];
        if (vert.x <= _min.x || vert.x >= _max.x ||
          vert.z <= _min.z || vert.z >= _max.z)
        {
          ++_vert_iter;
        }
        else
        {
          break;
        }
      }

      // restart if we used too many verts
      if(_vert_iter >= 145)
      {
        ++_chunk_iter;
        _vert_iter = -1;
        return next();
      }

      return true;
    }

    vert vert_iterator::get()
    {
      return vert(state(), *_chunk_iter,_vert_iter);
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
        , "next_tex", &vert::next_tex
        , "reset_tex", &vert::reset_tex
        , "get_tex", &vert::get_tex
        , "is_water_aligned", &vert::is_water_aligned
      );

      state->new_usertype<vert_iterator>("vert_iterator"
        , "next", &vert_iterator::next
        , "get", &vert_iterator::get
      );
    }
  } // namespace scripting
} // namespace noggit
