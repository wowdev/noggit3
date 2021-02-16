#include <noggit/scripting/script_chunk.hpp>
#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/MapChunk.h>
#include <noggit/MapHeaders.h>
#include <boost/algorithm/string/predicate.hpp>

using namespace noggit::scripting;

noggit::scripting::script_chunk::script_chunk(script_selection *sel, MapChunk *chunk)
{
    _sel = sel;
    _chunk = chunk;
}

void noggit::scripting::chunk_set_hole(script_chunk &chunk, bool hole)
{
    chunk._chunk->setHole(math::vector_3d(0, 0, 0), true, hole);
}

int noggit::scripting::chunk_add_texture(script_chunk &chunk, const char *texture)
{
    std::string tex = std::string(texture);
    if (!boost::starts_with(tex, "tileset/"))
    {
        tex = "tileset/" + tex;
    }
    return chunk._chunk->texture_set->addTexture(scoped_blp_texture_reference(tex));
}

void noggit::scripting::chunk_clear_textures(script_chunk &chunk)
{
    chunk._chunk->texture_set->eraseTextures();
}

void noggit::scripting::chunk_remove_texture(script_chunk &chunk, int index)
{
    chunk._chunk->texture_set->eraseTexture(index);
}

const char *noggit::scripting::chunk_get_texture(script_chunk &chunk, int index)
{
    return (chunk._chunk->texture_set->texture(index)->filename).c_str();
}

void noggit::scripting::chunk_apply_heightmap(script_chunk &chunk)
{
    chunk._chunk->updateVerticesData();
    get_ctx()->_world->recalc_norms(chunk._chunk);
}

void noggit::scripting::chunk_apply_textures(script_chunk &chunk)
{
    chunk._chunk->texture_set->apply_alpha_changes();
}

void noggit::scripting::chunk_apply_vertex_color(script_chunk &chunk)
{
    chunk._chunk->UpdateMCCV();
}

void noggit::scripting::chunk_apply_all(script_chunk &chunk)
{
    chunk_apply_heightmap(chunk);
    chunk_apply_textures(chunk);
    chunk_apply_vertex_color(chunk);
}

int noggit::scripting::chunk_get_area_id(script_chunk &chunk)
{
    return chunk._chunk->getAreaID();
}

void noggit::scripting::chunk_set_area_id(script_chunk &chunk, int value)
{
    return chunk._chunk->setAreaID(value);
}

void noggit::scripting::chunk_set_impassable(script_chunk &chunk, bool add)
{
    chunk._chunk->setFlag(add, 0x2);
}

static bool is_on_vert(script_chunk &chunk)
{
    return chunk._vert_index < 145;
}

static bool is_on_tex(script_chunk &chunk)
{
    return chunk._tex_index < 4096;
}

static void skip_vertices(script_chunk &chunk)
{
    auto sel = chunk._sel;
    if (sel == nullptr)
        return;
    while (is_on_vert(chunk))
    {
        auto &vert = chunk._chunk->mVertices[chunk._vert_index];
        if (vert.x <= sel->_min.x || vert.x >= sel->_max.x ||
            vert.z <= sel->_min.z || vert.z >= sel->_max.z)
        {
            ++chunk._vert_index;
        }
        else
        {
            break;
        }
    }
}

bool noggit::scripting::chunk_next_vert(script_chunk &chunk)
{
    ++chunk._vert_index;
    if (is_on_vert(chunk))
    {
        skip_vertices(chunk);
        return is_on_vert(chunk);
    }
    else
    {
        return false;
    }
}

bool noggit::scripting::chunk_next_tex(script_chunk &chunk)
{
    ++chunk._tex_index;
    return is_on_tex(chunk);
}

void noggit::scripting::chunk_reset_vert_itr(script_chunk &chunk)
{
    chunk._vert_index = -1;
}

void noggit::scripting::chunk_reset_tex_itr(script_chunk &chunk)
{
    chunk._tex_index = -1;
}

script_vert noggit::scripting::chunk_get_vert(script_chunk &chunk)
{
    return script_vert(chunk._chunk, chunk._vert_index);
}

script_tex noggit::scripting::chunk_get_tex(script_chunk &chunk)
{
    return script_tex(chunk._chunk, chunk._tex_index);
}

void noggit::scripting::chunk_clear_colors(script_chunk &chunk)
{
    for(int i=0;i<mapbufsize; ++i) 
    {
        chunk._chunk->mccv[i] = math::vector_3d(1.0,1.0,1.0);
    }
}