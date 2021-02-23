// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#pragma once

#include <string>
#include <set>
#include <functional>
#include <algorithm>
#include <limits>
#include <random>
#include <chrono>

#include <math/vector_3d.hpp>

class World;

namespace noggit
{
    class camera;

    namespace scripting
    {
        struct script_vec;

        struct script_context
        {
            script_context(World *world, math::vector_3d pos, float outer_radius, float inner_radius, noggit::camera *camera, bool alt, bool shift, bool ctrl, bool space);
            World *_world;
            bool _holding_alt;
            bool _holding_shift;
            bool _holding_ctrl;
            bool _holding_space;
            noggit::camera *_camera;
            math::vector_3d _pos;
            float _outer_radius;
            float _inner_radius;
        };

        script_context* get_ctx();
        void set_ctx(script_context *ctx);
    } // namespace scripting
} // namespace noggit