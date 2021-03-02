// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_brush.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/camera.hpp>
#include <noggit/Brush.h>
#include <math/vector_4d.hpp>

namespace noggit {
  namespace scripting {
    void brush_set_area_id(math::vector_3d const& pos, int id, bool adt)
    {
      get_ctx()->_world->setAreaID(pos, id, adt);
    }

    void brush_change_vertex_color(math::vector_3d const& pos, math::vector_3d const& color, float alpha, float change, float radius, bool editMode)
    {
      auto v = color;
      get_ctx()->_world->changeShader(pos, math::vector_4d(v.x, v.y, v.z, alpha), change, radius, editMode);
    }

    math::vector_3d brush_get_vertex_color(math::vector_3d const& pos)
    {
      return get_ctx()->_world->pickShaderColor(pos);
    }

    void brush_flatten_terrain(math::vector_3d const& pos, float remain, float radius, int brush_type, bool lower, bool raise, math::vector_3d const& origin, double angle, double orientation)
    {
      get_ctx()->_world->flattenTerrain(pos, remain, radius, brush_type, flatten_mode(raise, lower), origin, math::degrees(angle), math::degrees(orientation));
    }

    void brush_blur_terrain(math::vector_3d const& pos, float remain, float radius, int brush_type, bool lower, bool raise)
    {
      get_ctx()->_world->blurTerrain(pos, remain, radius, brush_type, flatten_mode(raise, lower));
    }

    void brush_erase_textures(math::vector_3d const& pos)
    {
      get_ctx()->_world->eraseTextures(pos);
    }

    void brush_clear_shadows(math::vector_3d const& pos)
    {
      get_ctx()->_world->clear_shadows(pos);
    }

    void brush_clear_textures(math::vector_3d const& pos)
    {
      get_ctx()->_world->clearTextures(pos);
    }

    void brush_clear_height(math::vector_3d const& pos)
    {
      get_ctx()->_world->clearHeight(pos);
    }

    void brush_set_hole(math::vector_3d const& pos, bool big, bool hole)
    {
      get_ctx()->_world->setHole(pos, big, hole);
    }

    void brush_set_hole_adt(math::vector_3d const& pos, bool hole)
    {
      get_ctx()->_world->setHoleADT(pos, hole);
    }

    void brush_update_vertices()
    {
      get_ctx()->_world->updateVertexCenter();
      get_ctx()->_world->updateSelectedVertices();
    }

    void brush_deselect_vertices(math::vector_3d const& pos, float radius)
    {
      get_ctx()->_world->deselectVertices(pos, radius);
    }

    void brush_move_vertices(float h)
    {
      get_ctx()->_world->moveVertices(h);
    }

    void brush_flatten_vertices(float h)
    {
      get_ctx()->_world->flattenVertices(h);
    }

    void brush_clear_vertex_selection()
    {
      get_ctx()->_world->clearVertexSelection();
    }

    void brush_paint_texture(math::vector_3d const& pos, float strength, float pressure, float hardness, float radius, char const* texture)
    {
      auto brush = Brush();
      brush.setHardness(hardness);
      brush.setRadius(radius);
      get_ctx()->_world->paintTexture(pos, &brush, strength, pressure, scoped_blp_texture_reference(std::string(texture)));
    }

    void brush_change_terrain(math::vector_3d const& pos, float change, float radius, float inner_radius, int brush_type)
    {
      get_ctx()->_world->changeTerrain(pos, change, radius, brush_type, inner_radius);
    }
  }
}
