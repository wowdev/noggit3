// This file is part of Noggit3, licensed under GNU General Public License (version 3).

/**
 * This is the documentation for the Noggit scripting API.
 * Functions not connected to a class are global and can be called from
 * anywhere in a script.
 */

// While the language in this file is TypeScript,
// everything here applies to lua as well.

type nil = undefined;

/**
 * Callback functions are unassigned by default, but may be assigned to
 * by the user
 */
type callback<T> = T|nil

/**
 * The type of callback used for brush events.
 * 
 * @note In lua, the first argument becomes "self" argument if using colon notation
 */
type brush_callback = callback<((brush: script_brush, event: script_brush_event) => void)>;

/**
 * Represents a 3-dimensional vector.
 * Most functions do not use the 'y' (height) value at all
 */
declare class vector_3d {
    x: number;
    y: number;
    z: number;
}

/**
 * Represents the event context passed to brush click events.
 */
declare class script_brush_event {
    /**
     * Sets the outer radius in the settings panel for this brush
     * @param value 
     */
    set_outer_radius(value: number);

    /**
     * Sets the outer radius in the settings panel for this brush
     * @param value - should be between 0-1
     */
    set_inner_radius(value: number);

    /**
     * Returns the current outer radius configured in the settings panel
     */
    outer_radius(): number;

    /**
     * Returns the current inner radius configured in the settings panel
     */
    inner_radius(): number;

    /**
     * Returns the delta-time since the last update frame
     */
    dt(): number;

    /**
     * Returns world position of this click event.
     * i.e. the world position where the user clicked, held or released a mouse button
     */
    pos(): vector_3d;
}


/**
 * Represents a script brush in the script window.
 */
declare class script_brush {
    /**
     * Changes the name of this script brush
     * @param name 
     */
    set_name(name: string): void;

    /**
     * Returns the current name of this script brush
     */
    get_name(): string;

    /**
     * Adds an empty tag, typically used to create empty lines in the settings panel
     */
    add_null_tag();

    /**
     * Adds a description row to the brush window
     * @param text 
     */
    add_description(text: string);

    /**
     * Adds an integer tag to the settings panel
     * @param item 
     * @param low 
     * @param high 
     * @param def 
     */
    add_int_tag(item: string, low: number, high: number, def: number): tag<number>;

    /**
     * Adds a real (i.e. float) tag to the settings panel
     * @param item 
     * @param low 
     * @param high 
     * @param def 
     */
    add_real_tag(item: string, low: number, high: number, def: number): tag<number>;

    /**
     * Adds a string tag to the settings panel
     * @param item 
     * @param def 
     */
    add_string_tag(item: string, def: string): tag<string>;

    /**
     * Adds a bool tag to the settings panel
     * @param item
     * @param def
     */
    add_bool_tag(item: string, def: boolean): tag<boolean>;

    /**
     * Adds a dropdown menu to the settings panel
     * @param item 
     * @param values 
     */
    add_string_list_tag(item: string, ...values: string[]): tag<string>;
    
    /**
     * The function to call when the user left clicks 
     * the world with this brush
     */
    on_left_click: brush_callback;

    /**
     * The funciton to call when the user holds the left mouse button 
     * in the world with this brush
     */
    on_left_hold: brush_callback;

    /**
     * The function to call when the user releases the left moues button
     * in the world with this brush
     */
    on_left_release: brush_callback;

    /**
     * The function to call when the user right clicks 
     * the world with this brush
     */
    on_right_click: brush_callback;

    /**
     * The funciton to call when the user holds the right mouse button 
     * in the world with this brush
     */
    on_right_hold: brush_callback;

    /**
     * The function to call when the user releases the right mouse button
     * in the world with this brush
     */
    on_right_release: brush_callback;
}

/**
 * Creates a new script brush
 * @param name 
 */
declare function brush(name: string): script_brush;


/**
 * Represents a chunk in the world
 */
declare class chunk {
    /**
     * Removes a texture layer from this chunk
     * and decreases the texture ids of all higher layers by 1
     * @param index 
     */
    remove_texture(index: number): void;

    /**
     * Returns the name of the texture at the specified layer.
     * @param index 
     */
    get_texture(index: number): string;

    /**
     * Returns the amount of textures on this chunk
     */
    get_texture_count(): number

    /**
     * Adds a new texture at the current topmost layer.
     * 
     * @param texture 
     * @param effect - effect id to add
     * -                -2 (default): does not change effect
     * -                -1: clears current effect index
     * -                0+: change to this effect index
     * @note A chunk can hold at most 4 texture layers.
     * @return texture index added to
     */
    add_texture(texture: string, effect: number): number;

    /**
     * Changes the effect id at a texture layer
     * @param layer 
     * @param effect - effect id to set (-1 to remove effects)
     */
    set_effect(layer: number, effect: number)

    /**
     * Returns the effect id at a texture layer
     * @param layer 
     */
    get_effect(layer: number): number

    /**
     * Removes all texture layers in this chunk
     */
    clear_textures(): void;

    /**
     * Creates or removes a hole in this chunk
     * @param hole 
     */
    set_hole(hole: boolean): void;

    /**
     * Removes all vertex colors in this chunk
     */
    clear_colors(): void;

    /**
     * Applies all changes to texture alphamaps in this chunk
     */
    apply_textures(): void;

    /**
     * Applies all changes to the heightmap in this chunk
     */
    apply_heightmap(): void;

    /**
     * Applies all changes to vertex colors in this chunk
     */
    apply_vertex_color(): void;

    /**
     * Applies all changes in this chunk
     */
    apply_all(): void;

    /**
     * Same as apply_all
     */
    apply(): void;

    /**
     * Sets whether this chunk should be impassable for players or not
     */
    set_impassable(impassable: boolean): void;

    /**
     * Returns the area id of a chunk
     */
    get_area_id(): number;

    /**
     * Changes the area id of a chunk
     * @param value 
     */
    set_area_id(value: number): void;

    /**
     * Returns a selection spanning this chunk
     *
     * @note - iterating will include border vert/texels
     */
    to_selection(): selection;

    /**
     * Returns a texel by index in this chunk
     * @param index valid in range [0-4095]
     */
    get_tex(index: number): tex;

    /**
     * Returns a vertex by index in this chunk
     * @param index valid in range [0-144]
     */
    get_vert(index: number): vert;

    /**
     * Returns true if the water in this chunk has deep/fishable flag data.
     */
    has_render_flags(): bool

    /**
     * Sets the fishable flag for the water in this chunk.
     * If render flag data is not present, it is automatically created.
     *
     * - high is 0 by default
     */
    set_fishable_flag(low: number, high?: number): void;

    /**
     * Returns the lower bits of the fishable flag
     * for the water in this chunk.
     *
     * If chunk has no render data, 0xffffffff is returned
     *
     * @note Only contains the lower 32 bits.
     *       For the higher bits, use get_fishable_flag_high
     */
    get_fishable_flag(): number;

    /**
     * Returns the higher bits of the fishable flag
     * for the water in this chunk.
     *
     * If chunk has no render data, 0xffffffff is returned
     */
    get_fishable_flag_high(): number;

    /**
     * Sets the deep flags for the water in this chunk.
     * If the first bit is set (it is for value=1), emulators typically interpret this
     * to mean fatigue should be applied here.
     *
     * -high is 0 by default.
     */
    set_deep_flag(low: number, high?: number): void;

    /**
     * Returns the lower bits of the deep flag
     * for the water in this chunk.
     *
     * If chunk has no render data, 0 is returned
     *
     * @note Only contains the lower 32 bits.
     *       For the higher bits, use get_fishable_flag_high
     */
    get_deep_flag(): number;

    /**
     * Returns the higher bits of the fishable flag
     * for the water in this chunk.
     *
     * If chunk has no render data, 0 is returned
     */
    get_deep_flag_high(): number;
}

/**
 * Writes text to a file
 * @param file 
 * @param content 
 * 
 * @note This operation REQUIRES explicit permission from the user,
 * or it will throw an error.
 */
declare function write_file(file: string, content: string): void;

/**
 * Appends text to a file
 * @param file 
 * @param content 
 * 
 * @note This operation REQUIRES explicit permission from the user,
 * or it will throw an error.
 */
declare function append_file(file: string, content: string): void;

/**
 * Reads a file from the file system.
 * @param file 
 * @note This operation does NOT require explicit permission from the user.
 */
declare function read_file(file: string): string;

/**
 * Returns true if a pathname exists already
 * @param path 
 */
declare function path_exists(path: string): boolean;

/**
 * Creates a new vector from its components
 * @param x 
 * @param y 
 * @param z 
 */
declare function vec(x: number, y: number, z: number): vector_3d;

/**
 * Returns the current camera position
 */
declare function camera_pos(): vector_3d;

/**
 * Spawns an m2 model in the world.
 * 
 * @param filename 
 * @param pos 
 * @param scale 
 * @param rotation 
 */
declare function add_m2(filename: string
                       , pos: vector_3d
                       , scale: number
                       , rotation: vector_3d
                       ): void

/**
 * Spawns a wmo model in the world.
 * 
 * @param filename 
 * @param pos 
 * @param rot 
 * 
 * @note wmo models cannot be scaled.
 */
declare function add_wmo( filename: string
                        , pos: vector_3d
                        , rot: vector_3d
                        ): void

/**
 * Returns the id of the currently open map
 */
declare function get_map_id(): number

/**
 * Returns the area id at a specific position.
 * 
 * The 'y' value is ignored for this operation.
 */
declare function get_area_id(pos: vector_3d): number

/**
 * Returns the cameras pitch rotation (the one you almost NEVER want)
 */
declare function cam_pitch(): number

/**
 * Returns the cameras yaw rotation (the one you almost ALWAYS want)
 */
declare function cam_yaw(): number

/**
 * Returns true if the user is currently pressing the alt key
 */
declare function holding_alt(): boolean

/**
 * Returns true if the user is currently pressing the shift key
 */
declare function holding_shift(): boolean

/**
 * Returns true if the user is currently pressing the ctrl key
 */
declare function holding_ctrl(): boolean

/**
 * Returns true if the user is currently pressing the spacebar
 */
declare function holding_space(): boolean

/**
 * Returns true if the user is currently pressing the left mouse button
 */
declare function holding_left_mouse(): boolean

/**
 * Returns true if the user is currently pressing the right mouse button
 */
declare function holding_right_mouse(): boolean

/**
 * Prints out a message to the script window.
 * (sometimes, with errors, print messages will be suppressed)
 * @param args 
 */
declare function print(...args: any[])

/**
 * Represents a bitmap image
 * 
 * images can be created from create_image or load_png
 */
declare class image {
    /**
     * Returns the pixel value at a coordinate
     * @param x 
     * @param y 
     */
    get_pixel(x: number, y: number): number;

    /**
     * Returns the blue channel (between 0-1) at an image coordinate
     * @param x 
     * @param y 
     */
    get_blue(x: number, y: number): number;

    /**
     * Returns the alpha channel (between 0-1) at an image coordinate
     * @param x 
     * @param y 
     */
    get_alpha(x: number, y: number): number;

    /**
     * Returns the red channel (between 0-1) at an image coordinate
     * @param x 
     * @param y 
     */
    get_red(x: number, y: number): number;

    /**
     * Returns the green channel (between 0-1) at an image coordinate
     * @param x 
     * @param y 
     */
    get_green(x: number, y: number): number;

    /**
     * Returns the pixel value at a relative horizontal coordinate
     * @param rel horizontal relative position (between 0-1)
     */
    gradient_scale(rel: number): number;

    /**
     * Sets the pixel value at an image coordinate
     * @param x 
     * @param y 
     * @param value 
     */
    set_pixel(x: number, y: number, value: number): void;

    /**
     * Sets the pixel value at an image coordinate
     * @param x
     * @param y
     * @param r - should be between 0-1
     * @param g - should be between 0-1
     * @param b - should be between 0-1
     * @param a - should be between 0-1
     */
    set_pixel_floats(x: number, y: number, r: number, g: number, b: number, a: number): void;

    /**
     * Saves this image to a file
     * @param filename 
     */
    save(filename: string);

    /**
     * Returns the width of this image
     */
    width(): number;

    /**
     * Returns the height of this image
     */
    height(): number;
}

/**
 * Creates a new blank image
 * @param width 
 * @param height 
 */
declare function create_image(width: number, height: number): image;

/**
 * Loads a png file into an in-memory image.
 * @param path 
 */
declare function load_png(path: string): image;

declare function round(a: number);
declare function pow(a1: number, a2: number)
declare function log10(a: number);
declare function log(a: number);
declare function ceil(a: number);
declare function floor(a: number);
declare function exp(a: number);
declare function cbrt(a: number);
declare function acosh(a: number);
declare function asinh(a: number);
declare function atanh(a: number);
declare function cosh(a: number);
declare function sinh(a: number);
declare function tanh(a: number);
declare function acos(a: number);
declare function asin(a: number);
declare function atan(a: number);

declare function cos(a: number);
declare function sin(a: number);
declare function tan(a: number);
declare function sqrt(arg: number);
declare function abs(arg: number);

/**
 * Returns the value at some percentage between two values.
 * 
 * @param from - the minimum range value
 * @param to - the maximum range value
 * @param ratio - the percentage to take (typically between 0-1)
 */
declare function lerp(from: number, to: number, ratio: number): number;

/**
 * Returns the 2d distance (ignoring y) between two vectors
 * @param from 
 * @param to 
 */
declare function dist_2d(from: vector_3d, to: vector_3d);

/**
 * Compares the 2d distance (ignoring y value) between two vectors to a given distance.
 * This operation is significantly faster than manually comparing to the result of dist_2d
 * 
 * @param from 
 * @param to 
 * @param dist 
 */
declare function dist_2d_compare(from: vector_3d, to:vector_3d, dist: number): number

/**
 * Returns a 3d point around an origin, ignoring the y value.
 * @param point 
 * @param origin 
 * @param angle 
 */
declare function rotate_2d(point: vector_3d, origin: vector_3d, angle: number): vector_3d

/**
 * Represents a model in the world. Can represent both an m2 and wmo model.
 */
declare class model {
    get_pos(): vector_3d;
    set_pos(pos: vector_3d): void;
    get_rot(): vector_3d;
    set_rot(pos: vector_3d): void;
    get_scale(): number;
    set_scale(scale: number): void;
    get_uid(): number;
    remove(): void;
    get_filename(): string;
    has_filename(name: string): boolean;
    replace(filename: string);
}

/**
 * Represents a map of floats values, typically use with a 
 * noise generator.
 */
declare class noisemap {
    /**
     * Returns the float value at a specific 3d position.
     * @param pos 
     * @note The 'y' value of pos is ignored by this operation.
     */
    get(pos: vector_3d): number;

    /**
     * Returns true if the float value at a 3d position
     * is the highest position within a given range.
     * 
     * This is typically used for object placement.
     * 
     * @param pos 
     * @param check_radius 
     */
    is_highest(pos: vector_3d, check_radius: number): boolean
    set(x: number, y: number, value: number): void;
    width(): number;
    height(): number;
}

/**
 * Creates a new noisemap
 * @param start_x
 * @param start_y
 * @param width
 * @param height
 * @param frequency
 * @param algorithm
 * @param seed
 */
declare function make_noise(start_x: number, start_y: number, width: number, height: number, frequency: number, algorithm: string, seed: string)

/**
 * Represents a random generator and its state.
 */
declare class random {
    integer(low: number, high: number): number;
    real(low: number, high: number): number;
}

/**
 * Creates a new random generator from a specific seed.
 * @param seed 
 */
declare function random_from_seed(seed: string): random;

/**
 * Creates a new random generator from the current system time.
 */
declare function random_from_time(): random;

/**
 * Represents a rectangular selection in the world and provides
 * iterators for heightmap vertices, texture units, chunks and models within it.
 */
declare class selection {
    /**
     * Creates a noisemap matching the location of this selection
     * 
     * @param frequency 
     * @param algorithm 
     * @param seed 
     */
    make_noise(frequency: number, algorithm: string, seed: string): noisemap

    /**
     * Returns the center point of this selection
     */
    center(): vector_3d;

    /**
     * Returns the smallest point of this selection
     */
    min(): vector_3d;

    /**
     * Returns the highest point of this selection
     */
    max(): vector_3d;

    /**
     * Returns a vector representing the size of this selection on each axis.
     * @note for iterators, only x and z values are respected. y (height) is ignored.
     */
    size(): vector_3d;
    
    /**
     * Creates and returns an iterator for all models inside this selection
     */
    models(): model[];

    /**
     * Creates and returns an iterator for all vertices inside this selection
     */
    verts(): vert[];

    /**
     * Creates and returns an iterator for all texture units inside this selection
     */
    tex(): tex[];

    /**
     * Creates and returns an iterator for all chunks inside this selection
     */
    chunks(): chunk[];
    
    /**
     * Applies all changes made inside this selection. 
     * You almost always want to call this function when you're done
     * with a selection.
     */
    apply(): void;
}

/**
 * Makes and returns a rectangular selection between two points.
 * @param point1 
 * @param point2 
 */
declare function select_between(point1: vector_3d, point2: vector_3d): selection;

/**
 * Makes and returns a rectangular selection around an origin point
 * 
 * @param origin - The center point of the selection
 * @param xRadius
 * @param zRadius 
 * @returns selection
 */
declare function select_origin(origin: vector_3d, xRadius: number, zRadius: number): selection;

/**
 * Returns the chunk at a given position.
 * The tile at the position must be loaded into memory for the
 * operation to be successful.
 * @param position
 */
declare function get_chunk(position: vector_3d): chunk

/**
 * Represents a settings tag that can be accessed at any time by a script.
 */
declare class tag<T> {
    /**
     * Returns the current value of this tag in the settings panel.
     */
    get(): T;
}

/**
 * Represents a single texture unit in the worlds texture layers.
 * 
 * A texture unit represents the smallest area where it's possible to
 * affect textures alpha layers. This is much smaller than the areas made
 * up by heightmap vertices.
 */
declare class tex {
    set_alpha(index: number, alpha: number): void;
    get_alpha(index: number): number;
    get_pos_2d(): vector_3d;
}

/**
 * Represents a single heightmap vertex in the world.
 * 
 * Changes to this vertex takes visible effect in Noggit after you call
 * "apply" on the chunk or selection that contains it.
 */
declare class vert {
    /**
     * Returns the full position of this vertex
     */
    get_pos(): vector_3d;

    /**
     * Changes the height of this vertex
     */
    set_height(y: number);
    add_height(y: number);
    sub_height(y: number);

    /**
     * Changes the vertex color that this vertex blends with the 
     * underlying texture. Values generally range between 0-1, but can 
     * also go higher.
     * 
     * @param red - How much red should be used (default: 1)
     * @param green - How much green should be used (default: 1)
     * @param blue - How much blue should be used (default: 1)
     */
    set_color(red: number, green: number, blue: number): void;

    /**
     * Changes the water type on this vertex, but only if the vertex is 
     * aligned with water tiles. If the vertex is not aligned
     * with a water tile, this function does nothing.
     * 
     * @param type 
     * @param height 
     * 
     * @note The C++ function backing this operation is very slow for the moment.
     * use with care.
     */
    set_water(type: number, height: number): void;

    /**
     * Sets whether this vertex should be a hole, but only if the
     * vertex is aligned with hole tiles. If the vertex is not aligned
     * with a hole tile, this function does nothing.
     * 
     * @param has_hole
     */
    set_hole(has_hole: boolean): void;

    /**
     * Sets a texture alpha layer of all texture units closest to this vertex. 
     * 
     * @param index 
     * @param alpha 
     */
    set_alpha(index: number, alpha: number): void;

    /**
     * Returns the average alpha of all texture units closest to this vertex.
     * @param index 
     */
    get_alpha(index: number);

    /**
     * Returns true if this vertex is aligned with water tiles.
     */
    is_water_aligned(): boolean;
}

/**
 * Contains some general-purpose procedures that don't fit anywhere else.
 * 
 * Access these functions through the global singleton "procedures".
 */
declare class procedures_class {
    paint_texture(sel: selection, img: image, layer: number, pressure: number, angle: number);
}

/**
 * singleton
 */
declare const procedures: procedures_class;