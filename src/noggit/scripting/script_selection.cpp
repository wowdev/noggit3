#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_texture_index.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <dukglue.h>

#include <noggit/World.h>
#include <noggit/MapChunk.h>
#include <noggit/MapHeaders.h>

using namespace noggit::scripting;

static float chunk_vertex_offset(float base, int index)
{
    return base+index*UNITSIZE;
}

struct TextureIndices{
    struct TextureIndex {
        signed char chunk;
        unsigned short index;
    };

    TextureIndex index[32];
};

noggit::scripting::script_selection::script_selection
(World* world, float origin_x, float origin_z, float inner_radius, float outer_radius)
{
    _world = world;
    _origin_x = origin_x;
    _origin_z = origin_z;
    _inner_radius = inner_radius;
    _outer_radius = outer_radius;
    _low_x_inner = origin_x - inner_radius;
    _low_x_outer = origin_x - outer_radius;
    _high_x_inner = origin_x + inner_radius;
    _high_x_outer = origin_x + outer_radius;

    _low_z_inner = origin_z - inner_radius;
    _low_z_outer = origin_z - outer_radius;
    _high_z_inner = origin_z + inner_radius;
    _high_z_outer = origin_z + outer_radius;

    math::vector_3d pos1(_low_x_outer,0,_low_z_outer);
    math::vector_3d pos2(_high_x_outer,0,_high_z_outer);
    world->select_all_chunks_between(pos1,pos2,_chunks);
    reset_cur_chunk();
}

bool noggit::scripting::script_selection::is_on_chunk()
{
    return _chunks.size() == 0 || _cur_chunk != _chunks.end();
}

bool noggit::scripting::script_selection::is_on_vertex()
{
    return _vertex_index < 145;
}

bool noggit::scripting::script_selection::is_on_tex()
{
    return _cur_texture != std::end(_cur_texture_set->indices) && *_cur_texture != -1;
}

void noggit::scripting::script_selection::next_tex()
{
    if(_cur_texture!=std::end(_cur_texture_set->indices))
        ++_cur_texture;
}

void noggit::scripting::script_selection::skip_vertices()
{
    while(_vertex_index<145)
    {
        if(_cur_vertex->x<=_low_x_outer||_cur_vertex->x>=_high_x_outer||
           _cur_vertex->z<=_low_z_outer||_cur_vertex->z>=_high_z_outer)
        {
            ++_cur_texture_set;
            ++_cur_mccv;
            ++_cur_vertex;
            ++_vertex_index;
        } 
        else
        {
            break;
        }
    }

    if(_vertex_index<145)
    {
        reset_cur_tex();
    }
}


void noggit::scripting::script_selection::next_vertex()
{
    if(_vertex_index>=145)
    {
        return;
    }
    ++_vertex_index;
    ++_cur_texture_set;
    ++_cur_mccv;
    ++_cur_vertex;
    skip_vertices();
}

void noggit::scripting::script_selection::next_chunk()
{
    if(_cur_chunk!=_chunks.end())
    {
        ++_cur_chunk;
        if(_cur_chunk!=_chunks.end())
        {
            reset_cur_vertex();
        }
    }
}

void noggit::scripting::script_selection::reset_cur_chunk()
{
    _cur_chunk = std::begin(_chunks);
    reset_cur_vertex();
}

void noggit::scripting::script_selection::reset_cur_vertex()
{
    _cur_vertex = std::begin((*_cur_chunk)->mVertices);
    _cur_mccv = std::begin((*_cur_chunk)->mccv);
    _cur_texture_set = std::begin(texture_index);
    _vertex_index = 0;
    skip_vertices();
}

void noggit::scripting::script_selection::reset_cur_tex()
{
    _cur_texture = std::begin(_cur_texture_set->indices);
}

void noggit::scripting::script_selection::vert_set_y(float value)
{
    _cur_vertex->y = value;
}

float noggit::scripting::script_selection::vert_get_x()
{
    return _cur_vertex->x;
}

float noggit::scripting::script_selection::vert_get_y()
{
    return _cur_vertex->y;
}

float noggit::scripting::script_selection::vert_get_z()
{
    return _cur_vertex->z;
}

int noggit::scripting::script_selection::chunk_add_texture(std::string texture)
{
    return (*_cur_chunk)->texture_set->addTexture(scoped_blp_texture_reference(texture));
}

void noggit::scripting::script_selection::chunk_clear_textures()
{
    (*_cur_chunk)->texture_set->eraseTextures();
}

void noggit::scripting::script_selection::chunk_remove_texture(size_t index)
{
    (*_cur_chunk)->texture_set->eraseTexture(index);
}

void noggit::scripting::script_selection::vert_set_color(float r, float g, float b)
{
    (*_cur_chunk)->CreateMCCV();
    _cur_mccv->x = 1.0+r;
    _cur_mccv->y = 1.0+g;
    _cur_mccv->z = 1.0+b;
}

std::string noggit::scripting::script_selection::chunk_get_texture(size_t index)
{
    return (*_cur_chunk)->texture_set->texture(index)->filename;
}

float noggit::scripting::script_selection::tex_get_alpha(size_t index)
{
    auto& ts = (*_cur_chunk)->texture_set;
    if(index>=ts->nTextures) return 0;
    if(index==0) return 1.0f;
    ts->create_temporary_alphamaps_if_needed();
    return ts->tmp_edit_values.get()[index][*_cur_texture];
}

void noggit::scripting::script_selection::vert_fill_tex_alpha(size_t index, float alpha)
{
    auto& ts = (*_cur_chunk)->texture_set;
    ts->create_temporary_alphamaps_if_needed();
    if(index==0)
    {
        return;
    }

    for(auto& tex_index: _cur_texture_set->indices)
    {
        if(tex_index==-1)
        {
            break;
        }
        ts->tmp_edit_values.get()[index][tex_index] = alpha;
    }
}

void noggit::scripting::script_selection::tex_set_alpha(size_t index, float alpha)
{
    auto& ts = (*_cur_chunk)->texture_set;
    ts->create_temporary_alphamaps_if_needed();
    if(index==0)
    {
        return;
    }
    ts->tmp_edit_values.get()[index][*_cur_texture] = alpha;
}

void noggit::scripting::script_selection::chunk_apply_heightmap()
{
    (*_cur_chunk)->updateVerticesData();
}

void noggit::scripting::script_selection::chunk_apply_textures()
{
    (*_cur_chunk)->texture_set->apply_alpha_changes();
}

void noggit::scripting::script_selection::chunk_apply_vertex_color()
{
    (*_cur_chunk)->UpdateMCCV();
}

void noggit::scripting::script_selection::chunk_apply_all()
{
    chunk_apply_heightmap();
    chunk_apply_textures();
    chunk_apply_vertex_color();
}

bool noggit::scripting::script_selection::vert_is_water_aligned()
{
    return (_vertex_index%17)>8;
}

void noggit::scripting::script_selection::vert_set_water(int type, float height)
{
    if(!vert_is_water_aligned())
    {
        return;
    }
    // TODO: Not very efficient...
    (*_cur_chunk)->liquid_chunk()->paintLiquid(*_cur_vertex,1,type,true,math::radians(0),math::radians(0),true,math::vector_3d(0,height,0),true,true,*_cur_chunk,1);
}

int noggit::scripting::script_selection::chunk_get_area_id()
{
    return (*_cur_chunk)->getAreaID();
}

void noggit::scripting::script_selection::chunk_set_area_id(int value)
{
    return (*_cur_chunk)->setAreaID(value);
}

void noggit::scripting::script_selection::vert_set_hole(bool add)
{
    (*_cur_chunk)->setHole(*_cur_vertex,false,add);
}

void noggit::scripting::script_selection::chunk_set_impassable(bool add)
{
    (*_cur_chunk)->setFlag(add,0x2);
}

void noggit::scripting::register_selection_functions(duk_context* ctx)
{
    GLUE_METHOD(ctx,script_selection,is_on_chunk);
    GLUE_METHOD(ctx,script_selection,is_on_vertex);
    GLUE_METHOD(ctx,script_selection,is_on_tex);

    GLUE_METHOD(ctx,script_selection,next_chunk);
    GLUE_METHOD(ctx,script_selection,next_vertex);
    GLUE_METHOD(ctx,script_selection,next_tex);

    GLUE_METHOD(ctx,script_selection,reset_cur_chunk);
    GLUE_METHOD(ctx,script_selection,reset_cur_vertex);
    GLUE_METHOD(ctx,script_selection,reset_cur_tex);

    GLUE_METHOD(ctx,script_selection,chunk_add_texture);
    GLUE_METHOD(ctx,script_selection,chunk_clear_textures);
    GLUE_METHOD(ctx,script_selection,chunk_remove_texture);
    GLUE_METHOD(ctx,script_selection,chunk_get_texture);
    GLUE_METHOD(ctx,script_selection,chunk_apply_textures);
    GLUE_METHOD(ctx,script_selection,chunk_apply_heightmap);
    GLUE_METHOD(ctx,script_selection,chunk_apply_vertex_color);
    GLUE_METHOD(ctx,script_selection,chunk_apply_all);
    GLUE_METHOD(ctx,script_selection,chunk_set_impassable);
    GLUE_METHOD(ctx,script_selection,chunk_get_area_id);
    GLUE_METHOD(ctx,script_selection,chunk_set_area_id);
    GLUE_METHOD(ctx,script_selection,vert_get_x);
    GLUE_METHOD(ctx,script_selection,vert_get_y);
    GLUE_METHOD(ctx,script_selection,vert_get_z);

    GLUE_METHOD(ctx,script_selection,vert_set_y);
    GLUE_METHOD(ctx,script_selection,vert_set_color);
    GLUE_METHOD(ctx,script_selection,vert_set_water);
    GLUE_METHOD(ctx,script_selection,vert_is_water_aligned);
    GLUE_METHOD(ctx,script_selection,vert_set_hole);
    GLUE_METHOD(ctx,script_selection,tex_get_alpha);
    GLUE_METHOD(ctx,script_selection,tex_set_alpha);
}