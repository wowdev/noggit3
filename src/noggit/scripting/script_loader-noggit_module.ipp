// This file is part of Noggit3, licensed under GNU General Public License (version 3).

// Note: This file should only be included by script_loader.cpp
#pragma once

#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_image.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_filesystem.hpp>
#include <noggit/scripting/script_math.hpp>
#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/script_random.hpp>
#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_brush.hpp>
#include <noggit/scripting/script_misc.hpp>
#include <noggit/scripting/script_chunk.hpp>
#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/script_vert.hpp>
#include <noggit/scripting/script_heap.hpp>
#include <math/vector_3d.hpp>

#include <daScript/daScript.h>

#include <string>

using namespace das;
using namespace noggit::scripting;
using namespace math;

// TODO: Rename this one, it's the odd one
#define FUNC(name, side_effect) \
  addExtern<DAS_BIND_FUN(name)>(*this, lib, #name, SideEffects::side_effect, #name)

#define FUNC_RETVALUE(name, side_effect) \
  addExtern<DAS_BIND_FUN(noggit::scripting::name), SimNode_ExtFuncCallAndCopyOrMove>(*this, lib, #name, SideEffects::side_effect, #name)

#define FUNC_SCOPED(name, side_effect) \
  addExtern<DAS_BIND_FUN(noggit::scripting::name)>(*this, lib, #name, SideEffects::side_effect, #name)

#define FUNC_RENAME(name_in, name_out, side_effect) \
  addExtern<DAS_BIND_FUN(name_in)>(*this, lib, name_out, SideEffects::side_effect, name_out)

#define FIELD(name) \
  addField<DAS_BIND_MANAGED_FIELD(name)>(#name);

#define CLASS(name, ...)                                                         \
  MAKE_TYPE_FACTORY(name, name);                                                 \
  struct name##_annotation : public ManagedStructureAnnotation<name, true, true> \
  {                                                                              \
    name##_annotation(ModuleLibrary &ml) : ManagedStructureAnnotation(#name, ml)  \
    {                                                                            \
      __VA_ARGS__                                                                \
    }                                                                            \
    virtual bool isLocal() const override { return true; }                       \
    virtual bool canCopy() const override { return true; }                       \
    /* These three enable use in arrays and maps */                              \
    virtual bool hasNonTrivialCtor() const override { return false; }            \
    virtual bool hasNonTrivialDtor() const override { return false; }            \
    virtual bool hasNonTrivialCopy() const override { return false; }            \
  };

#define CLASS_TEMPLATE(name, template, regname, ...)                                                     \
  MAKE_TYPE_FACTORY(name<template>, name<template>);                                                     \
  struct name##_##template##_annotation : public ManagedStructureAnnotation<name<template>, true, true>{ \
    name##_##template##_annotation(ModuleLibrary & ml) : ManagedStructureAnnotation(#regname, ml){       \
        __VA_ARGS__} virtual bool isLocal() const override{return true;                                  \
  }                                                                                                      \
  virtual bool canCopy() const override { return true; }                                                 \
  virtual bool canMove() const override { return true; }                                                 \
  }                                                                                                      \
  ;

#define CLASS_ANNOTATION(name) \
  addAnnotation(make_smart<name##_annotation>(lib));

// Classes
CLASS(vector_3d, FIELD(x) FIELD(y) FIELD(z))
CLASS(image, FIELD(_image))
CLASS(noisemap)
CLASS(chunk)
CLASS(vert)
CLASS(tex)
CLASS(model)
CLASS(model_iterator)
CLASS(random)
CLASS(selection)

class NoggitModule : public Module
{
public:
  NoggitModule() : Module("noggit")
  {
    ModuleLibrary lib;
    lib.addModule(this);
    lib.addBuiltInModule();

    // class annotations
    CLASS_ANNOTATION(vector_3d);
    CLASS_ANNOTATION(image);
    CLASS_ANNOTATION(random);
    CLASS_ANNOTATION(selection);
    CLASS_ANNOTATION(noisemap);
    CLASS_ANNOTATION(chunk);
    CLASS_ANNOTATION(vert);
    CLASS_ANNOTATION(tex);
    CLASS_ANNOTATION(model);
    CLASS_ANNOTATION(model_iterator);

    // script_random.hpp
    FUNC_RETVALUE(random_from_seed, worstDefault);
    FUNC_RETVALUE(random_from_time, worstDefault);
    FUNC_SCOPED(rand_int32, worstDefault);
    FUNC_SCOPED(rand_uint32, worstDefault);
    FUNC_SCOPED(rand_double, worstDefault);
    FUNC_SCOPED(rand_float, worstDefault);

    // script_noise.hpp
    FUNC_SCOPED(noise_get, worstDefault);
    FUNC_SCOPED(noise_set, worstDefault);
    FUNC_RETVALUE(noise_start, worstDefault);
    FUNC_RENAME(noggit::scripting::noise_width, "width", worstDefault);
    FUNC_RENAME(noggit::scripting::noise_height, "height", worstDefault);
    FUNC_SCOPED(noise_is_highest, worstDefault);
    FUNC_RETVALUE(make_noise_size, worstDefault);
    FUNC_RETVALUE(make_noise_selection, worstDefault);

    // script_image.hpp
    FUNC_SCOPED(img_get_index, worstDefault);
    FUNC_SCOPED(img_get_pixel, worstDefault);
    FUNC_SCOPED(img_set_pixel, worstDefault);
    FUNC_SCOPED(img_gradient_scale, worstDefault);
    FUNC_SCOPED(img_save, worstDefault);
    FUNC_RENAME(noggit::scripting::img_width, "width", worstDefault);
    FUNC_RENAME(noggit::scripting::img_height, "height", worstDefault);
    FUNC_RETVALUE(create_image, worstDefault);
    FUNC_RETVALUE(load_png, worstDefault);

    FUNC_SCOPED(write_file, worstDefault);
    FUNC_SCOPED(append_file, worstDefault);
    FUNC_SCOPED(read_file, worstDefault);
    FUNC_SCOPED(path_exists, worstDefault);

    // script_selections.hpp
    FUNC_RETVALUE(select_origin, worstDefault);
    FUNC_RETVALUE(select_between, worstDefault);
    FUNC_SCOPED(sel_next_chunk, worstDefault);
    FUNC_RETVALUE(sel_get_chunk, worstDefault);
    FUNC_SCOPED(sel_reset_chunk_itr, worstDefault);
    FUNC_SCOPED(sel_next_model, worstDefault);
    FUNC_RETVALUE(sel_get_model, worstDefault);
    FUNC_SCOPED(sel_reset_model_itr, worstDefault);
    FUNC_SCOPED(sel_requery_models, worstDefault);
    FUNC_RETVALUE(sel_center, worstDefault);
    FUNC_RETVALUE(sel_min, worstDefault);
    FUNC_RETVALUE(sel_max, worstDefault);
    FUNC_RETVALUE(sel_size, worstDefault);

    FUNC_SCOPED(chunk_set_hole, worstDefault);
    FUNC_SCOPED(chunk_remove_texture, worstDefault);
    FUNC_SCOPED(chunk_get_texture, worstDefault);
    FUNC_SCOPED(chunk_add_texture, worstDefault);
    FUNC_SCOPED(chunk_clear_textures, worstDefault);
    FUNC_SCOPED(chunk_apply_textures, worstDefault);
    FUNC_SCOPED(chunk_apply_heightmap, worstDefault);
    FUNC_SCOPED(chunk_apply_vertex_color, worstDefault);
    FUNC_SCOPED(chunk_apply_all, worstDefault);
    FUNC_SCOPED(chunk_set_impassable, worstDefault);
    FUNC_SCOPED(chunk_get_area_id, worstDefault);
    FUNC_SCOPED(chunk_set_area_id, worstDefault);
    FUNC_SCOPED(chunk_next_vert, worstDefault);
    FUNC_SCOPED(chunk_next_tex, worstDefault);
    FUNC_SCOPED(chunk_reset_vert_itr, worstDefault);
    FUNC_SCOPED(chunk_reset_tex_itr, worstDefault);
    FUNC_SCOPED(chunk_clear_colors, worstDefault);
    FUNC_RETVALUE(chunk_get_vert, worstDefault);
    FUNC_RETVALUE(chunk_get_tex, worstDefault);

    FUNC_RETVALUE(vert_get_pos, worstDefault);
    FUNC_SCOPED(vert_set_height, worstDefault);
    FUNC_SCOPED(vert_add_height, worstDefault);
    FUNC_SCOPED(vert_sub_height, worstDefault);
    FUNC_SCOPED(vert_set_color, worstDefault);
    FUNC_SCOPED(vert_set_water, worstDefault);
    FUNC_SCOPED(vert_set_hole, worstDefault);
    FUNC_SCOPED(vert_set_alpha, worstDefault);
    FUNC_SCOPED(vert_get_alpha, worstDefault);
    FUNC_SCOPED(vert_next_tex, worstDefault);
    FUNC_SCOPED(vert_reset_tex, worstDefault);
    FUNC_RETVALUE(vert_get_tex, worstDefault);
    FUNC_SCOPED(vert_is_water_aligned, worstDefault);
    FUNC_SCOPED(tex_set_alpha, worstDefault);
    FUNC_SCOPED(tex_get_alpha, worstDefault);
    FUNC_RETVALUE(tex_get_pos_2d, worstDefault);

    FUNC_RETVALUE(model_get_pos, worstDefault);
    FUNC_SCOPED(model_set_pos, worstDefault);
    FUNC_RETVALUE(model_get_rot, worstDefault);
    FUNC_SCOPED(model_set_rot, worstDefault);
    FUNC_SCOPED(model_get_scale, worstDefault);
    FUNC_SCOPED(model_set_scale, worstDefault);
    FUNC_SCOPED(model_get_uid, worstDefault);
    FUNC_SCOPED(model_remove, worstDefault);
    FUNC_SCOPED(model_get_filename, worstDefault);
    FUNC_SCOPED(model_replace, worstDefault);

    // script_math.hpp
    FUNC_SCOPED(round, worstDefault);
    FUNC_SCOPED(pow, worstDefault);
    FUNC_SCOPED(log10, worstDefault);
    FUNC_SCOPED(log, worstDefault);
    FUNC_SCOPED(ceil, worstDefault);
    FUNC_SCOPED(floor, worstDefault);
    FUNC_SCOPED(exp, worstDefault);
    FUNC_SCOPED(cbrt, worstDefault);
    FUNC_SCOPED(acosh, worstDefault);
    FUNC_SCOPED(asinh, worstDefault);
    FUNC_SCOPED(atanh, worstDefault);
    FUNC_SCOPED(cosh, worstDefault);
    FUNC_SCOPED(sinh, worstDefault);
    FUNC_SCOPED(tanh, worstDefault);
    FUNC_SCOPED(acos, worstDefault);
    FUNC_SCOPED(asin, worstDefault);
    FUNC_SCOPED(atan, worstDefault);
    FUNC_SCOPED(cos, worstDefault);
    FUNC_SCOPED(sin, worstDefault);
    FUNC_SCOPED(tan, worstDefault);
    FUNC_SCOPED(sqrt, worstDefault);
    FUNC_SCOPED(abs, worstDefault);
    FUNC_SCOPED(lerp, worstDefault);
    FUNC_SCOPED(dist_2d, worstDefault);
    FUNC_SCOPED(dist_2d_compare, worstDefault);
    FUNC_RETVALUE(rotate_2d, worstDefault);

    // script_context.hpp
    FUNC_RETVALUE(pos, worstDefault);
    FUNC_RETVALUE(vec, worstDefault);
    FUNC_SCOPED(brush_change_terrain, worstDefault);
    FUNC_SCOPED(add_m2, worstDefault);
    FUNC_SCOPED(add_wmo, worstDefault);
    FUNC_SCOPED(get_map_id, worstDefault);
    FUNC_SCOPED(get_area_id, worstDefault);
    FUNC_SCOPED(brush_set_area_id, worstDefault);
    FUNC_SCOPED(brush_change_vertex_color, worstDefault);
    FUNC_RETVALUE(brush_get_vertex_color, worstDefault);
    FUNC_SCOPED(brush_flatten_terrain, worstDefault);
    FUNC_SCOPED(brush_blur_terrain, worstDefault);
    FUNC_SCOPED(brush_erase_textures, worstDefault);
    FUNC_SCOPED(brush_clear_shadows, worstDefault);
    FUNC_SCOPED(brush_clear_textures, worstDefault);
    FUNC_SCOPED(brush_clear_height, worstDefault);
    FUNC_SCOPED(brush_set_hole, worstDefault);
    FUNC_SCOPED(brush_set_hole_adt, worstDefault);
    FUNC_SCOPED(brush_deselect_vertices, worstDefault);
    FUNC_SCOPED(brush_clear_vertex_selection, worstDefault);
    FUNC_SCOPED(brush_move_vertices, worstDefault);
    FUNC_SCOPED(brush_flatten_vertices, worstDefault);
    FUNC_SCOPED(brush_update_vertices, worstDefault);
    FUNC_SCOPED(brush_paint_texture, worstDefault);
    FUNC_SCOPED(cam_pitch, worstDefault);
    FUNC_SCOPED(cam_yaw, worstDefault);
    FUNC_SCOPED(outer_radius, worstDefault);
    FUNC_SCOPED(inner_radius, worstDefault);
    FUNC_SCOPED(holding_alt, worstDefault);
    FUNC_SCOPED(holding_shift, worstDefault);
    FUNC_SCOPED(holding_ctrl, worstDefault);
    FUNC_SCOPED(holding_space, worstDefault);
    FUNC_SCOPED(dt, worstDefault);

    // scripting_tool.hpp
    FUNC_SCOPED(get_string_param, worstDefault);
    FUNC_SCOPED(get_string_list_param, worstDefault);
    FUNC_SCOPED(get_int_param, worstDefault);
    FUNC_SCOPED(get_double_param, worstDefault);
    FUNC_SCOPED(get_bool_param, worstDefault);
    FUNC_SCOPED(add_string_param, worstDefault);
    FUNC_SCOPED(add_int_param, worstDefault);
    FUNC_SCOPED(add_double_param, worstDefault);
    FUNC_SCOPED(add_float_param, worstDefault);
    FUNC_SCOPED(get_float_param, worstDefault);
    FUNC_SCOPED(add_bool_param, worstDefault);
    FUNC_SCOPED(add_string_list_param, worstDefault);
    FUNC_SCOPED(add_description, worstDefault);

    // script_heap (for debugging, remove this later)
    FUNC_SCOPED(script_heap_read_byte, worstDefault);
    FUNC_SCOPED(script_heap_write_byte, worstDefault);
    FUNC_SCOPED(script_calloc, worstDefault);
    FUNC_SCOPED(overlaps, worstDefault);
  }
};

REGISTER_MODULE(NoggitModule);
