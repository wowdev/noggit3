// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

namespace noggit
{
  namespace scripting
  {
    math::vector_3d pos();
    math::vector_3d vec(float x, float y, float c);
    void add_m2(char const* filename, math::vector_3d const& pos, float scale, math::vector_3d const& rotation);
    void add_wmo(char const* filename, math::vector_3d const& pos, math::vector_3d const& rotation);
    unsigned int get_map_id();
    unsigned int get_area_id(math::vector_3d const& pos);
    float cam_pitch();
    float cam_yaw();
    float outer_radius();
    float inner_radius();
    bool holding_alt();
    bool holding_shift();
    bool holding_ctrl();
    bool holding_space();
  } // namespace scripting
} // namespace noggit
