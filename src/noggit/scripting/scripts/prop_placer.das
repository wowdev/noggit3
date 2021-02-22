// Made by TSWoW (https://github.com/tswow), licensed under GNU General Public License (version 3).
require noggit
require strings

[export]
def title(): string
	return "Prop Placer"

[export]
def select()
    add_string_param("Seed","noggit")
    add_float_param("Max height",-1000.0,1000.0,20.0,5)
    add_float_param("Min Scale",0.001,10.0,0.7,3)
    add_float_param("Max Scale",0.001,10.0,1.3,3)
    add_int_param("Distance",1,20,1)

    add_int_param("Prop count",1,20,5)

    add_description("Randomly places props into the world");
    add_description("");
    add_description("Reload the script to apply the propcount");
    add_description("");
    add_description("Uses noise sampling, try disc sampling if you want better results.");
    // funny hack to create dynamic parameters
    for i in range(get_int_param("Prop count"))
        add_string_param("Prop {i}","")

[export]
def left_hold()
    var max_height = get_float_param("Max height");
    var min_scale = get_float_param("Min Scale");
    var max_scale = get_float_param("Max Scale");
    var distance = get_int_param("Distance")

    var valid: array<string>
    var seed = get_string_param("Seed")

    for i in range(get_int_param("Prop count"))
        var str = get_string_param("Prop {i}")
        if length(str) > 3
            push(valid,str)

    if length(valid) == 0
        print("No valid models!")
        return
    
	// Select the origin 
	var sel = make_selector()
	select_origin(sel,pos(),outer_radius(),outer_radius())

    // Generate a noisemap matching the selection we just made
	var gen = make_noisegen_simplex()
    var map = make_noisemap()
	noise_fill_selection(gen,map,sel,seed,10.0,10)

    while sel_next_model(sel)
        var model = sel_get_model(sel)
        model_remove(model)

    while sel_next_chunk(sel)
		var chunk = sel_get_chunk(sel)
        while chunk_next_vert(chunk)
            var vert = chunk_get_vert(chunk)
            var loc = vert_get_pos(vert)
            if(noise_is_highest_global(map,loc,distance) && loc.y < max_height)
                // This way, we always get the same random for the same location
                var rnd = random_from_seed("{seed} {loc.x} {loc.z}")
                var model = valid[rand_int32(rnd,0,length(valid))];
                var size = rand_float(rnd,min_scale,max_scale)
                var rot = rand_float(rnd,0.0,360.0)
                add_m2(model,loc,size,vec(0.0,rot,0.0))

        chunk_apply_all(chunk)