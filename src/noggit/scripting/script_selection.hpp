#pragma once

#include <vector>
#include <string>

namespace math {
    struct vector_3d;
}
class MapChunk;
class World;
struct duk_hthread;

struct TextureIndex {
    short indices[36];
};

namespace noggit {
    namespace scripting {
        class script_selection {
        public:
            script_selection(World* world, float origin_x, float origin_z, float inner_radius, float outer_radius);

            bool is_on_chunk();
            bool is_on_vertex();
            bool is_on_tex();

            void next_chunk();
            void next_vertex();
            void next_tex();

            void reset_cur_chunk();
            void reset_cur_vertex();
            void reset_cur_tex();

            int chunk_add_texture(std::string texture);
            void chunk_clear_textures();
            void chunk_remove_texture(size_t index);
            std::string chunk_get_texture(size_t index);
            void chunk_apply_textures();
            void chunk_apply_heightmap();
            void chunk_apply_vertex_color();
            void chunk_apply_all();
            void chunk_set_impassable(bool add);
            int chunk_get_area_id();
            void chunk_set_area_id(int value);

            float vert_get_x();
            float vert_get_y();
            float vert_get_z();
            void vert_set_y(float value);
            void vert_set_color(float r, float g, float b);
            void vert_set_water(int type, float height);
            bool vert_is_water_aligned();
            void vert_set_hole(bool add);
            void vert_fill_tex_alpha(size_t index, float alpha);
            float tex_get_alpha(size_t index);
            void tex_set_alpha(size_t index, float alpha);
        private:
            std::vector<MapChunk*> _chunks;
            World* _world;
            float _inner_radius;
            float _outer_radius;
            float _origin_z;
            float _origin_x;

            float _low_x_inner;
            float _low_z_inner;
            float _high_x_inner;
            float _high_z_inner;

            float _low_x_outer;
            float _low_z_outer;
            float _high_x_outer;
            float _high_z_outer;
        
            std::vector<MapChunk*>::iterator _cur_chunk;
            math::vector_3d* _cur_vertex;
            unsigned _vertex_index = 0;
            math::vector_3d* _cur_mccv;
            TextureIndex* _cur_texture_set;
            short *_cur_texture;

            void skip_vertices();
        };

        void register_selection_functions(duk_hthread* ctx);
    }
}