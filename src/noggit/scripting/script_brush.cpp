#include <noggit/scripting/script_brush.hpp>
#include <noggit/World.h>
#include <noggit/scripting/script_context.hpp>
#include <math/vector_4d.hpp>
#include <noggit/camera.hpp>
#include <noggit/Brush.h>

void noggit::scripting::brush_set_area_id(math::vector_3d &pos, int id, bool adt)
{
    get_ctx()->_world->setAreaID(pos, id, adt);
}

void noggit::scripting::brush_change_vertex_color(math::vector_3d &pos, math::vector_3d &color, float alpha, float change, float radius, bool editMode)
{
    auto v = color;
    get_ctx()->_world->changeShader(pos, math::vector_4d(v.x, v.y, v.z, alpha), change, radius, editMode);
}

math::vector_3d noggit::scripting::brush_get_vertex_color(math::vector_3d &pos)
{
    return get_ctx()->_world->pickShaderColor(pos);
}

void noggit::scripting::brush_flatten_terrain(math::vector_3d &pos, float remain, float radius, int brush_type, bool lower, bool raise, math::vector_3d &origin, double angle, double orientation)
{
    get_ctx()->_world->flattenTerrain(pos, remain, radius, brush_type, flatten_mode(raise, lower), origin, math::degrees(angle), math::degrees(orientation));
}

void noggit::scripting::brush_blur_terrain(math::vector_3d &pos, float remain, float radius, int brush_type, bool lower, bool raise)
{
    get_ctx()->_world->blurTerrain(pos, remain, radius, brush_type, flatten_mode(raise, lower));
}

void noggit::scripting::brush_erase_textures(math::vector_3d &pos)
{
    get_ctx()->_world->eraseTextures(pos);
}

void noggit::scripting::brush_clear_shadows(math::vector_3d &pos)
{
    get_ctx()->_world->clear_shadows(pos);
}

void noggit::scripting::brush_clear_textures(math::vector_3d &pos)
{
    get_ctx()->_world->clearTextures(pos);
}

void noggit::scripting::brush_clear_height(math::vector_3d &pos)
{
    get_ctx()->_world->clearHeight(pos);
}

void noggit::scripting::brush_set_hole(math::vector_3d &pos, bool big, bool hole)
{
    get_ctx()->_world->setHole(pos, big, hole);
}

void noggit::scripting::brush_set_hole_adt(math::vector_3d &pos, bool hole)
{
    get_ctx()->_world->setHoleADT(pos, hole);
}

void noggit::scripting::brush_update_vertices()
{
    get_ctx()->_world->updateVertexCenter();
    get_ctx()->_world->updateSelectedVertices();
}

void noggit::scripting::brush_deselect_vertices(math::vector_3d &pos, float radius)
{
    get_ctx()->_world->deselectVertices(pos, radius);
}

void noggit::scripting::brush_move_vertices(float h)
{
    get_ctx()->_world->moveVertices(h);
}

void noggit::scripting::brush_flatten_vertices(float h)
{
    get_ctx()->_world->flattenVertices(h);
}

void noggit::scripting::brush_clear_vertex_selection()
{
    get_ctx()->_world->clearVertexSelection();
}

void noggit::scripting::brush_paint_texture(math::vector_3d &pos, float strength, float pressure, float hardness, float radius, const char *texture)
{
    auto brush = Brush();
    brush.setHardness(hardness);
    brush.setRadius(radius);
    get_ctx()->_world->paintTexture(pos, &brush, strength, pressure, scoped_blp_texture_reference(std::string(texture)));
}

void noggit::scripting::brush_change_terrain(math::vector_3d &pos, float change, float radius, float inner_radius, int brush_type)
{
    get_ctx()->_world->changeTerrain(pos, change, radius, brush_type, inner_radius);
}