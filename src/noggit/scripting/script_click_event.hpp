// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <das/Context.fwd.hpp>

#include <math/vector_3d.hpp>

#include <string>
#include <set>
#include <functional>
#include <algorithm>
#include <limits>
#include <random>
#include <chrono>

class World;

namespace noggit
{
  class camera;

  namespace scripting
  {
    struct script_vec;
    struct script_click_event
    {
      script_click_event(World* world, math::vector_3d pos, float outer_radius, float inner_radius, noggit::camera* camera, bool alt, bool shift, bool ctrl, bool space, float dt);
      World* _world;
      math::vector_3d _pos;
      float _outer_radius;
      float _inner_radius;
      noggit::camera* _camera;
      bool _holding_alt;
      bool _holding_shift;
      bool _holding_ctrl;
      bool _holding_space;
      float _dt;
    };
  } // namespace scripting
} // namespace noggit
