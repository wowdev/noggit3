// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_vert.hpp>
#include <noggit/scripting/script_object.hpp>
#include <noggit/MapChunk.h>
#include <cstdint>

namespace noggit
{
  namespace scripting
  {
    class script_context;
    class selection;
    class tex;
    class vert;
    class chunk: public script_object
    {
    public:
      chunk(script_context * ctx, MapChunk* chunk);
      void remove_texture(int index);
      std::string get_texture(int index);
      int get_effect(int index);
      void set_effect(int index, int effectID);
      int add_texture(std::string const& texture, int effectID /* = -2*/);
      int add_texture_1(std::string const& texture)
      { return add_texture(texture,-2);}
      int get_texture_count();
      void clear_textures();
      void set_hole(bool hole);
      void clear_colors();
      void apply_textures();
      void apply_heightmap();
      void apply_vertex_color();
      void apply_all();

      void set_deep_flag(std::uint32_t low, std::uint32_t high);
      void set_deep_flag_1(std::uint32_t low);
      std::uint32_t get_deep_flag();
      std::uint32_t get_deep_flag_high();

      void set_fishable_flag(std::uint32_t low, std::uint32_t high);
      void set_fishable_flag_1(std::uint32_t low);
      std::uint32_t get_fishable_flag();
      std::uint32_t get_fishable_flag_high();
      bool has_render_flags();

      void set_impassable(bool add);
      int get_area_id();
      void set_area_id(int value);
      tex get_tex(int index);
      vert get_vert(int index);
      std::shared_ptr<selection> to_selection();
    private:
      MH2O_Render getRenderOrDefault();
      MH2O_Render& getOrCreateRender();
      MapChunk* _chunk;
      friend class selection;
    };

    void register_chunk(script_context * state);
  } // namespace scripting
} // namespace noggit