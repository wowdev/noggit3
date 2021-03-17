#pragma once

#include <sol/sol.hpp>
#include <noggit/World.h>
#include <noggit/camera.hpp>

void register_state(sol::state * lua, World* world, noggit::camera * camera)
{
    lua.set_function("camera_pos", [camera](){ return camera.position; });
}