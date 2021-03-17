// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/MapChunk.h>
#include <math/vector_3d.hpp>

namespace sol {
  class state;
}

namespace noggit
{
  namespace scripting
  {
    struct selection;
    class scripting_tool;

    class tex
    {
    public:
      tex(MapChunk* chunk, int index);
      tex() = default;

      void set_alpha(int index, float alpha);
      float get_alpha(int index);
      math::vector_3d get_pos_2d();
    
    private:
      MapChunk* _chunk;
      int _index;
    };

    class vert
    {
    public:
      vert(MapChunk* chunk, int index);
      vert() = default;
      math::vector_3d get_pos();
      void set_height(float y);
      void add_height(float y);
      void sub_height(float y);

      void set_color(float r, float g, float b);
      void set_water(int type, float height);
      void set_hole(bool add);
      void set_alpha(int index, float alpha);
      float get_alpha(int index);
      bool next_tex();
      void reset_tex();

      tex get_tex();
      bool is_water_aligned();

    private:
      bool is_tex_done();
      MapChunk* _chunk;
      int _index;
      int _tex_index = -1;
    };

    void register_vert(sol::state * state, scripting_tool * tool);
  } // namespace scripting
} // namespace noggit
