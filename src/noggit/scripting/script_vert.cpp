#include <noggit/scripting/script_vert.hpp>
#include <noggit/scripting/script_texture_index.hpp>

#include <vector>

using namespace noggit::scripting;

noggit::scripting::script_vert::script_vert(MapChunk *chunk, int index)
    : _chunk(chunk), _index(index)
{
}

noggit::scripting::script_tex::script_tex(MapChunk *chunk, int index)
    : _chunk(chunk), _index(index)
{
}

void noggit::scripting::vert_set_height(script_vert &vert, float value)
{
    vert._chunk->mVertices[vert._index].y = value;
}

void noggit::scripting::vert_add_height(script_vert &vert, float value)
{
    vert._chunk->mVertices[vert._index].y += value;
}

void noggit::scripting::vert_sub_height(script_vert &vert, float value)
{
    vert._chunk->mVertices[vert._index].y -= value;
}

void noggit::scripting::vert_set_color(script_vert &vert, float r, float g, float b)
{
    vert._chunk->CreateMCCV();
    vert._chunk->mccv[vert._index] = math::vector_3d(r, g, b);
}

void noggit::scripting::vert_set_water(script_vert &vert, int type, float height)
{
    if (!vert_is_water_aligned(vert))
    {
        return;
    }

    // TODO: Extremely inefficient
    vert._chunk->liquid_chunk()->paintLiquid(vert_get_pos(vert), 1, type, true, math::radians(0), math::radians(0), true, math::vector_3d(0, height, 0), true, true, vert._chunk, 1);
}

void noggit::scripting::vert_set_hole(script_vert &vert, bool add)
{
    vert._chunk->setHole(vert_get_pos(vert), false, add);
}

math::vector_3d noggit::scripting::vert_get_pos(script_vert &vert)
{
    return vert._chunk->mVertices[vert._index];
}

void noggit::scripting::vert_set_alpha(script_vert &vert, int index, float alpha)
{
    if (index == 0)
    {
        return;
    }
    auto &ts = vert._chunk->texture_set;
    ts->create_temporary_alphamaps_if_needed();

    auto tex_indices = texture_index[vert._index];

    for (auto iter = std::begin(tex_indices.indices); iter != std::end(tex_indices.indices); ++iter)
    {
        if (*iter == -1)
            break;
        ts->tmp_edit_values.get()[index][*iter] = alpha;
    }
}

float noggit::scripting::vert_get_alpha(script_vert &vert, int index)
{
    if (index == 0)
    {
        return 1;
    }
    auto &ts = vert._chunk->texture_set;
    ts->create_temporary_alphamaps_if_needed();
    auto tex_indices = texture_index[vert._index];

    float sum = 0;
    int ctr = 0;
    for (auto iter = std::begin(tex_indices.indices); iter != std::end(tex_indices.indices); ++iter)
    {
        if (*iter == -1)
            break;
        sum += ts->tmp_edit_values.get()[index][*iter];
        ++ctr;
    }
    return sum / float(ctr);
}

bool noggit::scripting::vert_is_water_aligned(script_vert &vert)
{
    return (vert._index % 17) > 8;
}

static bool is_tex_done(script_vert &vert)
{
    return vert._tex_index >= 36 || texture_index[vert._index].indices[vert._tex_index] == -1;
}

void noggit::scripting::vert_reset_tex(script_vert &vert)
{
    vert._tex_index = -1;
}

bool noggit::scripting::vert_next_tex(script_vert &vert)
{
    ++vert._tex_index;
    return !is_tex_done(vert);
}

script_tex noggit::scripting::vert_get_tex(script_vert &vert)
{
    return script_tex(vert._chunk, texture_index[vert._index].indices[vert._tex_index]);
}

float noggit::scripting::tex_get_alpha(script_tex &tex, int index)
{
    auto &ts = tex._chunk->texture_set;
    ts->create_temporary_alphamaps_if_needed();
    return ts->tmp_edit_values.get()[index][tex._index];
}

void noggit::scripting::tex_set_alpha(script_tex &tex, int index, float value)
{
    auto &ts = tex._chunk->texture_set;
    ts->create_temporary_alphamaps_if_needed();
    ts->tmp_edit_values.get()[index][tex._index] = value;
}

math::vector_3d noggit::scripting::tex_get_pos_2d(script_tex &tex)
{
    float cx = tex._chunk->xbase;
    float cz = tex._chunk->zbase;
    float x = tex._index % 64;
    float z = (float(tex._index) / 64.0);
    return math::vector_3d(cx + x * TEXDETAILSIZE, 0, cz + z * TEXDETAILSIZE);
}