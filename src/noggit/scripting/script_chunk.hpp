// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_vert.hpp>
#include <noggit/MapChunk.h>

namespace noggit
{
  namespace scripting
  {
    class chunk
    {
    public:
      chunk(MapChunk* chunk);
      chunk() = default;

      void remove_texture(int index);
      std::string get_texture(int index);
      int add_texture(char const* texture);
      void clear_textures();
      void set_hole(bool hole);
      void clear_colors();
      void apply_textures();
      void apply_heightmap();
      void apply_vertex_color();
      void apply_all();
      void set_impassable(bool add);
      int get_area_id();
      void set_area_id(int value);
    private:
      MapChunk* _chunk;
    };

    void register_chunk(sol::state * state, scripting_tool * tool);
  } // namespace scripting
} // namespace noggit
