#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/scripting/script_vec.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_printf.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <noggit/scripting/script_setup.hpp>
#include <noggit/Brush.h>
#include <math/vector_3d.hpp>
#include <dukglue.h>

using namespace noggit::scripting;

/** script_context */

noggit::scripting::script_context::script_context(World *world, math::vector_3d pos, float outer_radius, float inner_radius, noggit::camera *camera, bool alt, bool shift, bool ctrl, bool space)
: _world(world), _pos(pos), _camera(camera),
    _holding_alt(alt), _holding_shift(shift), 
    _holding_ctrl(ctrl), _holding_space(space),
    _outer_radius(outer_radius), _inner_radius(inner_radius)
    {}

float noggit::scripting::script_context::cam_pitch()
{
    return _camera->pitch()._;
}

float noggit::scripting::script_context::outer_radius()
{
    return _outer_radius;
}

float noggit::scripting::script_context::inner_radius()
{
    return _inner_radius;
}

float noggit::scripting::script_context::cam_yaw()
{
    return _camera->yaw()._;
}

std::shared_ptr<script_selection> noggit::scripting::script_context::select(float origin_x, float origin_z, float inner_radius, float outer_radius)
{
    return std::make_shared<script_selection>(_world,origin_x,origin_z,inner_radius,outer_radius);
}

void noggit::scripting::script_context::change_terrain(vecptr pos, float change, float radius, float inner_radius, int brush_type)
{
    _world->changeTerrain(pos->_vec,change,radius,brush_type,inner_radius);
}

void noggit::scripting::script_context::add_m2(std::string filename, vecptr pos, float scale, vecptr rotation)
{
    auto p = object_paste_params();
    _world->addM2(filename,pos->_vec,scale,rotation->_vec,&p);
}

void noggit::scripting::script_context::add_wmo(std::string filename, vecptr pos, vecptr rotation)
{
    _world->addWMO(filename,pos->_vec,rotation->_vec);
}

unsigned int noggit::scripting::script_context::get_map_id()
{
    return _world->getMapID();
}

unsigned int noggit::scripting::script_context::get_area_id(vecptr pos)
{
    return _world->getAreaID(pos->_vec);
}

void noggit::scripting::script_context::set_area_id(vecptr pos, int id, bool adt)
{
    _world->setAreaID(pos->_vec,id,adt);
}

void noggit::scripting::script_context::change_vertex_color(vecptr pos, vecptr color, float alpha, float change, float radius, bool editMode)
{
    auto v = color->_vec;
    _world->changeShader(pos->_vec,math::vector_4d(v.x,v.y,v.z,alpha),change,radius,editMode);
}

vecptr noggit::scripting::script_context::get_vertex_color(vecptr pos)
{
    return std::make_shared<script_vec>(_world->pickShaderColor(pos->_vec));
}

void noggit::scripting::script_context::flatten_terrain(vecptr pos, float remain, float radius, int brush_type, bool lower, bool raise, vecptr origin, double angle, double orientation)
{
    _world->flattenTerrain(pos->_vec,remain,radius,brush_type,flatten_mode(raise,lower),origin->_vec,math::degrees(angle),math::degrees(orientation));
}

void noggit::scripting::script_context::blur_terrain(vecptr pos, float remain, float radius, int brush_type, bool lower, bool raise)
{
    _world->blurTerrain(pos->_vec,remain,radius,brush_type,flatten_mode(raise,lower));
}

void noggit::scripting::script_context::erase_textures(vecptr pos)
{
    _world->eraseTextures(pos->_vec);
}

void noggit::scripting::script_context::clear_shadows(vecptr pos)
{
    _world->clear_shadows(pos->_vec);
}

void noggit::scripting::script_context::clear_textures(vecptr pos)
{
    _world->clearTextures(pos->_vec);
}

void noggit::scripting::script_context::clear_height(vecptr pos)
{
    _world->clearHeight(pos->_vec);
}

void noggit::scripting::script_context::set_hole(vecptr pos, bool big, bool hole)
{
    _world->setHole(pos->_vec,big,hole);
}

void noggit::scripting::script_context::set_hole_adt(vecptr pos, bool hole)
{
    _world->setHoleADT(pos->_vec,hole);
}

void noggit::scripting::script_context::update_vertices()
{
    _world->updateVertexCenter();
    _world->updateSelectedVertices();
}

void noggit::scripting::script_context::deselect_vertices_radius(vecptr pos, float radius)
{
    _world->deselectVertices(pos->_vec,radius);
}

void noggit::scripting::script_context::move_vertices(float h)
{
    _world->moveVertices(h);
}

void noggit::scripting::script_context::flatten_vertices(float h)
{
    _world->flattenVertices(h);
}

void noggit::scripting::script_context::clear_vertex_selection()
{
    _world->clearVertexSelection();
}

void noggit::scripting::script_context::paint_texture(vecptr pos, float strength, float pressure, float hardness, float radius, std::string texture)
{
    auto brush = Brush();
    brush.setHardness(hardness);
    brush.setRadius(radius);
    _world->paintTexture(pos->_vec,&brush,strength,pressure,scoped_blp_texture_reference(texture));
}

bool noggit::scripting::script_context::holding_alt() 
{ 
    return _holding_alt; 
}
bool noggit::scripting::script_context::holding_shift() 
{ 
    return _holding_shift; 
}
bool noggit::scripting::script_context::holding_ctrl() 
{
    return _holding_ctrl; 
}
bool noggit::scripting::script_context::holding_space() 
{ 
    return _holding_space; 
}

std::shared_ptr<script_vec> script_context::pos() 
{ 
    return std::make_shared<script_vec>(_pos); 
}

/** registry */
void noggit::scripting::register_context_functions(duk_context *ctx)
{
    GLUE_METHOD(ctx, script_context, update_vertices);
    GLUE_METHOD(ctx, script_context, pos);
    GLUE_METHOD(ctx, script_context, change_terrain);
    GLUE_METHOD(ctx, script_context, add_m2);
    GLUE_METHOD(ctx, script_context, add_wmo);
    GLUE_METHOD(ctx, script_context, get_map_id);
    GLUE_METHOD(ctx, script_context, get_area_id);
    GLUE_METHOD(ctx, script_context, set_area_id);
    GLUE_METHOD(ctx, script_context, change_vertex_color);
    GLUE_METHOD(ctx, script_context, get_vertex_color);
    GLUE_METHOD(ctx, script_context, flatten_terrain);
    GLUE_METHOD(ctx, script_context, blur_terrain);
    GLUE_METHOD(ctx, script_context, erase_textures);
    GLUE_METHOD(ctx, script_context, clear_shadows);
    GLUE_METHOD(ctx, script_context, clear_textures);
    GLUE_METHOD(ctx, script_context, clear_height);
    GLUE_METHOD(ctx, script_context, set_hole);
    GLUE_METHOD(ctx, script_context, set_hole_adt);
    GLUE_METHOD(ctx, script_context, deselect_vertices_radius);
    GLUE_METHOD(ctx, script_context, clear_vertex_selection);
    GLUE_METHOD(ctx, script_context, move_vertices);
    GLUE_METHOD(ctx, script_context, flatten_vertices);
    GLUE_METHOD(ctx, script_context, cam_pitch);
    GLUE_METHOD(ctx, script_context, cam_yaw);

    GLUE_METHOD(ctx, script_context, outer_radius);
    GLUE_METHOD(ctx, script_context, inner_radius);
    GLUE_METHOD(ctx, script_context, select);

    GLUE_METHOD(ctx, script_context, holding_alt);
    GLUE_METHOD(ctx, script_context, holding_shift);
    GLUE_METHOD(ctx, script_context, holding_ctrl);
    GLUE_METHOD(ctx, script_context, holding_space);
    GLUE_METHOD(ctx, script_context, paint_texture);
}