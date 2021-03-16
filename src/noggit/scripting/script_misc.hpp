// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

namespace das {
  class Context;
}

namespace noggit
{
  namespace scripting
  {
    math::vector_3d pos (das::Context* context);
    math::vector_3d vec(float x, float y, float c);
    void add_m2(char const* filename, math::vector_3d const& pos, float scale, math::vector_3d const& rotation, das::Context* context);
    void add_wmo(char const* filename, math::vector_3d const& pos, math::vector_3d const& rotation, das::Context* context);
    unsigned int get_map_id (das::Context* context);
    unsigned int get_area_id(math::vector_3d const& pos, das::Context* context);
    float cam_pitch (das::Context* context);
    float cam_yaw (das::Context* context);
    float outer_radius (das::Context* context);
    float inner_radius (das::Context* context);
    bool holding_alt (das::Context* context);
    bool holding_shift (das::Context* context);
    bool holding_ctrl (das::Context* context);
    bool holding_space (das::Context* context);
    float dt (das::Context* context);
  } // namespace scripting
} // namespace noggit
