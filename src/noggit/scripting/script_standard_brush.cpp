// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_standard_brush.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/camera.hpp>
#include <noggit/Brush.h>

#include <math/vector_4d.hpp>

#include <sol/sol.hpp>

namespace noggit {
  namespace scripting {
    standard_brush::standard_brush(script_context * ctx)
    : script_object(ctx)
    {}

    void standard_brush::set_area_id(math::vector_3d const& pos
                                    , int id
                                    , bool adt
                                    )
    {
      world()->setAreaID(pos, id, adt);
    }

    void standard_brush::change_vertex_color(
          math::vector_3d const& pos
        , math::vector_3d const& color
        , float alpha
        , float change
        , float radius
        , bool editMode
        )
    {
      auto v = color;
      world()->changeShader(
        pos, math::vector_4d(v.x, v.y, v.z, alpha), change, radius, editMode);
    }

    math::vector_3d standard_brush::get_vertex_color(
      math::vector_3d const& pos)
    {
      return world()->pickShaderColor(pos);
    }

    void standard_brush::flatten_terrain(math::vector_3d const& pos
                                        , float remain
                                        , float radius
                                        , int brush_type
                                        , bool lower
                                        , bool raise
                                        , math::vector_3d const& origin
                                        , double angle
                                        , double orientation
                                        )
    {
      world()->flattenTerrain( pos
                             , remain
                             , radius
                             , brush_type
                             , flatten_mode(raise, lower)
                             , origin
                             , math::degrees(angle)
                             , math::degrees(orientation));
    }

    void standard_brush::blur_terrain(math::vector_3d const& pos
                                     , float remain
                                     , float radius
                                     , int brush_type
                                     , bool lower
                                     , bool raise
                                     )
    {
      world()->blurTerrain( pos
                          , remain
                          , radius
                          , brush_type
                          , flatten_mode(raise, lower));
    }

    void standard_brush::erase_textures(math::vector_3d const& pos)
    {
      world()->eraseTextures(pos);
    }

    void standard_brush::clear_shadows(math::vector_3d const& pos)
    {
      world()->clear_shadows(pos);
    }

    void standard_brush::clear_textures(math::vector_3d const& pos)
    {
      world()->clearTextures(pos);
    }

    void standard_brush::clear_height(math::vector_3d const& pos)
    {
      world()->clearHeight(pos);
    }

    void standard_brush::set_hole( math::vector_3d const& pos
                                 , bool big
                                 , bool hole
                                 )
    {
      world()->setHole(pos, big, hole);
    }

    void standard_brush::set_hole_adt( math::vector_3d const& pos
                                     , bool hole
                                     )
    {
      world()->setHoleADT(pos, hole);
    }

    void standard_brush::update_vertices ()
    {
      world()->updateVertexCenter();
      world()->updateSelectedVertices();
    }

    void standard_brush::deselect_vertices( math::vector_3d const& pos
                                          , float radius
                                          )
    {
      world()->deselectVertices(pos, radius);
    }

    void standard_brush::move_vertices(float h)
    {
      world()->moveVertices(h);
    }

    void standard_brush::flatten_vertices(float h)
    {
      world()->flattenVertices(h);
    }

    void standard_brush::clear_vertex_selection ()
    {
      world()->clearVertexSelection();
    }

    void standard_brush::paint_texture( math::vector_3d const& pos
                                      , float strength
                                      , float pressure
                                      , float hardness
                                      , float radius
                                      , std::string const& texture
                                      )
    {
      auto brush = Brush();
      brush.setHardness(hardness);
      brush.setRadius(radius);
      world()->paintTexture(pos
                           , &brush
                           , strength
                           , pressure
                           , scoped_blp_texture_reference(std::string(texture)));
    }

    void standard_brush::change_terrain( math::vector_3d const& pos
                                       , float change
                                       , float radius
                                       , float inner_radius
                                       , int brush_type
                                       )
    {
      world()->changeTerrain( pos
                            , change
                            , radius
                            , brush_type
                            , inner_radius
                            );
    }

    void register_standard_brush(script_context * state)
    {
      state->new_usertype<standard_brush>("standard_brush"
        , "change_terrain", &standard_brush::change_terrain
        , "set_area_id", &standard_brush::set_area_id
        , "change_vertex_color", &standard_brush::change_vertex_color
        , "get_vertex_color", &standard_brush::get_vertex_color
        , "flatten_terrain", &standard_brush::flatten_terrain
        , "blur_terrain", &standard_brush::blur_terrain
        , "erase_textures", &standard_brush::erase_textures
        , "clear_shadows", &standard_brush::clear_shadows
        , "clear_textures", &standard_brush::clear_textures
        , "clear_height", &standard_brush::clear_height
        , "set_hole", &standard_brush::set_hole
        , "set_hole_adt", &standard_brush::set_hole_adt
        , "deselect_vertices", &standard_brush::deselect_vertices
        , "clear_vertex_selection", &standard_brush::clear_vertex_selection
        , "move_vertices", &standard_brush::move_vertices
        , "flatten_vertices", &standard_brush::flatten_vertices
        , "update_vertices", &standard_brush::update_vertices
        , "paint_texture", &standard_brush::paint_texture
      );
    }
  }
}
