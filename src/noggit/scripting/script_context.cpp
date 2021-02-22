// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#include <daScript/daScript.h>
#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/Brush.h>
#include <lodepng.h>
#include <noggit/camera.hpp>


using namespace noggit::scripting;

static script_context *ctx;

void noggit::scripting::set_ctx(script_context *nctx)
{
    ctx = nctx;
}

script_context* noggit::scripting::get_ctx()
{
    return ctx;
}

/** script_context */

noggit::scripting::script_context::script_context(World *world, math::vector_3d pos, float outer_radius, float inner_radius, noggit::camera *camera, bool alt, bool shift, bool ctrl, bool space)
    : _world(world), _pos(pos), _camera(camera),
      _holding_alt(alt), _holding_shift(shift),
      _holding_ctrl(ctrl), _holding_space(space),
      _outer_radius(outer_radius), _inner_radius(inner_radius)
{}

float noggit::scripting::cam_pitch()
{
    return ctx->_camera->pitch()._;
}

float noggit::scripting::outer_radius()
{
    return ctx->_outer_radius;
}

float noggit::scripting::inner_radius()
{
    return ctx->_inner_radius;
}

float noggit::scripting::cam_yaw()
{
    return ctx->_camera->yaw()._;
}

void noggit::scripting::brush_change_terrain(math::vector_3d &pos, float change, float radius, float inner_radius, int brush_type)
{
    ctx->_world->changeTerrain(pos, change, radius, brush_type, inner_radius);
}

math::vector_3d noggit::scripting::vec(float x, float y, float z)
{
    return math::vector_3d(x,y,z);
}

void noggit::scripting::add_m2(const char *filename, math::vector_3d &pos, float scale, math::vector_3d &rotation)
{
    auto p = object_paste_params();
    ctx->_world->addM2(filename, pos, scale, rotation, &p);
}

void noggit::scripting::add_wmo(const char *filename, math::vector_3d &pos, math::vector_3d &rotation)
{
    ctx->_world->addWMO(filename, pos, rotation);
}

unsigned int noggit::scripting::get_map_id()
{
    return ctx->_world->getMapID();
}

unsigned int noggit::scripting::get_area_id(math::vector_3d &pos)
{
    return ctx->_world->getAreaID(pos);
}

void noggit::scripting::brush_set_area_id(math::vector_3d &pos, int id, bool adt)
{
    ctx->_world->setAreaID(pos, id, adt);
}

void noggit::scripting::brush_change_vertex_color(math::vector_3d &pos, math::vector_3d &color, float alpha, float change, float radius, bool editMode)
{
    auto v = color;
    ctx->_world->changeShader(pos, math::vector_4d(v.x, v.y, v.z, alpha), change, radius, editMode);
}

math::vector_3d noggit::scripting::brush_get_vertex_color(math::vector_3d &pos)
{
    return ctx->_world->pickShaderColor(pos);
}

void noggit::scripting::brush_flatten_terrain(math::vector_3d &pos, float remain, float radius, int brush_type, bool lower, bool raise, math::vector_3d &origin, double angle, double orientation)
{
    ctx->_world->flattenTerrain(pos, remain, radius, brush_type, flatten_mode(raise, lower), origin, math::degrees(angle), math::degrees(orientation));
}

void noggit::scripting::brush_blur_terrain(math::vector_3d &pos, float remain, float radius, int brush_type, bool lower, bool raise)
{
    ctx->_world->blurTerrain(pos, remain, radius, brush_type, flatten_mode(raise, lower));
}

void noggit::scripting::brush_erase_textures(math::vector_3d &pos)
{
    ctx->_world->eraseTextures(pos);
}

void noggit::scripting::brush_clear_shadows(math::vector_3d &pos)
{
    ctx->_world->clear_shadows(pos);
}

void noggit::scripting::brush_clear_textures(math::vector_3d &pos)
{
    ctx->_world->clearTextures(pos);
}

void noggit::scripting::brush_clear_height(math::vector_3d &pos)
{
    ctx->_world->clearHeight(pos);
}

void noggit::scripting::brush_set_hole(math::vector_3d &pos, bool big, bool hole)
{
    ctx->_world->setHole(pos, big, hole);
}

void noggit::scripting::brush_set_hole_adt(math::vector_3d &pos, bool hole)
{
    ctx->_world->setHoleADT(pos, hole);
}

void noggit::scripting::brush_update_vertices()
{
    ctx->_world->updateVertexCenter();
    ctx->_world->updateSelectedVertices();
}

void noggit::scripting::brush_deselect_vertices(math::vector_3d &pos, float radius)
{
    ctx->_world->deselectVertices(pos, radius);
}

void noggit::scripting::brush_move_vertices(float h)
{
    ctx->_world->moveVertices(h);
}

void noggit::scripting::brush_flatten_vertices(float h)
{
    ctx->_world->flattenVertices(h);
}

void noggit::scripting::brush_clear_vertex_selection()
{
    ctx->_world->clearVertexSelection();
}

void noggit::scripting::brush_paint_texture(math::vector_3d &pos, float strength, float pressure, float hardness, float radius, const char *texture)
{
    auto brush = Brush();
    brush.setHardness(hardness);
    brush.setRadius(radius);
    ctx->_world->paintTexture(pos, &brush, strength, pressure, scoped_blp_texture_reference(std::string(texture)));
}

bool noggit::scripting::holding_alt()
{
    return ctx->_holding_alt;
}
bool noggit::scripting::holding_shift()
{
    return ctx->_holding_shift;
}
bool noggit::scripting::holding_ctrl()
{
    return ctx->_holding_ctrl;
}
bool noggit::scripting::holding_space()
{
    return ctx->_holding_space;
}

math::vector_3d noggit::scripting::pos()
{
    return ctx->_pos;
}
