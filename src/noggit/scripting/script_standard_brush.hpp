// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_object.hpp>

#include <math/vector_3d.hpp>

class World;

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class script_context;
    class standard_brush : public script_object {
    public:
      standard_brush(script_context * ctx);
      void change_terrain(math::vector_3d const&
                         , float change
                         , float radius
                         , float inner_radius
                         , int brush_type
                         );

      void set_area_id(math::vector_3d const&, int id, bool adt);

      void change_vertex_color(math::vector_3d const& pos
                              , math::vector_3d const& color
                              , float alpha
                              , float change
                              , float radius
                              , bool editMode
                              );

      math::vector_3d get_vertex_color(math::vector_3d const& pos);

      void flatten_terrain(math::vector_3d const& pos
                          , float remain
                          , float radius
                          , int brush_type
                          , bool lower
                          , bool raise
                          , math::vector_3d const& origin
                          , double angle
                          , double orientation
                          );

      void blur_terrain(math::vector_3d const& pos
                       , float remain
                       , float radius
                       , int brush_type
                       , bool lower
                       , bool raise
                       );

      void erase_textures(math::vector_3d const& pos);
      void clear_shadows(math::vector_3d const& pos);
      void clear_textures(math::vector_3d const& pos);
      void clear_height(math::vector_3d const& pos);
      void set_hole(math::vector_3d const& pos, bool big, bool hole);
      void set_hole_adt(math::vector_3d const& pos, bool hole);
      void deselect_vertices(math::vector_3d const& pos, float radius);
      void clear_vertex_selection ();
      void move_vertices(float h);
      void flatten_vertices(float h);
      void update_vertices ();
      void paint_texture(math::vector_3d const& pos
                        , float strength
                        , float pressure
                        , float hardness
                        , float radius
                        , std::string const& texture
                        );
    };

    void register_standard_brush(script_context * state);
  } // namespace scripting
} // namespace noggit
