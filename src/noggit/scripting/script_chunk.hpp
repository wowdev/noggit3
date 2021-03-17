// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_vert.hpp>
#include <noggit/MapChunk.h>

namespace noggit
{
  namespace scripting
  {
    struct selection;

    struct chunk
    {
      chunk(selection* sel, MapChunk* chunk);
      chunk() = default;
      MapChunk* _chunk;
      selection* _sel;
      int _vert_index = -1;
      int _tex_index = -1;
    };

    void chunk_remove_texture(chunk& chunk, int index);
    std::string chunk_get_texture(chunk const& chunk, int index);
    int chunk_add_texture(chunk& chunk, char const* texture);
    void chunk_clear_textures(chunk& chunk);
    void chunk_set_hole(chunk& chunk, bool hole);
    void chunk_clear_colors(chunk& chunk);
    void chunk_apply_textures(chunk& chunk);
    void chunk_apply_heightmap(chunk& chunk, das::Context* context);
    void chunk_apply_vertex_color(chunk& chunk);
    void chunk_apply_all(chunk& chunk, das::Context* context);
    void chunk_set_impassable(chunk& chunk, bool add);
    int chunk_get_area_id(chunk const& chunk);
    void chunk_set_area_id(chunk& chunk, int value);
    bool chunk_next_vert(chunk& chunk);
    bool chunk_next_tex(chunk& chunk);
    void chunk_reset_vert_itr(chunk& chunk);
    void chunk_reset_tex_itr(chunk& chunk);
    vert chunk_get_vert(chunk const& chunk);
    tex chunk_get_tex(chunk const& chunk);
  } // namespace scripting
} // namespace noggit
