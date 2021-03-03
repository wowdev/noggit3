// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_vert.hpp>
#include <noggit/scripting/script_vert-script_texture_index.ipp>
#include <noggit/scripting/script_exception.hpp>

#include <vector>

// amount of texunits per chunk length
#define TEXTURE_UNITS_WIDTH 64
// maximum amount of texunits that can be closest to a vertex
#define MAX_TEXUNITS_PER_VERT 36
// total amount of vertices in each pair of two rows
#define VERTS_PER_TWO_ROWS 17
// maximum vertex index on odd rows
#define VERTS_ON_ODD_ROWS 8

namespace noggit
{
  namespace scripting
  {
    script_vert::script_vert(MapChunk* chunk, int index)
      : _chunk(chunk), _index(index)
    {
    }

    script_tex::script_tex(MapChunk* chunk, int index)
      : _chunk(chunk), _index(index)
    {
    }

    void vert_set_height(script_vert& vert, float value)
    {
      vert._chunk->mVertices[vert._index].y = value;
    }

    void vert_add_height(script_vert& vert, float value)
    {
      vert._chunk->mVertices[vert._index].y += value;
    }

    void vert_sub_height(script_vert& vert, float value)
    {
      vert._chunk->mVertices[vert._index].y -= value;
    }

    void vert_set_color(script_vert& vert, float r, float g, float b)
    {
      vert._chunk->maybe_create_mccv();
      vert._chunk->mccv[vert._index] = math::vector_3d(r, g, b);
    }

    void vert_set_water(script_vert& vert, int type, float height)
    {
      if (!vert_is_water_aligned(vert))
      {
        return;
      }

      // TODO: Extremely inefficient
      vert._chunk->liquid_chunk()->paintLiquid(vert_get_pos(vert), 1, type, true, math::radians(0), math::radians(0), true, math::vector_3d(0, height, 0), true, true, vert._chunk, 1);
    }

    void vert_set_hole(script_vert& vert, bool add)
    {
      vert._chunk->setHole(vert_get_pos(vert), false, add);
    }

    math::vector_3d vert_get_pos(script_vert const& vert)
    {
      return vert._chunk->mVertices[vert._index];
    }

    void vert_set_alpha(script_vert& vert, int index, float alpha)
    {
      if(index<0||index>3)
      {
        throw script_exception(
            std::string("invalid texture layer: ")
          + std::to_string(index)
          + std::string(" (in call to vert_set_alpha)")
          );
      }
      if (index == 0)
      {
        return;
      }
      auto& ts = vert._chunk->texture_set;
      ts->create_temporary_alphamaps_if_needed();

      auto tex_indices = texture_index[vert._index];

      for (auto iter = std::begin(tex_indices.indices); iter != std::end(tex_indices.indices); ++iter)
      {
        if (*iter == -1)
          break;
        ts->tmp_edit_values.get()[index][*iter] = alpha;
      }
    }

    float vert_get_alpha(script_vert const& vert, int index)
    {
      if(index<0||index>3)
      {
        throw script_exception(
            std::string("invalid texture layer: ")
          + std::to_string(index)
          + std::string(" (in call to vert_get_alpha)")
          );
      }
      if (index == 0)
      {
        return 1;
      }
      auto& ts = vert._chunk->texture_set;
      ts->create_temporary_alphamaps_if_needed();
      auto tex_indices = texture_index[vert._index];

      float sum = 0;
      int ctr = 0;
      for (auto iter = std::begin(tex_indices.indices); iter != std::end(tex_indices.indices); ++iter)
      {
        if (*iter == -1)
          break;
        sum += ts->tmp_edit_values.get()[index][*iter];
        ++ctr;
      }
      return sum / float(ctr);
    }

    bool vert_is_water_aligned(script_vert const& vert)
    {
      return (vert._index % VERTS_PER_TWO_ROWS) > VERTS_ON_ODD_ROWS;
    }

    static bool is_tex_done(script_vert& vert)
    {
      return vert._tex_index >= MAX_TEXUNITS_PER_VERT || texture_index[vert._index].indices[vert._tex_index] == -1;
    }

    void vert_reset_tex(script_vert& vert)
    {
      vert._tex_index = -1;
    }

    bool vert_next_tex(script_vert& vert)
    {
      ++vert._tex_index;
      return !is_tex_done(vert);
    }

    script_tex vert_get_tex(script_vert& vert)
    {
      if(is_tex_done(vert))
      {
        throw script_exception("accessing invalid texture unit: iterator is done");
      }
      return script_tex(vert._chunk, texture_index[vert._index].indices[vert._tex_index]);
    }

    float tex_get_alpha(script_tex const& tex, int index)
    {
      auto& ts = tex._chunk->texture_set;
      ts->create_temporary_alphamaps_if_needed();
      return ts->tmp_edit_values.get()[index][tex._index];
    }

    void tex_set_alpha(script_tex& tex, int index, float value)
    {
      if(index<0||index>3)
      {
        throw script_exception(
            std::string("invalid texture layer: ")
          + std::to_string(index)
          + std::string(" (in call to tex_set_alpha)")
          );
      }
      auto& ts = tex._chunk->texture_set;
      ts->create_temporary_alphamaps_if_needed();
      ts->tmp_edit_values.get()[index][tex._index] = value;
    }

    math::vector_3d tex_get_pos_2d(script_tex const& tex)
    {
      float cx = tex._chunk->xbase;
      float cz = tex._chunk->zbase;
      float x = tex._index % TEXTURE_UNITS_WIDTH;
      float z = (float(tex._index) / float(TEXTURE_UNITS_WIDTH));
      return math::vector_3d(cx + x * TEXDETAILSIZE, 0, cz + z * TEXDETAILSIZE);
    }
  } // namespace scripting
} // namespace noggit
