-- This file is part of Noggit3, licensed under GNU General Public License (version 3).
local noise_brush = brush("Height Noise")

local algo = noise_brush:add_string_tag("Algorithm","HgANAAUAAAAAAABAEAAAAAA/CAAAAACAPwAAAAA/AAAAAAABEwBI4RpAGwANAAMAAAAAAABACAAAAAAAPwAAAAAAAI/C9Tw=")
local seed = noise_brush:add_string_tag("Seed","noggit")
local frequency = noise_brush:add_real_tag("Frequency",0.0005,1.0,0.001,5)
local amplitude = noise_brush:add_real_tag("Amplitude",1.0,1000.0,410.0,2)

function noise_brush:on_left_hold(evt)
    local sel = select_origin(
        evt:pos(),
        evt:outer_radius(),
        evt:outer_radius()
    )
    local map = sel:make_noise(
        frequency:get(),
        algo:get(),
        seed:get()
    )
    for i,vert in pairs(sel:verts()) do
        local height = map:get(
            vert:get_pos()
        ) * amplitude:get()
        vert:set_height(height)
    end
    sel:apply()
end