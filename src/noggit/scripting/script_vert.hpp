// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/MapChunk.h>
#include <math/vector_3d.hpp>

struct TextureIndex;

namespace noggit
{
  namespace scripting
  {
    struct script_selection;

    struct script_vert
    {
      script_vert(MapChunk* chunk, int index);
      script_vert() {}
      MapChunk* _chunk;
      int _index;
      int _tex_index = -1;
    };

    struct script_tex
    {
      script_tex() {}
      script_tex(MapChunk* chunk, int index);
      MapChunk* _chunk;
      int _index;
    };

    math::vector_3d vert_get_pos(script_vert const& vert);
    void vert_set_height(script_vert& vert, float y);
    void vert_add_height(script_vert& vert, float y);
    void vert_sub_height(script_vert& vert, float y);

    void vert_set_color(script_vert& vert, float r, float g, float b);
    void vert_set_water(script_vert& vert, int type, float height);
    void vert_set_hole(script_vert& vert, bool add);
    void vert_set_alpha(script_vert& vert, int index, float alpha);
    float vert_get_alpha(script_vert const& vert, int index);
    bool vert_next_tex(script_vert& vert);
    void vert_reset_tex(script_vert& vert);

    script_tex vert_get_tex(script_vert& vert);
    bool vert_is_water_aligned(script_vert const& chunk);

    void tex_set_alpha(script_tex& tex, int index, float alpha);
    float tex_get_alpha(script_tex const& tex, int index);
    math::vector_3d tex_get_pos_2d(script_tex const& tex);
  } // namespace scripting
} // namespace noggit
