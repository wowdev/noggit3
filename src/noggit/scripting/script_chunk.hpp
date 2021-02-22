// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/MapChunk.h>
#include <noggit/scripting/script_vert.hpp>

namespace noggit
{
    namespace scripting
    {
        struct script_selection;

        struct script_chunk
        {
            script_chunk(script_selection *sel, MapChunk *chunk);
            script_chunk() {}
            MapChunk *_chunk;
            script_selection *_sel;
            int _vert_index = -1;
            int _tex_index = -1;
        };

        void chunk_remove_texture(script_chunk &chunk, int index);
        const char *chunk_get_texture(script_chunk &chunk, int index);
        int chunk_add_texture(script_chunk &chunk, const char *texture);
        void chunk_clear_textures(script_chunk &chunk);

        void chunk_set_hole(script_chunk &chunk, bool hole);

        void chunk_clear_colors(script_chunk &chunk);
        void chunk_apply_textures(script_chunk &chunk);
        void chunk_apply_heightmap(script_chunk &chunk);
        void chunk_apply_vertex_color(script_chunk &chunk);
        void chunk_apply_all(script_chunk &chunk);
        void chunk_set_impassable(script_chunk &chunk, bool add);
        int chunk_get_area_id(script_chunk &chunk);
        void chunk_set_area_id(script_chunk &chunk, int value);

        bool chunk_next_vert(script_chunk &chunk);
        bool chunk_next_tex(script_chunk &chunk);

        void chunk_reset_vert_itr(script_chunk &chunk);
        void chunk_reset_tex_itr(script_chunk &chunk);

        script_vert chunk_get_vert(script_chunk &chunk);
        script_tex chunk_get_tex(script_chunk &chunk);
    } // namespace scripting
} // namespace noggit