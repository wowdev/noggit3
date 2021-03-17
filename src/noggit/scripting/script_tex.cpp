#include <noggit/scripting/script_tex.hpp>
#include <noggit/MapChunk.h>
#include <noggit/scripting/script_exception.hpp>

#include <sol/sol.hpp>

// amount of texunits per chunk length
#define TEXTURE_UNITS_WIDTH 64

namespace noggit {
  namespace scripting {
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
    
    void register_tex(sol::state * state, scripting_tool * tool)
    {
      state->new_usertype<tex>("tex"
        , "set_alpha", &tex::set_alpha
        , "get_alpha", &tex::get_alpha
        , "get_pos_2d", &tex::get_pos_2d
      );
    }
  }
}