require noggit

[export]
def title(): string
	return "Height Noise"

[export]
def select()
	add_string_param("Algorithm","HgANAAUAAAAAAABAEAAAAAA/CAAAAACAPwAAAAA/AAAAAAABEwBI4RpAGwANAAMAAAAAAABACAAAAAAAPwAAAAAAAI/C9Tw=")
	add_string_param("Seed","noggit")
	add_float_param("Frequency",0.0005,1.0,0.001,5)
	add_float_param("Amplitude",1.0,1000.0,410.0,2)
	add_float_param("Speed",0.0,999.0,1.0,2)

	add_int_param("Hill Texture Index",0,3,0)

	add_description("Generates a heightmap using custom noise from NoiseTool.");
	add_description("")
	add_description("To create your own noise:")
	add_description("1. Get the NoiseTool"); 
	add_description("2. Make some noise"); 
	add_description("3. Copy the encoded node tree at the top of the node editor into \"Algorithm\""); 

[export]
def left_hold()
	var algo = get_string_param("Algorithm")
	var seed = get_string_param("Seed")
	var freq = get_float_param("Frequency")
	var amp = get_float_param("Amplitude")
	var spd = get_float_param("Speed")

	var hilltex = get_int_param("Hill Texture Index")

	// Select the origin 
	var sel = make_selector()
	select_origin(sel,pos(),outer_radius(),outer_radius())

	// Generate a noisemap matching the selection we just made
	var gen = make_noisegen_custom(algo);
    var map = make_noisemap()
	noise_fill_selection(gen,map,sel,seed,freq,10)

	while sel_next_chunk(sel)
		var chunk = sel_get_chunk(sel)

		while chunk_next_vert(chunk)
			var vert = chunk_get_vert(chunk)
			var loc = vert_get_pos(vert);
			var height = noise_get_global(map,loc)*amp;
			vert_set_height(vert,height);
			if hilltex > 0 && height > 20.0
				vert_set_alpha(vert, hilltex,0.5)

		chunk_apply_all(chunk)