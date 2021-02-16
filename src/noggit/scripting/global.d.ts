// script_math
declare function round(arg: number): number;
declare function pow(a1: number, a2: number): number;
declare function log10(arg: number): number;
declare function log(arg: number): number;
declare function ceil(arg: number): number;
declare function floor(arg: number): number;
declare function exp(arg: number): number;
declare function cbrt(arg: number): number;
declare function acosh(arg: number): number;
declare function asinh(arg: number): number;
declare function atanh(arg: number): number;
declare function cosh(arg: number): number;
declare function sinh(arg: number): number;
declare function tanh(arg: number): number;
declare function acos(arg: number): number;
declare function asin(arg: number): number;
declare function atan(arg: number): number;
declare function cos(arg: number): number;
declare function sin(arg: number): number;
declare function tan(arg: number): number;
declare function sqrt(arg: number): number;
declare function abs(arg: number): number;

// script_vec
declare class script_vec {
    x(): number;
    y(): number;
    z(): number;
    add(x: number, y: number, z: number): script_vec;
}
declare function vec(x: number, y: number, z: number): script_vec;

// script_setup
declare class Holder<T>{
    get(): T;
}
declare function add_script(name: string, callback: ()=>void);
declare function add_desc(desc: string);
declare function param_double(name: string, min: number, max: number, def: number): Holder<number>;
declare function param_int(name: string, min: number, max: number, def: number): Holder<number>;
declare function param_bool(name: string, def: boolean): Holder<boolean>;
declare function param_string(name: string, def: boolean): Holder<string>;

// script_selection
declare class script_selection {
    is_on_chunk(): boolean;
    is_on_vertex(): boolean;
    is_on_tex(): boolean;

    next_chunk();
    next_vertex();
    next_tex();

    reset_cur_chunk();
    reset_cur_vertex();
    reset_cur_tex();

    chunk_add_texture(texture: string): number;
    chunk_clear_textures();
    chunk_remove_texture(index: number);
    chunk_get_texture(index: number): string;
    chunk_apply_textures();
    chunk_apply_heightmap();
    chunk_apply_vertex_color();
    chunk_apply_all();
    chunk_set_impassable(add: boolean);
    chunk_get_area_id(): number;
    chunk_set_area_id(value: number);
    
    vert_get_x(): number;
    vert_get_y(): number;
    vert_get_z(): number;

    vert_set_y(value: number);
    vert_set_color(r: number, g: number, b: number);
    vert_set_water(type: number, height: number);
    vert_is_water_aligned(): boolean;
    vert_set_hole(add: boolean): void;
    tex_get_alpha(index: number): number;
    tex_set_alpha(index: number, alpha: number);
}

// script_random
declare class script_random {
    get_int32(low: number, high: number): number;
    get_uint32(low: number, high: number): number;

    get_double(low: number, high: number): number;
    get_float(low: number, high: number): number;
}
declare function random_from_seed(seed: string): script_random;
declare function random_from_time(): script_random;

// script_print
declare function print(...args: any[]);

// script_noise
declare class script_noise_2d {
    /** Gets a value from integer parameters (faster) */
    get(x: number, y: number);
    /** Gets a value from float parameters (slower) */
    get_f(x: number, y: number);
    set(x: number, y: number, value: number);
    get_start_x(): number;
    get_start_y(): number;
    get_width(): number;
    get_height(): number;
}

declare class script_noise_generator {
    uniform_2d(seed: string, xStart: number, yStart: number, xSize: number, ySize: number, frequency: number);
}

declare function noise_simplex(): script_noise_generator;
declare function noise_perlin(): script_noise_generator;
declare function noise_value(): script_noise_generator;
declare function noise_fractal(): script_noise_generator;
declare function noise_cellular(): script_noise_generator;
declare function noise_white(): script_noise_generator;
declare function noise_custom(encodedNodeTree: string): script_noise_generator;

// script_image
declare class script_image {
    get_pixel(x: number, y: number): number;
    set_pixel(x: number, y: number, value: number): number;
    save(path: string);
    get_width(): number;
    get_height(): number;
}

declare function load_image(path: string): script_image;
declare function create_image(width: number, height: number): script_image;

// script_filesystem
declare class script_file_iterator {
    is_on_file(): boolean;
    get_file(): string;
    next_file();
}

declare function write_file(path: string, content: string);
declare function append_file(path: string, content: string);
declare function read_directory(path: string): script_file_iterator;
declare function read_file(path: string): string;
declare function path_exists(path: string): boolean;

// script_context
declare class script_context {
    pos(): script_vec;
    select(origin_x: number, origin_z: number, inner_radius: number, outer_raduis: number): script_selection;
    change_terrain(vec: script_vec, change: number, radius: number, inner_radius: number, brush_type: number);
    add_m2(filename: string, pos: script_vec, scale: number, rotation: script_vec);
    add_wmo(filename: string, pos: script_vec, rotation: script_vec);
    get_map_id(): number;
    get_area_id(): number;
    set_area_id(pos: script_vec, id: number, adt?: boolean);
    change_vertex_color(pos: script_vec, color: script_vec, alpha: number, change: number, radius: number, edit_mode: boolean);
    get_vertex_color(pos: script_vec): script_vec;
    flatten_terrain(pos: script_vec, remain: number, radius: number, brush_type: number, lower: boolean, raise: boolean, origin: script_vec, angle: number, orientation: number);
    blur_terrain(pos: script_vec, remain: number, radius: number, brush_type: number, lower: boolean, raise: boolean);
    erase_textures(pos: script_vec);
    clear_shadows(pos: script_vec);
    clear_textures(pos: script_vec);
    clear_height(pos: script_vec);
    /**
     * @param pos 
     * @param big: If the hole should cover the whole chunk
     * @param hole: true = create hole, false = remove hole
     */
    set_hole(pos: script_vec, big: boolean, hole: boolean);
    set_hole_adt(pos: script_vec, hole: boolean);
    deselect_vertices_radius(pos: script_vec, radius: number);
    clear_vertex_selection();
    move_vertices(amount: number);
    flatten_vertices(amount: number);
    update_vertices();
    paint_texture(pos: script_vec, strength: number, pressure: number, hardness: number, radius: number, texture: string);

    cam_pitch(): number;
    cam_yaw(): number;

    holding_alt(): boolean;
    holding_shift(): boolean;
    holding_ctrl(): boolean;
    holding_space(): boolean;
}

// basic events
declare function on_left_click(callback: (arg: script_context)=>void);
declare function on_left_hold(callback: (arg: script_context)=>void);
declare function on_left_release(callback: (arg: script_context)=>void);

declare function on_right_click(callback: (arg: script_context)=>void);
declare function on_right_hold(callback: (arg: script_context)=>void);
declare function on_right_release(callback: (arg: script_context)=>void);