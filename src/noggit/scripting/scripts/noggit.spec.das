/**
 * Adds a "string list" (dropdown menu) parameter to the script window
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

/**
 * Adds a string parameter to the script window
 */
[export]
def add_string_param(name: string, value: string)
    pass
    
/**
 * Adds an integer parameter to the script window
 */
[export]
def add_int_param(name: string, min: int, max: int, default: int)
    pass

/**
 * Adds a double parameter to the script window
 */
[export]
def add_double_param(name: string, min: double, max: double, default: double, zeros: int)
    pass

/**
 * Adds a float parameter to the script window
 */
[export]
def add_float_param(name: string, min: double, max: double, default: double, zeros: int)
    pass

/**
 * Adds a boolean parameter to the script window
 */
[export]
def add_bool_param(name: string, default: bool)
    pass

/**
 * Returns the currently selected string parameter in a parameter list.
 */
[export]
def get_string_list_param(name: string): string
    pass

/**
 * Reads a string parameter from the script window
 * 
 * @throws if name is not a valid parameter
 */
[export]
def get_string_param(name: string): string
    pass

/**
 * Reads an integer parameter from the script window
 *
 * @throws if name is not a valid parameter
 */
[export]
def get_int_param(name: string): int
    pass

/**
 * Reads a double parameter from the script window
 *
 * @throws if name is not a valid parameter
 */
[export]
def get_double_param(name: string): float
    pass

/**
 * Reads a float parameter from the script window
 *
 * @throws if name is not a valid parameter
 */
[export]
def get_float_param(name: string): float
    pass

/**
 * Reads a boolean parameter from the script window
 *
 * @throws if name is not a valid parameter
 */
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

/**
 * Uses the standard Noggit brush functions to deselect vertices
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_deselect_vertices(pos: vector_3d, float radius);
    pass

/**
 * Uses the standard Noggit brush functions to deselect all vertices
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_clear_vertex_selection();
    pass

/**
 * Uses the standard Noggit brush functions to move move selected vertices vertically.
 *
 * @see brush_select_vertices
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_move_vertices(h: float);
    pass

/**
 * Uses the standard Noggit brush functions to deselect all vertices
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_flatten_vertices(h: float);
    pass

/**
 * Uses the standard Noggit brush functions to update changes to selected vertices.
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
[export]
def brush_update_vertices();
    pass

/**
 * Uses the standard Noggit brush functions to paint texture alphamaps.
 *
 * Brush functions are very inefficient if called more than once per frame,
 * for that, consider using a script_selector instead.
 */
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

/**
 * Writes a string to a file
 */
[export]
def write_file(path: string, content: string)
    pass

/**
 * Appends a string to a file
 */
[export]
def append_file(path: string, content: string)
    pass

/**
 * Reads a file into a string
 */
[export]
def read_file(path: string): string
    pass

/**
 * Returns true if a filepath exists
 */
[export]
def path_exists(path: string): bool
    pass

// image.hpp

/**
 * Returns the pixel value (RGBA) at a specified location in an image
 */
[export]
def img_get_pixel(img: image,x: image, y: image): uint
    pass

/**
 * Sets the pixel value (RGBA) at a specified location in an image.
 */
[export]
def img_set_pixel(img: image,x: int, y: int, value: uint)
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
def img_gradient_scale(img: image,scale: float)

/**
 * Writes an image to the file system
 */
[export]
def img_save(img: image,filename: string): int
    pass

/**
 * Returns the width of an image
 */
[export]
def width(img: image): int
    pass

/**
 * Returns the height of an image
 */
[export]
def height(img: image): int
    pass

/**
 * Loads an image from the file system
 */
[export]
def load_png(path: string): image
    pass

/**
 * Creates a new empty image object
 */
[export]
def create_image(width: int, height: int): image
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

/**
 * Compares the distance between two vectors. 
 * Ignores the 'y' axis.
 * 
 * @returns
 * - 1 = vector distance is larger
 * - 0 = vector distance is equal
 * - -1 = vector distance is smaller
 */
[export]
def dist_2d_compare(from: vector_3d, to: vector_3d, distance: float): int
    pass

/**
 * Rotates a point around an origin point. Uses degrees, as most Noggit functions do.
 */
[export]
def rotate_2d(point: vector_3d, origin: vector_3d, degrees: float): vector_3d
    pass

// script_noise.hpp

/**
 * Gets the value of this noise in global space
 */
[export]
def noise_get(noise: noisemap, pos: vector_3d): float
    pass

/**
 * Checks if the noise point is the highest within a given radius. Uses global coordinates
 */
[export]
def noise_is_highest(noise: noisemap, pos: vector_3d, check_radius: int)
    pass

/**
 * Returns the starting coordinates of a noise map. Uses x/z, y is always 0.
 */
[export]
def noise_start(noise: noisemap): vector_3d
    pass

/**
 * Returns the width of a noise map
 */
[export]
def width(noise: noisemap): int
    pass

/**
 * Returns the height of a noise map
 */
[export]
def height(noise: noisemap): int
    pass

[export]
def make_noise_size(start_x: int, start_y: int, width: int, height: int, frequency: float, algorithm: string, seed: string): noisemap
    pass

[export]
def make_noise_selection(sel: selection, frequency: float, algorithm: string, seed: string): noisemap
    pass

/**
 * Fills a noisemap
 */
[export]
def noise_fill(thiz: script_noise_generator, map: script_map,seed: string, x: int, y: int, xSize: int, ySize: int, frequency: float): noisemap
    pass

/**
 * Fills a noise_map
 * @param padding: How many extra cells should be added around the entire selection
 */
[export]
def noise_fill_selection(generator: script_noise_generator, map: script_map, sel: selection,seed: string, frequency: float, padding: int): noisemap
    pass

[export]
def rand_int32(rand: random, low: int, high: int): int
    pass

[export]
def rand_float(rand: random, low: float, high: float): float
    pass

[export]
def random_from_seed(seed: string): random
    pass

[export]
def random_from_time(): random
    pass

[export]
def select_origin(origin: vector_3d, x_radius: float, z_radius: float): selection
    pass

[export]
def select_between(pos1: vector_3d, pos2: vector_3d): selection
    pass

[export]
def sel_next_chunk(selection &sel): bool
    pass

[export]
def sel_get_chunk(selection &sel): chunk
    pass

[export]
def sel_reset_chunk_itr(selection &sel)
    pass

[export]
def sel_next_model(selection &sel): bool
    pass

[export]
def sel_get_model(selection &sel): model
    pass

[export]
def sel_reset_model_itr(selection &sel)
    pass

[export]
def sel_requery_models(selection &sel)
    pass

[export]
def sel_center(selection &sel): vector_3d
    pass

[export]
def sel_min(selection &sel): vector_3d
    pass

[export]
def sel_max(selection &sel): vector_3d
    pass

[export]
def sel_size(selection &sel): vector_3d
    pass

[export]
def chunk_set_hole(chunk &chunk, hole: bool)
    pass

[export]
def chunk_remove_texture(chunk: chunk, index: int)
    pass

[export]
def chunk_get_texture(chunk: chunk, int index): string
    pass

[export]
def chunk_add_texture(chunk: chunk, texture: string): int
    pass

[export]
def chunk_clear_textures(chunk &chunk)
    pass

[export]
def chunk_apply_textures(chunk &chunk)
    pass

[export]
def chunk_apply_heightmap(chunk &chunk)
    pass

[export]
def chunk_apply_vertex_color(chunk &chunk)
    pass

[export]
def chunk_apply_all(chunk &chunk)
    pass

[export]
def chunk_set_impassable(chunk &chunk, bool add)
    pass

[export]
def chunk_get_area_id(chunk &chunk): int
    pass

[export]
def chunk_set_area_id(chunk &chunk, int value)
    pass

[export]
def chunk_next_vert(chunk &chunk): bool
    pass

[export]
def chunk_next_tex(chunk &chunk): bool
    pass

[export]
def chunk_reset_vert_itr(chunk &chunk)
    pass

[export]
def chunk_reset_tex_itr(chunk &chunk)
    pass

[export]
def chunk_get_vert(chunk &chunk): vert
    pass

[export]
def chunk_get_tex(chunk &chunk): vert
    pass

[export]
def chunk_clear_colors(chunk &chunk)
    pass

[export]
def vert_get_pos(vert& vert): vector_3d
    pass

[export]
def vert_set_height(vert& vert,float y)
    pass

[export]
def vert_add_height(vert& vert,float y)
    pass

[export]
def vert_sub_height(vert& vert,float y)
    pass

[export]
def vert_set_color(vert& vert, float r, float g, float b)
    pass

[export]
def vert_set_water(vert& vert, int type, float height)
    pass

[export]
def vert_set_hole(vert& vert, bool add)
    pass

[export]
def vert_set_alpha(vert& vert, int index, float alpha)
    pass

[export]
def vert_get_alpha(vert& vert, int index): float
    pass

[export]
def vert_next_tex(vert& vert): bool
    pass

[export]
def vert_reset_tex(vert& vert)
    pass

[export]
def vert_get_tex(vert& vert): tex
    pass

[export]
def vert_is_water_aligned(vert &chunk): bool
    pass

[export]
def tex_set_alpha(tex& tex, int index, float alpha)
    pass

[export]
def tex_get_alpha(tex& tex, int index): float
    pass

/**
 * Returns the x/z coordinates of this texture unit.
 *
 * The returned y value is always 0.
 */
[export]
def tex_get_pos_2d(tex& tex): float
    pass

[export]
def model_get_pos(model: model): vector_3d
    pass

[export]
def model_set_pos(model: model,pos: vector_3d)
    pass

[export]
def model_get_rot(model: model): vector_3d
    pass

[export]
def model_set_rot(model: model,rot: vector_3d)
    pass

[export]
def model_get_scale(model: model): float
    pass

[export]
def model_set_scale(model: model,scale: float)
    pass

[export]
def model_get_filename(model: model): string
    pass

[export]
def model_replace(model: model,filename: string)
    pass

[export]
def model_remove(model: model)
    pass

[export]
def print(print: string)