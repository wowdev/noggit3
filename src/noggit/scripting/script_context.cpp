// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/World.h>
#include <noggit/camera.hpp>

namespace noggit
{
  namespace scripting
  {
    // makes it easier for scripters to not pass this around
    static script_context* ctx = nullptr;

    void set_ctx(script_context* nctx)
    {
      ctx = nctx;
    }

    script_context* get_ctx(const char* caller)
    {
      if(ctx==nullptr)
      {
        throw script_exception(
          caller,
          "accessing script context outside of interact (click/hold/release) event");
      }
      return ctx;
    }

    script_context::script_context(World* world, math::vector_3d pos, float outer_radius, float inner_radius, noggit::camera* camera, bool alt, bool shift, bool ctrl, bool space, float dt)
      : _world(world),
        _pos(pos),
        _outer_radius(outer_radius),
        _inner_radius(inner_radius),
        _camera(camera),
        _holding_alt(alt),
        _holding_shift(shift),
        _holding_ctrl(ctrl),
        _holding_space(space),
        _dt(dt)
    {
    }
  } // namespace scripting
} // namespace noggit
