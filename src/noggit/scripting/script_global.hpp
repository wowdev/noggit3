#pragma once

#include <math/vector_3d.hpp>

namespace noggit {
  namespace scripting {
    class scripting_tool;

    math::vector_3d camera_pos(scripting_tool * global);
    void add_m2(scripting_tool * global, char const* filename, math::vector_3d const& pos, float scale, math::vector_3d const& rotation);
    void add_wmo(scripting_tool * global, char const* filename, math::vector_3d const& pos, math::vector_3d const& rotation);
    unsigned int get_map_id(scripting_tool * global);
    unsigned int get_area_id(scripting_tool * global, math::vector_3d const& pos);
    float cam_pitch(scripting_tool * global);
    float cam_yaw(scripting_tool * global);

    bool holding_alt(scripting_tool * global);
    bool holding_shift(scripting_tool * global);
    bool holding_ctrl(scripting_tool * global);
    bool holding_space(scripting_tool * global);

    bool holding_left_mouse(scripting_tool * global);
    bool holding_right_mouse(scripting_tool * global);
  }
}