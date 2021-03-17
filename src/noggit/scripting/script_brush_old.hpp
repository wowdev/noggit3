// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <math/vector_3d.hpp>

namespace noggit
{
  namespace scripting
  {
    void brush_change_terrain(math::vector_3d const&, float change, float radius, float inner_radius, int brush_type);
    void brush_set_area_id(math::vector_3d const&, int id, bool adt);
    void brush_change_vertex_color(math::vector_3d const& pos, math::vector_3d const& color, float alpha, float change, float radius, bool editMode);
    math::vector_3d brush_get_vertex_color(math::vector_3d const& pos);
    void brush_flatten_terrain(math::vector_3d const& pos, float remain, float radius, int brush_type, bool lower, bool raise, math::vector_3d const& origin, double angle, double orientation);
    void brush_blur_terrain(math::vector_3d const& pos, float remain, float radius, int brush_type, bool lower, bool raise);
    void brush_erase_textures(math::vector_3d const& pos);
    void brush_clear_shadows(math::vector_3d const& pos);
    void brush_clear_textures(math::vector_3d const& pos);
    void brush_clear_height(math::vector_3d const& pos);
    void brush_set_hole(math::vector_3d const& pos, bool big, bool hole);
    void brush_set_hole_adt(math::vector_3d const& pos, bool hole);
    void brush_deselect_vertices(math::vector_3d const& pos, float radius);
    void brush_clear_vertex_selection ();
    void brush_move_vertices(float h);
    void brush_flatten_vertices(float h);
    void brush_update_vertices ();
    void brush_paint_texture(math::vector_3d const& pos, float strength, float pressure, float hardness, float radius, char const* texture);
  } // namespace scripting
} // namespace noggit
