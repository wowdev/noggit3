#pragma once

#include <string>
#include <set>
#include <functional>
#include <algorithm>
#include <limits>
#include <random>
#include <chrono>
#include <noggit/scripting/script_selection.hpp>

#include <lodepng.h>
#include <noggit/scripting/script_setup.hpp>
#include <noggit/camera.hpp>

/**
 * script_context.hpp
 * 
 * Contains library functions that scripts can call.
 */
class World;
namespace math {
    struct vector_3d;
}

struct duk_hthread;

namespace noggit
{
    class camera;

    namespace scripting
    {
        struct script_vec;
        typedef std::shared_ptr<script_vec> vecptr;

        class script_context {
        public:
            script_context(World *world, math::vector_3d pos, float outer_radius, float inner_radius, noggit::camera *camera, bool alt, bool shift, bool ctrl, bool space);
            std::shared_ptr<script_vec> pos();
            std::shared_ptr<script_selection> select(float origin_x, float origin_z, float inner_radius, float outer_radius);
            void change_terrain(vecptr, float change, float radius, float inner_radius, int brush_type);
            void add_m2(std::string filename, vecptr pos, float scale, vecptr rotation);
            void add_wmo(std::string filename, vecptr pos, vecptr rotation);
            unsigned int get_map_id();
            unsigned int get_area_id(vecptr);
            void set_area_id(vecptr, int id, bool adt = false);
            void change_vertex_color(vecptr pos, vecptr color, float alpha, float change, float radius, bool editMode);
            vecptr get_vertex_color(vecptr pos);
            void flatten_terrain(vecptr pos, float remain, float radius, int brush_type, bool lower, bool raise, vecptr origin, double angle, double orientation);
            void blur_terrain(vecptr pos, float remain, float radius, int brush_type, bool lower, bool raise);
            void erase_textures(vecptr pos);
            void clear_shadows(vecptr pos);
            void clear_textures(vecptr pos);
            void clear_height(vecptr pos);
            void set_hole(vecptr pos, bool big, bool hole);
            void set_hole_adt(vecptr pos, bool hole);
            void deselect_vertices_radius(vecptr pos, float radius);
            void clear_vertex_selection();
            void move_vertices(float h);
            void flatten_vertices(float h);
            void update_vertices();

            void paint_texture(vecptr pos, float strength, float pressure, float hardness, float radius, std::string texture);
            
            float cam_pitch();
            float cam_yaw();

            float outer_radius();
            float inner_radius();

            bool holding_alt();
            bool holding_shift();
            bool holding_ctrl();
            bool holding_space();
        private:
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

        void register_context_functions(duk_hthread* ctx);
    }
}