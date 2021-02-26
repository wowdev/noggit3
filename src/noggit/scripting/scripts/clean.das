// This file is part of Noggit3, licensed under GNU General Public License (version 3).
require noggit
require strings

[export]
def title(): string
	return "Clean"

[export]
def select()
    add_float_param("Default Height",-1000.0,1000.0,0.0,5)
    add_string_param("Texture 1","")
    add_string_param("Texture 2","")
    add_string_param("Texture 3","")
    add_string_param("Texture 4","")

    add_description("Usage:")
    add_description("")
    add_description("Alt+Click: Delete Models")
    add_description("Shift+Click: Delete Textures")
    add_description("Ctrl+Click: Reset Height")

[export]
def left_hold()
    var t1 = get_string_param("Texture 1")
    var t2 = get_string_param("Texture 2")
    var t3 = get_string_param("Texture 3")
    var t4 = get_string_param("Texture 4")
    var h = get_float_param("Default Height")

    // Makes a selection around the cursor
    var sel = make_selector()
    select_origin(sel,pos(),outer_radius(),outer_radius())

    if(holding_shift())
        while sel_next_chunk(sel)
            var chunk = sel_get_chunk(sel)
            chunk_clear_textures(chunk)

            if length(t1) > 0
                chunk_add_texture(chunk,t1)

            if length(t2) > 0
                chunk_add_texture(chunk,t2)

            if length(t3) > 0
                chunk_add_texture(chunk,t3)

            if length(t4) > 0
                chunk_add_texture(chunk,t4)

            chunk_clear_colors(chunk)
            chunk_apply_all(chunk)

    if(holding_ctrl())
        while sel_next_chunk(sel)
            var chunk = sel_get_chunk(sel)
            while chunk_next_vert(chunk)
                var vert = chunk_get_vert(chunk)
                vert_set_height(vert, h)

            chunk_set_hole(chunk,false);
            chunk_apply_all(chunk)

    if(holding_alt())
        while sel_next_model(sel)
            model_remove(sel_get_model(sel))