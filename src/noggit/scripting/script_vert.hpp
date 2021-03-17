// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/MapChunk.h>
#include <math/vector_3d.hpp>

namespace noggit
{
  namespace scripting
  {
    struct selection;

    struct vert
    {
      vert(MapChunk* chunk, int index);
      vert() = default;
      MapChunk* _chunk;
      int _index;
      int _tex_index = -1;
    };

    struct tex
    {
      tex() = default;
      tex(MapChunk* chunk, int index);
      MapChunk* _chunk;
      int _index;
    };

    math::vector_3d vert_get_pos(vert const& vert);
    void vert_set_height(vert& vert, float y);
    void vert_add_height(vert& vert, float y);
    void vert_sub_height(vert& vert, float y);

    void vert_set_color(vert& vert, float r, float g, float b);
    void vert_set_water(vert& vert, int type, float height);
    void vert_set_hole(vert& vert, bool add);
    void vert_set_alpha(vert& vert, int index, float alpha);
    float vert_get_alpha(vert const& vert, int index);
    bool vert_next_tex(vert& vert);
    void vert_reset_tex(vert& vert);

    tex vert_get_tex(vert& vert);
    bool vert_is_water_aligned(vert const& chunk);

    void tex_set_alpha(tex& tex, int index, float alpha);
    float tex_get_alpha(tex const& tex, int index);
    math::vector_3d tex_get_pos_2d(tex const& tex);
  } // namespace scripting
} // namespace noggit
