#pragma once

#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/World.h>
#include <noggit/camera.hpp>

#include <sol/sol.hpp>

void register_state(sol::state * lua, noggit::scripting::scripting_tool * tool)
{
    lua->set_function("camera_pos", [tool](){ return tool->get_camera()->position; });
}