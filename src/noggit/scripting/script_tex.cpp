// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_tex.hpp>
#include <noggit/MapChunk.h>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_context.hpp>

#include <sol/sol.hpp>

// amount of texunits per chunk length
#define TEXTURE_UNITS_WIDTH 64

namespace noggit {
  namespace scripting {

    tex::tex(script_context * ctx, MapChunk* chunk, int index)
      : script_object(ctx)
      , _chunk(chunk)
      , _index(index)
    {}

    float tex::get_alpha(int index)
    {
      auto& ts = _chunk->texture_set;
      ts->create_temporary_alphamaps_if_needed();
      return ts->tmp_edit_values.get()[index][_index];
    }

    void tex::set_alpha(int index, float value)
    {
      if(index<0||index>3)
      {
        throw script_exception(
            "tex_set_alpha",
            std::string("invalid texture layer: ")
          + std::to_string(index)
          + std::string(" (in call to tex_set_alpha)")
          );
      }
      auto& ts = _chunk->texture_set;
      ts->create_temporary_alphamaps_if_needed();
      ts->tmp_edit_values.get()[index][_index] = value;
    }

    math::vector_3d tex::get_pos_2d()
    {
      float cx = _chunk->xbase;
      float cz = _chunk->zbase;
      float x = _index % TEXTURE_UNITS_WIDTH;
      float z = (float(_index) / float(TEXTURE_UNITS_WIDTH));
      return math::vector_3d(cx + x * TEXDETAILSIZE, 0, cz + z * TEXDETAILSIZE);
    }

    tex_iterator::tex_iterator(
        script_context * ctx
      , std::shared_ptr<std::vector<MapChunk*>> chunks
      , math::vector_3d const&
      , math::vector_3d const& )
      : script_object(ctx)
      , _chunks(chunks)
      , _chunk_iter(_chunks->begin())
      //, _min(min)
      //, _max(max)
      {}

    bool tex_iterator::next()
    {
      if(_chunk_iter==_chunks->end())
      {
        return false;
      }

      ++_tex_iter;
      while(_tex_iter < 4096)
      {
        // TODO: Implement skipping
        break;
      }

      if(_tex_iter >= 4096)
      {
        ++_chunk_iter;
        _tex_iter = -1;
        return next();
      }

      return true;
    }

    tex tex_iterator::get()
    {
      return tex(state(), *_chunk_iter,_tex_iter);
    }
    
    void register_tex(script_context * state)
    {
      state->new_usertype<tex>("tex"
        , "set_alpha", &tex::set_alpha
        , "get_alpha", &tex::get_alpha
        , "get_pos_2d", &tex::get_pos_2d
      );

      state->new_usertype<tex_iterator>("tex_iterator"
        , "next", &tex_iterator::next
        , "get", &tex_iterator::get
      );
    }
  }
}