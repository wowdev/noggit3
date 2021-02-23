// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/scripting/scripting_tool.hpp>
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

noggit::scripting::script_context::script_context(World *world, math::vector_3d pos, float outer_radius, float inner_radius, noggit::camera *camera, bool alt, bool shift, bool ctrl, bool space)
    : _world(world), _pos(pos), _camera(camera),
      _holding_alt(alt), _holding_shift(shift),
      _holding_ctrl(ctrl), _holding_space(space),
      _outer_radius(outer_radius), _inner_radius(inner_radius)
{}