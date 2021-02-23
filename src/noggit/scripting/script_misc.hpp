// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#pragma once

namespace noggit {
    namespace scripting {
        math::vector_3d pos();
        math::vector_3d vec(float x, float y, float c);
        void add_m2(const char *filename, math::vector_3d &pos, float scale, math::vector_3d &rotation);
        void add_wmo(const char *filename, math::vector_3d &pos, math::vector_3d &rotation);
        unsigned int get_map_id();
        unsigned int get_area_id(math::vector_3d &);
        float cam_pitch();
        float cam_yaw();
        float outer_radius();
        float inner_radius();
        bool holding_alt();
        bool holding_shift();
        bool holding_ctrl();
        bool holding_space();
    }
}