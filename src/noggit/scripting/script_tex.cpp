// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_tex.hpp>
#include <noggit/MapChunk.h>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_math.hpp>
#include <noggit/scripting/script_image.hpp>

#include <sol/sol.hpp>

// amount of texunits per chunk length
#define TEXTURE_UNITS_WIDTH 64
#define TEXTURE_UNITS_PER_CHUNK 4096

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

    namespace {
      math::vector_3d tex_location(MapChunk* chnk, int index)
      {
        float cx = chnk->xbase;
        float cz = chnk->zbase;
        float x = index % TEXTURE_UNITS_WIDTH;
        float z = (float(index) / float(TEXTURE_UNITS_WIDTH));
        return math::vector_3d(cx + x * TEXDETAILSIZE, 0, cz + z * TEXDETAILSIZE);
      }
    }

    math::vector_3d tex::get_pos_2d()
    {
      return tex_location(_chunk, _index);
    }

    void collect_textures(
        script_context * ctx
      , MapChunk* chnk
      , std::vector<tex>& vec
      , math::vector_3d const& min
      , math::vector_3d const& max
    )
    {
      for (int i = 0; i < TEXTURE_UNITS_PER_CHUNK; ++i)
      {
        math::vector_3d loc = tex_location(chnk, i);
        if (loc.x >= min.x && loc.x <= max.x && loc.z >= min.z && loc.z <= max.z)
        {
          vec.emplace_back(ctx, chnk, i);
        }
      }
    }
    
    void register_tex(script_context * state)
    {
      state->new_usertype<tex>("tex"
        , "set_alpha", &tex::set_alpha
        , "get_alpha", &tex::get_alpha
        , "get_pos_2d", &tex::get_pos_2d
      );
    }
  }
}