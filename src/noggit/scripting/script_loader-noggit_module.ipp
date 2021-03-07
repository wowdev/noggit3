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

#include <noggit/scripting/script_loader-number_hack.ipp>

#include <daScript/daScript.h>

#include <string>

using namespace das;

#define FUNC(name_in, name_out, side_effect) \
  addExtern<DAS_BIND_FUN(name_in)>(*this, lib, name_out, SideEffects::side_effect, name_out)

#define FUNC_RETCLASS(name_in, name_out, side_effect) \
  addExtern<DAS_BIND_FUN(name_in), SimNode_ExtFuncCallAndCopyOrMove>(*this, lib, name_out, SideEffects::side_effect, name_out)

#define FIELD(name) \
  addField<DAS_BIND_MANAGED_FIELD(name)>(#name);

#define CLASS(name_in, name_out, ...)                                                \
  MAKE_TYPE_FACTORY(name_out, name_in);                                                     \
  struct name_out##_annotation : public ManagedStructureAnnotation<name_in, true, true> \
  {                                                                                  \
    name_out##_annotation(ModuleLibrary &ml) : ManagedStructureAnnotation(#name_out, ml) \
    {                                                                                \
      __VA_ARGS__                                                                    \
    }                                                                                \
    virtual bool isLocal() const override { return true; }                           \
    virtual bool canCopy() const override { return true; }                           \
    /* These three enable use in arrays and maps */                                  \
    virtual bool hasNonTrivialCtor() const override { return false; }                \
    virtual bool hasNonTrivialDtor() const override { return false; }                \
    virtual bool hasNonTrivialCopy() const override { return false; }                \
  };

#define CLASS_ANNOTATION(name) \
  addAnnotation(make_smart<name##_annotation>(lib));

/**
 * <Class Registry>
 * - Exposes a class to daScript (allowing functions to return it as a type)
 * 
 * - The registry name MUST match the class name (without the namespace)
 * 
 * - To expose fields to daScript (obj.fieldA, obj.fieldB etc.), use FIELD annotations
 * 
 * - You cannot expose methods.
 *  
 * - Remember to add a CLASS_ANNOTATION (<Class Annotations>)
 */

CLASS(math::vector_3d, vector_3d, FIELD(x) FIELD(y) FIELD(z))
CLASS(noggit::scripting::image, image, FIELD(_image))
CLASS(noggit::scripting::noisemap, noisemap)
CLASS(noggit::scripting::chunk, chunk)
CLASS(noggit::scripting::vert, vert)
CLASS(noggit::scripting::tex, tex)
CLASS(noggit::scripting::model, model)
CLASS(noggit::scripting::model_iterator, model_iterator)
CLASS(noggit::scripting::random, random)
CLASS(noggit::scripting::selection, selection)

class NoggitModule : public Module
{
public:
  NoggitModule() : Module("noggit")
  {
    ModuleLibrary lib;
    lib.addModule(this);
    lib.addBuiltInModule();

    /**
     * <Class Annotations>
     * 
     * - Simply add the non-namespaced name of classes registered with CLASS()
     */
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

    /**
     * <Function Registry>
     * - Used to expose functions to daScript.
     * 
     * - "FUNC" will register a function with a script-side name and a side-effect
     * 
     * - "FUNC_RETCLASS" must be used if your function returns a class (registered with CLASS())
     * 
     * - If your function takes float parameters and you want to also accept ints, 
     *   don't register them here, and see the file "script_loader-number_hack.ipp" instead.
     * 
     * - If unsure, just use "worstDefault" for the side-effect.
     * 
     * - Just like C++, two functions can have the same name if their parameter types differ.
     * 
     * - Please keep the functions organized by file like below.
     */

    // script_random.hpp
    FUNC(noggit::scripting::rand_int32, "rand_int32", worstDefault);
    FUNC_RETCLASS(noggit::scripting::random_from_seed, "random_from_seed", worstDefault);
    FUNC_RETCLASS(noggit::scripting::random_from_time, "random_from_time", worstDefault);
    FUNC(noggit::scripting::rand_uint32, "rand_uint32", worstDefault);
    FUNC(noggit::scripting::rand_double, "rand_double", worstDefault);

    // script_noise.hpp
    FUNC(noggit::scripting::noise_get, "noise_get", worstDefault);
    FUNC(noggit::scripting::noise_set, "noise_set", worstDefault);
    FUNC_RETCLASS(noggit::scripting::noise_start, "noise_start", worstDefault);
    FUNC(noggit::scripting::noise_width, "width", worstDefault);
    FUNC(noggit::scripting::noise_height, "height", worstDefault);
    FUNC(noggit::scripting::noise_is_highest, "noise_is_highest", worstDefault);

    // script_image.hpp
    FUNC(noggit::scripting::img_get_index, "img_get_index", worstDefault);
    FUNC(noggit::scripting::img_get_pixel, "img_get_pixel", worstDefault);
    FUNC(noggit::scripting::img_set_pixel, "img_set_pixel", worstDefault);
    FUNC(noggit::scripting::img_gradient_scale, "img_gradient_scale", worstDefault);
    FUNC(noggit::scripting::img_save, "img_save", worstDefault);
    FUNC(noggit::scripting::img_width, "width", worstDefault);
    FUNC(noggit::scripting::img_height, "height", worstDefault);
    FUNC_RETCLASS(noggit::scripting::create_image, "create_image", worstDefault);
    FUNC_RETCLASS(noggit::scripting::load_png, "load_png", worstDefault);

    FUNC(noggit::scripting::write_file, "write_file", worstDefault);
    FUNC(noggit::scripting::append_file, "append_file", worstDefault);
    FUNC(noggit::scripting::read_file, "read_file", worstDefault);
    FUNC(noggit::scripting::path_exists, "path_exists", worstDefault);

    // script_selections.hpp
    FUNC_RETCLASS(noggit::scripting::select_between, "select_between", worstDefault);
    FUNC(noggit::scripting::sel_next_chunk, "sel_next_chunk", worstDefault);
    FUNC_RETCLASS(noggit::scripting::sel_get_chunk, "sel_get_chunk", worstDefault);
    FUNC(noggit::scripting::sel_reset_chunk_itr, "sel_reset_chunk_itr", worstDefault);
    FUNC(noggit::scripting::sel_next_model, "sel_next_model", worstDefault);
    FUNC_RETCLASS(noggit::scripting::sel_get_model, "sel_get_model", worstDefault);
    FUNC(noggit::scripting::sel_reset_model_itr, "sel_reset_model_itr", worstDefault);
    FUNC(noggit::scripting::sel_requery_models, "sel_requery_model", worstDefault);
    FUNC_RETCLASS(noggit::scripting::sel_center, "sel_center", worstDefault);
    FUNC_RETCLASS(noggit::scripting::sel_min, "sel_min", worstDefault);
    FUNC_RETCLASS(noggit::scripting::sel_max, "sel_max", worstDefault);
    FUNC_RETCLASS(noggit::scripting::sel_size, "sel_size", worstDefault);

    // script_chunk.hpp
    FUNC(noggit::scripting::chunk_set_hole, "chunk_set_hole", worstDefault);
    FUNC(noggit::scripting::chunk_remove_texture, "chunk_remove_texture", worstDefault);
    FUNC(noggit::scripting::chunk_get_texture, "chunk_get_texture", worstDefault);
    FUNC(noggit::scripting::chunk_add_texture, "chunk_add_texture", worstDefault);
    FUNC(noggit::scripting::chunk_clear_textures, "chunk_clear_textures", worstDefault);
    FUNC(noggit::scripting::chunk_apply_textures, "chunk_apply_textures", worstDefault);
    FUNC(noggit::scripting::chunk_apply_heightmap, "chunk_apply_heightmap", worstDefault);
    FUNC(noggit::scripting::chunk_apply_vertex_color, "chunk_apply_vertex_color", worstDefault);
    FUNC(noggit::scripting::chunk_apply_all, "chunk_apply_all", worstDefault);
    FUNC(noggit::scripting::chunk_set_impassable, "chunk_set_impassable", worstDefault);
    FUNC(noggit::scripting::chunk_get_area_id, "chunk_get_area_id", worstDefault);
    FUNC(noggit::scripting::chunk_set_area_id, "chunk_set_area_id", worstDefault);
    FUNC(noggit::scripting::chunk_next_vert, "chunk_next_vert", worstDefault);
    FUNC(noggit::scripting::chunk_next_tex, "chunk_next_tex", worstDefault);
    FUNC(noggit::scripting::chunk_reset_vert_itr, "chunk_reset_vert_itr", worstDefault);
    FUNC(noggit::scripting::chunk_reset_tex_itr, "chunk_reset_tex_itr", worstDefault);
    FUNC(noggit::scripting::chunk_clear_colors, "chunk_clear_colors", worstDefault);
    FUNC_RETCLASS(noggit::scripting::chunk_get_vert, "chunk_get_vert", worstDefault);
    FUNC_RETCLASS(noggit::scripting::chunk_get_tex, "chunk_get_tex", worstDefault);

    // script_vert.hpp
    FUNC_RETCLASS(noggit::scripting::vert_get_pos, "vert_get_pos", worstDefault);
    FUNC(noggit::scripting::vert_set_hole, "vert_set_hole", worstDefault);
    FUNC(noggit::scripting::vert_get_alpha, "vert_get_alpha", worstDefault);
    FUNC(noggit::scripting::vert_next_tex, "vert_next_tex", worstDefault);
    FUNC(noggit::scripting::vert_reset_tex, "vert_reset_tex", worstDefault);
    FUNC_RETCLASS(noggit::scripting::vert_get_tex, "vert_get_tex", worstDefault);
    FUNC(noggit::scripting::vert_is_water_aligned, "vert_is_water_aligned", worstDefault);
    FUNC(noggit::scripting::tex_get_alpha, "tex_get_alpha", worstDefault);
    FUNC_RETCLASS(noggit::scripting::tex_get_pos_2d, "tex_get_pos_2d", worstDefault);

    // script_model.hpp
    FUNC_RETCLASS(noggit::scripting::model_get_pos, "model_get_pos", worstDefault);
    FUNC(noggit::scripting::model_set_pos, "model_set_pos", worstDefault);
    FUNC_RETCLASS(noggit::scripting::model_get_rot, "model_get_rot", worstDefault);
    FUNC(noggit::scripting::model_set_rot, "model_set_rot", worstDefault);
    FUNC(noggit::scripting::model_get_scale, "model_get_scale", worstDefault);
    FUNC(noggit::scripting::model_get_uid, "model_get_uid", worstDefault);
    FUNC(noggit::scripting::model_remove, "model_remove", worstDefault);
    FUNC(noggit::scripting::model_get_filename, "model_get_filename", worstDefault);
    FUNC(noggit::scripting::model_replace, "model_replace", worstDefault);

    // script_math.hpp
    FUNC(noggit::scripting::round, "round", worstDefault);
    FUNC(noggit::scripting::pow, "pow", worstDefault);
    FUNC(noggit::scripting::log10, "log10", worstDefault);
    FUNC(noggit::scripting::log, "log", worstDefault);
    FUNC(noggit::scripting::ceil, "ceil", worstDefault);
    FUNC(noggit::scripting::floor, "floor", worstDefault);
    FUNC(noggit::scripting::exp, "exp", worstDefault);
    FUNC(noggit::scripting::cbrt, "cbrt", worstDefault);
    FUNC(noggit::scripting::acosh, "acosh", worstDefault);
    FUNC(noggit::scripting::asinh, "asinh", worstDefault);
    FUNC(noggit::scripting::atanh, "atanh", worstDefault);
    FUNC(noggit::scripting::cosh, "cosh", worstDefault);
    FUNC(noggit::scripting::sinh, "sinh", worstDefault);
    FUNC(noggit::scripting::tanh, "tanh", worstDefault);
    FUNC(noggit::scripting::acos, "acos", worstDefault);
    FUNC(noggit::scripting::asin, "asin", worstDefault);
    FUNC(noggit::scripting::atan, "atan", worstDefault);
    FUNC(noggit::scripting::cos, "cos", worstDefault);
    FUNC(noggit::scripting::sin, "sin", worstDefault);
    FUNC(noggit::scripting::tan, "tan", worstDefault);
    FUNC(noggit::scripting::sqrt, "sqrt", worstDefault);
    FUNC(noggit::scripting::dist_2d, "dist_2d", worstDefault);
    FUNC(noggit::scripting::dist_2d_compare, "dist_2d_compare", worstDefault);

    // script_context.hpp
    FUNC_RETCLASS(noggit::scripting::pos, "pos", worstDefault);
    FUNC(noggit::scripting::brush_change_terrain, "brush_change_terrain", worstDefault);
    FUNC(noggit::scripting::add_wmo, "add_wmo", worstDefault);
    FUNC(noggit::scripting::get_map_id, "get_map_id", worstDefault);
    FUNC(noggit::scripting::get_area_id, "get_area_id", worstDefault);
    FUNC(noggit::scripting::brush_set_area_id, "brush_set_area_id", worstDefault);
    FUNC(noggit::scripting::brush_change_vertex_color, "brush_change_vertex_color", worstDefault);
    FUNC_RETCLASS(noggit::scripting::brush_get_vertex_color, "brush_get_vertex_color", worstDefault);
    FUNC(noggit::scripting::brush_flatten_terrain, "brush_flatten_terrain", worstDefault);
    FUNC(noggit::scripting::brush_blur_terrain, "brush_blur_terrain", worstDefault);
    FUNC(noggit::scripting::brush_erase_textures, "brush_erase_texture", worstDefault);
    FUNC(noggit::scripting::brush_clear_shadows, "brush_clear_shadows", worstDefault);
    FUNC(noggit::scripting::brush_clear_textures, "brush_clear_textures", worstDefault);
    FUNC(noggit::scripting::brush_clear_height, "brush_clear_height", worstDefault);
    FUNC(noggit::scripting::brush_set_hole, "brush_set_hole", worstDefault);
    FUNC(noggit::scripting::brush_set_hole_adt, "brush_set_hole_adt", worstDefault);
    FUNC(noggit::scripting::brush_deselect_vertices, "brush_deselect_vertices", worstDefault);
    FUNC(noggit::scripting::brush_clear_vertex_selection, "brush_clear_vertex_selection", worstDefault);
    FUNC(noggit::scripting::brush_move_vertices, "brush_move_vertices", worstDefault);
    FUNC(noggit::scripting::brush_flatten_vertices, "brush_flatten_vertices", worstDefault);
    FUNC(noggit::scripting::brush_update_vertices, "brush_update_vertices", worstDefault);
    FUNC(noggit::scripting::brush_paint_texture, "brush_paint_texture", worstDefault);
    FUNC(noggit::scripting::cam_pitch, "cam_pitch", worstDefault);
    FUNC(noggit::scripting::cam_yaw, "cam_yaw", worstDefault);
    FUNC(noggit::scripting::outer_radius, "outer_radius", worstDefault);
    FUNC(noggit::scripting::inner_radius, "inner_radius", worstDefault);
    FUNC(noggit::scripting::holding_alt, "holding_alt", worstDefault);
    FUNC(noggit::scripting::holding_shift, "holding_shift", worstDefault);
    FUNC(noggit::scripting::holding_ctrl, "holding_ctrl", worstDefault);
    FUNC(noggit::scripting::holding_space, "holding_space", worstDefault);
    FUNC(noggit::scripting::dt, "dt", worstDefault);

    // scripting_tool.hpp
    FUNC(noggit::scripting::get_string_param, "get_string_param", worstDefault);
    FUNC(noggit::scripting::get_string_list_param, "get_string_list_param", worstDefault);
    FUNC(noggit::scripting::get_int_param, "get_int_param", worstDefault);
    FUNC(noggit::scripting::get_bool_param, "get_bool_param", worstDefault);
    FUNC(noggit::scripting::add_string_param, "add_string_param", worstDefault);
    FUNC(noggit::scripting::add_int_param, "add_int_param", worstDefault);
    FUNC(noggit::scripting::get_float_param, "get_float_param", worstDefault);
    FUNC(noggit::scripting::add_bool_param, "add_bool_param", worstDefault);
    FUNC(noggit::scripting::add_string_list_param, "add_string_list_param", worstDefault);
    FUNC(noggit::scripting::add_description, "add_description", worstDefault);

    // script_heap (for debugging, remove this later)
    FUNC(noggit::scripting::script_heap_read_byte, "script_heap_read_byte", worstDefault);
    FUNC(noggit::scripting::script_heap_write_byte, "script_heap_write_byte", worstDefault);
    FUNC(noggit::scripting::script_calloc, "script_calloc", worstDefault);
    FUNC(noggit::scripting::overlaps, "overlaps", worstDefault);

    register_hack_functions(this, lib);
  }
};

REGISTER_MODULE(NoggitModule);