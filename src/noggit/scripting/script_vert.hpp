// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_tex.hpp>

#include <noggit/MapChunk.h>
#include <math/vector_3d.hpp>

namespace sol {
  class state;
}

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;

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

    class vert_iterator {
      public:
        vert_iterator(
          std::shared_ptr<std::vector<MapChunk*>> chunks
          , math::vector_3d const& min
          , math::vector_3d const& max);
        bool next();
        vert get();
    private:
      std::vector<MapChunk*>::iterator _chunk_iter;
      int _vert_iter = -1;
      std::shared_ptr<std::vector<MapChunk*>> _chunks;
      math::vector_3d const& _min;
      math::vector_3d const& _max;
    };

    void register_vert(lua_state * state);
  } // namespace scripting
} // namespace noggit
