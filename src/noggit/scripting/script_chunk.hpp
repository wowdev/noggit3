// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_vert.hpp>
#include <noggit/scripting/script_object.hpp>
#include <noggit/MapChunk.h>

namespace noggit
{
  namespace scripting
  {
    class script_context;
    class selection;
    class chunk: public script_object
    {
    public:
      chunk(script_context * ctx, MapChunk* chunk);
      void remove_texture(int index);
      std::string get_texture(int index);
      int get_effect(int index);
      void set_effect(int index, int effectID);
      int add_texture(std::string const& texture, int effectID = -2);
      int get_texture_count();
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
      std::shared_ptr<selection> to_selection();
    private:
      MapChunk* _chunk;
    };

    class chunk_iterator : public script_object
    {
    public:
      chunk_iterator(script_context* ctx, std::shared_ptr<std::vector<MapChunk*>> chunks);
      bool next();
      chunk get();
      void reset();
    private:
      int _cur = -1;
      std::shared_ptr<std::vector<MapChunk*>> _chunks;
    };

    void register_chunk(script_context * state);
  } // namespace scripting
} // namespace noggit