// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#include <daScript/daScript.h>
#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/scripting/scripting_tool.hpp>
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