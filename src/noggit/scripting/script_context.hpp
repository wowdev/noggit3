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

/**
 * script_context.hpp
 * 
 * Contains library functions that scripts can call.
 */
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
        math::vector_3d pos();
        math::vector_3d vec(float x, float y, float c);
        void brush_change_terrain(math::vector_3d &, float change, float radius, float inner_radius, int brush_type);
        void add_m2(const char *filename, math::vector_3d &pos, float scale, math::vector_3d &rotation);
        void add_wmo(const char *filename, math::vector_3d &pos, math::vector_3d &rotation);
        unsigned int get_map_id();
        unsigned int get_area_id(math::vector_3d &);
        void brush_set_area_id(math::vector_3d &, int id, bool adt = false);
        void brush_change_vertex_color(math::vector_3d &pos, math::vector_3d &color, float alpha, float change, float radius, bool editMode);
        math::vector_3d brush_get_vertex_color(math::vector_3d &pos);
        void brush_flatten_terrain(math::vector_3d &pos, float remain, float radius, int brush_type, bool lower, bool raise, math::vector_3d &origin, double angle, double orientation);
        void brush_blur_terrain(math::vector_3d &pos, float remain, float radius, int brush_type, bool lower, bool raise);
        void brush_erase_textures(math::vector_3d &pos);
        void brush_clear_shadows(math::vector_3d &pos);
        void brush_clear_textures(math::vector_3d &pos);
        void brush_clear_height(math::vector_3d &pos);
        void brush_set_hole(math::vector_3d &pos, bool big, bool hole);
        void brush_set_hole_adt(math::vector_3d &pos, bool hole);
        void brush_deselect_vertices(math::vector_3d &pos, float radius);
        void brush_clear_vertex_selection();
        void brush_move_vertices(float h);
        void brush_flatten_vertices(float h);
        void brush_update_vertices();
        void brush_paint_texture(math::vector_3d &pos, float strength, float pressure, float hardness, float radius, const char *texture);
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