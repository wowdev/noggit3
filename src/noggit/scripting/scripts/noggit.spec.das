
// scripting_tool.hpp

/**
 * Adds a "string list" (dropdown menu) parameter
 * 
 * As opposed to other parameter types, you can call this
 * function multiple times with the same name and different values
 * to add more entries to the dropdown menu.
 */
[export]
def add_string_list_param(name: string, value: string)
    pass

/**
 * Adds a line to the description section of the brush window.
 */
[export]
def add_description(name: string, value: string)
    pass

[export]
def add_string_param(name: string, value: string)
    pass
    
[export]
def add_int_param(name: string, min: int, max: int, default: int)
    pass

[export]
def add_double_param(name: string, min: double, max: double, default: double, zeros: int)
    pass

[export]
def add_float_param(name: string, min: double, max: double, default: double, zeros: int)
    pass

[export]
def add_bool_param(name: string, default: bool)
    pass

/**
 * Returns the currently selected string parameter in a parameter list.
 */
[export]
def get_string_list_param(name: string): string
    pass

[export]
def get_string_param(name: string): string
    pass

[export]
def get_int_param(name: string): int
    pass

[export]
def get_double_param(name: string): float
    pass

[export]
def get_float_param(name: string): float
    pass

[export]
def get_bool_param(name: string): float
    pass

// script_context.hpp

/**
 * Returns the position of the click that caused the current event
 */
[export]
def pos(): vector_3d

/**
 * Selects a part of the world to modify individual chunks, vertices and texture points.
 */
[export]
def world_select(origin: vector_3d, float, inner_radius: float, outer_radius: float): math::vector_3d)
    pass

/**
 * Uses the standard Noggit brush function to change terrain.
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_change_terrain(pos: vector_3d, change: float, radius: float, inner_radius: float, brush_type: int);
    pass

/**
 * Adds a .m2 model to the world. Use this function if your 
 * models name ends with .m2
 */
[export]
def add_m2(filename: string, pos: vector_3d, scale: float, rotation: vector_3d);
    pass

/**
 * Adds a .wmo model to the world. Use this function if yur
 * models name ends with .wmo
 */
[export]
def add_wmo(filename: string, pos: vector_3d, rotation: vector_3d)
    pass

/**
 * Returns the current map id
 */
[export]
def get_map_id(): int
    pass

/**
 * Returns the area id at a specific vector location
 */
[export]
def get_area_id(vector_3d: int): int
    pass

/**
 * Uses the standard Noggit brush function to change the area ID.
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_set_area_id(pos: vector_3d, id: int, adt : bool)
    pass

/**
 * Uses the standard Noggit brush function to change vertex colors
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_change_vertex_color(pos: vector_3d, color: vector_3d, alpha: float, change: float, radius: float, editMode: bool)
    pass

/**
 * Uses the standard Noggit brush function to fetch vertex color.
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_get_vertex_color(pos: vector_3d) : vector_3d
    pass

/**
 * Uses the standard Noggit brush function to flatten terrain.
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_flatten_terrain(pos: vector_3d, remain: float, radius: float, brush_type: int, lower: bool, raise: bool, origin: vector_3d, angle: double, orientation: double);
    pass

/**
 * Uses the standard Noggit brush function to blur terrain
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_blur_terrain(pos: vector_3d, remain: float, radius: float, brush_type: int, lower: bool, raise: bool);
    pass

/**
 * Uses the standard Noggit brush function to erase textures
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_erase_textures(pos: vector_3d);
    pass
    

/**
 * Uses the standard Noggit brush function to remove shadows
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_clear_shadows(pos: vector_3d);
    pass

/**
 * Uses the standard Noggit brush function to remove textures
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_clear_textures(pos: vector_3d);
    pass

/**
 * Uses the standard Noggit brush function to reset height to 0.
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_clear_height(pos: vector_3d);
    pass

/**
 * Uses the standard Noggit brush function to create or remove holes
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_set_hole(pos: vector_3d, big: bool, hole: bool);
    pass

[export]
def brush_set_hole_adt(pos: vector_3d, bool hole);
    pass

[export]
def brush_deselect_vertices(pos: vector_3d, float radius);
    pass

[export]
def brush_clear_vertex_selection();
    pass

[export]
def brush_move_vertices(h: float);
    pass

[export]
def brush_flatten_vertices(h: float);
    pass

[export]
def brush_update_vertices();
    pass

[export]
def brush_paint_texture(pos: vector_3d, strength: float, pressure: float, hardness: float, radius: float, texture: string)
    pass

/**
 * Returns the pitch rotation of the camera
 */
[export]
def cam_pitch(): float
    pass

/**
 * Returns the yaw rotation of the camera
 */
[export]
def cam_yaw(): float
    pass

/**
 * Returns the currently selected outer radius in the script menu.
 */
[export]
def outer_radius(): float
    pass

/**
 * Returns the currently selected inner radius in the script menu.
 */
[export]
def inner_radius(): float
    pass

/**
 * Returns true if the alt key is currently being pressed.
 */
[export]
def holding_alt(): bool;
    pass

/**
 * Returns true if the shift key is currently being pressed.
 */
[export]
def holding_shift(): bool
    pass

/**
 * Returns true if the ctrl key is currently being pressed.
 */
[export]
def holding_ctrl(): bool
    pass

/**
 * Returns true if the spacebar key is currently being pressed.
 */
[export]
def holding_space(): bool
    pass

// script_filesystem.hpp


[export]
def file_itr_next(thiz: script_file_iterator): bool
    pass

[export]
def file_itr_get(thiz: script_file_iterator): bool
    pass

[export]
def write_file(path: string, content: string)
    pass

[export]
def append_file(path: string, content: string)
    pass

[export]
def read_directory(directory: string): script_file_iterator
    pass

[export]
def read_file(path: string): string
    pass

[export]
def path_exists(path: string): bool
    pass

// script_image.hpp
[export]
def img_get_index(img: script_image,x: int, y: int): int
    pass

[export]
def img_get_pixel(img: script_image,x: script_image, y: script_image): uint
    pass

[export]
def img_set_pixel(img: script_image,x: int, y: int, value: uint)
    pass

/**
 * Reads a gradient from an image.
 *
 * Gradient images are assumed to be laid out left->right
 * with black values representing 0 and white values representing 1.
 * 
 * You can create gradients with something like gimp or photoshop.
 * Larger images yield better results, but height is irrelevant.
 */
[export]
def img_gradient_scale(img: script_image,scale: float)

[export]
def img_save(img: script_image,filename: string): int
    pass

[export]
def img_width(img: script_image): int
    pass

[export]
def img_height(img: script_image): int
    pass

[export]
def img_resize(img: script_image, width: uint, height: uint): script_image
    pass

[export]
def img_load_png(img: script_image, path: string)
    pass

[export]
def create_image(): script_image
    pass

[export]
def round(a1: float): float
    pass

[export]
def pow(a1: float, a2: float): float
    pass

[export]
def log10(arg: float): float
    pass

[export]
def log(arg: float): float
    pass

[export]
def ceil(arg: float): float
    pass

[export]
def floor(arg: float): float
    pass

[export]
def exp(arg: float): float
    pass

[export]
def cbrt(arg: float): float
    pass

[export]
def acosh(arg: float): float
    pass

[export]
def asinh(arg: float): float
    pass

[export]
def atanh(arg: float): float
    pass

[export]
def cosh(arg: float): float
    pass

[export]
def sinh(arg: float): float
    pass

[export]
def tanh(arg: float): float
    pass

[export]
def acos(arg: float): float
    pass

[export]
def asin(arg: float): float
    pass

[export]
def atan(arg: float): float
    pass

[export]
def cos(arg: float): float
    pass

[export]
def sin(arg: float): float
    pass

[export]
def tan(arg: float): float
    pass

[export]
def sqrt(arg: float): float
    pass

[export]
def abs(arg: float): float
    pass

/**
 * Linear interpolation/fade between two numbers.
 */
[export]
def lerp(from: float, to: float, ratio: float): float
    pass

/**
 * Finds the distance between two vectors.
 *
 * Ignores the 'y' axis.
 */
[export]
def dist_2d(from: vector_3d, to: vector_3d, ratio: float): float
    pass

[export]
def dist_2d_compare(from: vector_3d, to: vector_3d, ratio: float): int
    pass

[export]
def rotate_2d(point: vector_3d, origin: vector_3d, angle: float): vector_3d
    pass

// script_noise.hpp

[export]
def noise_get_index(noise: script_noise_map, x: int, y: int): float
    pass

/**
 * Gets the value of this noise in global space ()
 */
[export]
def noise_get_global(noise: script_noise_map, pos: vector_3d): float
    pass

[export]
def noise_set(noise: script_noise_map, x: int, y: int, value: float)
    pass

[export]
def noise_is_highest_global(noise: script_noise_map, pos: vector_3d, check_radius: int)
    pass

[export]
def noise_start_x(noise: script_noise_map): int
    pass

[export]
def noise_start_y(noise: script_noise_map): int
    pass

[export]
def noise_width(noise: script_noise_map): int
    pass

[export]
def noise_height(noise: script_noise_map): int
    pass

[export]
def make_noisemap(): script_noise_map
    pass

[export]
def noise_fill(thiz: script_noise_generator, map: script_map,seed: string, x: int, y: int, xSize: int, ySize: int, frequency: float): script_noise_map
    pass

/**
 * Fills a noise_map
 * @param padding: How many extra cells should be added around the entire selection
 */
[export]
def noise_fill_selection(generator: script_noise_generator, map: script_map, sel: script_selection,seed: string, frequency: float, padding: int): script_noise_map
    pass

[export]
def make_noisegen_simplex(): script_noise_generator
    pass

[export]
def make_noisegen_perlin(): script_noise_generator
    pass

[export]
def make_noisegen_value(): script_noise_generator
    pass

[export]
def make_noisegen_fractal(): script_noise_generator
    pass

[export]
def make_noisegen_cellular(): script_noise_generator
    pass

[export]
def make_noisegen_white(): script_noise_generator
    pass

[export]
def make_noisegen_custom(encodedNodeTree: string): script_noise_generator
    pass

[export]
def rand_int32(rand: script_random, low: int, high: int): int
    pass

[export]
def rand_float(rand: script_random, low: float, high: float): float
    pass

[export]
def random_from_seed(seed: string): script_random
    pass

[export]
def random_from_time(): script_random
    pass

[export]
def make_selector(): script_selection
    pass

[export]
def select_origin(sel: script_selection, origin: vector_3d, x_radius: float, z_radius: float)
    pass

[export]
def select_between(sel: script_selection, pos1: vector_3d, pos2: vector_3d)
    pass

[export]
def sel_next_chunk(script_selection &sel): bool
    pass

[export]
def sel_get_chunk(script_selection &sel): script_chunk
    pass

[export]
def sel_reset_chunk_itr(script_selection &sel)
    pass

[export]
def sel_next_model(script_selection &sel): bool
    pass

[export]
def sel_get_model(script_selection &sel): script_model
    pass

[export]
def sel_reset_model_itr(script_selection &sel)
    pass

[export]
def sel_requery_models(script_selection &sel)
    pass

[export]
def sel_center(script_selection &sel): vector_3d
    pass

[export]
def sel_min(script_selection &sel): vector_3d
    pass

[export]
def sel_max(script_selection &sel): vector_3d
    pass

[export]
def sel_size(script_selection &sel): vector_3d
    pass

[export]
def chunk_set_hole(script_chunk &chunk, hole: bool)
    pass

[export]
def chunk_remove_texture(chunk: script_chunk, index: int)
    pass

[export]
def chunk_get_texture(chunk: script_chunk, int index): string
    pass

[export]
def chunk_add_texture(chunk: script_chunk, texture: string): int
    pass

[export]
def chunk_clear_textures(script_chunk &chunk)
    pass

[export]
def chunk_apply_textures(script_chunk &chunk)
    pass

[export]
def chunk_apply_heightmap(script_chunk &chunk)
    pass

[export]
def chunk_apply_vertex_color(script_chunk &chunk)
    pass

[export]
def chunk_apply_all(script_chunk &chunk)
    pass

[export]
def chunk_set_impassable(script_chunk &chunk, bool add)
    pass

[export]
def chunk_get_area_id(script_chunk &chunk): int
    pass

[export]
def chunk_set_area_id(script_chunk &chunk, int value)
    pass

[export]
def chunk_next_vert(script_chunk &chunk): bool
    pass

[export]
def chunk_next_tex(script_chunk &chunk): bool
    pass

[export]
def chunk_reset_vert_itr(script_chunk &chunk)
    pass

[export]
def chunk_reset_tex_itr(script_chunk &chunk)
    pass

[export]
def chunk_get_vert(script_chunk &chunk): script_vert
    pass

[export]
def chunk_get_tex(script_chunk &chunk): script_vert
    pass

[export]
def chunk_clear_colors(script_chunk &chunk)
    pass

[export]
def vert_get_pos(script_vert& vert): vector_3d
    pass

[export]
def vert_set_height(script_vert& vert,float y)
    pass

[export]
def vert_add_height(script_vert& vert,float y)
    pass

[export]
def vert_sub_height(script_vert& vert,float y)
    pass

[export]
def vert_set_color(script_vert& vert, float r, float g, float b)
    pass

[export]
def vert_set_water(script_vert& vert, int type, float height)
    pass

[export]
def vert_set_hole(script_vert& vert, bool add)
    pass

[export]
def vert_set_alpha(script_vert& vert, int index, float alpha)
    pass

[export]
def vert_get_alpha(script_vert& vert, int index): float
    pass

[export]
def vert_next_tex(script_vert& vert): bool
    pass

[export]
def vert_reset_tex(script_vert& vert)
    pass

[export]
def vert_get_tex(script_vert& vert): script_tex
    pass

[export]
def vert_is_water_aligned(script_vert &chunk): bool
    pass

[export]
def tex_set_alpha(script_tex& tex, int index, float alpha)
    pass

[export]
def tex_get_alpha(script_tex& tex, int index): float
    pass

/**
 * Returns the x/z coordinates of this texture unit.
 *
 * The returned y value is always 0.
 */
[export]
def tex_get_pos_2d(script_tex& tex): float
    pass

[export]
def model_get_pos(model: script_model): vector_3d
    pass

[export]
def model_set_pos(model: script_model,pos: vector_3d)
    pass

[export]
def model_get_rot(model: script_model): vector_3d
    pass

[export]
def model_set_rot(model: script_model,rot: vector_3d)
    pass

[export]
def model_get_scale(model: script_model): float
    pass

[export]
def model_set_scale(model: script_model,scale: float)
    pass

[export]
def model_get_filename(model: script_model): string
    pass

[export]
def model_set_filename(model: script_model,filename: string)
    pass

[export]
def model_remove(model: script_model)
    pass

[export]
def print(print: string)