#include <daScript/daScript.h>
#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <lodepng.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/camera.hpp>
#include <noggit/scripting/script_misc.hpp>

float noggit::scripting::cam_pitch()
{
    return get_ctx()->_camera->pitch()._;
}

float noggit::scripting::outer_radius()
{
    return get_ctx()->_outer_radius;
}

float noggit::scripting::inner_radius()
{
    return get_ctx()->_inner_radius;
}

float noggit::scripting::cam_yaw()
{
    return get_ctx()->_camera->yaw()._;
}

math::vector_3d noggit::scripting::vec(float x, float y, float z)
{
    return math::vector_3d(x,y,z);
}

void noggit::scripting::add_m2(const char *filename, math::vector_3d &pos, float scale, math::vector_3d &rotation)
{
    auto p = object_paste_params();
    get_ctx()->_world->addM2(filename, pos, scale, rotation, &p);
}

void noggit::scripting::add_wmo(const char *filename, math::vector_3d &pos, math::vector_3d &rotation)
{
    get_ctx()->_world->addWMO(filename, pos, rotation);
}

unsigned int noggit::scripting::get_map_id()
{
    return get_ctx()->_world->getMapID();
}

unsigned int noggit::scripting::get_area_id(math::vector_3d &pos)
{
    return get_ctx()->_world->getAreaID(pos);
}
bool noggit::scripting::holding_alt()
{
    return get_ctx()->_holding_alt;
}
bool noggit::scripting::holding_shift()
{
    return get_ctx()->_holding_shift;
}
bool noggit::scripting::holding_ctrl()
{
    return get_ctx()->_holding_ctrl;
}
bool noggit::scripting::holding_space()
{
    return get_ctx()->_holding_space;
}

math::vector_3d noggit::scripting::pos()
{
    return get_ctx()->_pos;
}