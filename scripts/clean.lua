-- This file is part of Noggit3, licensed under GNU General Public License (version 3).
local clean = brush("Clean");

local def_height = clean:add_real_tag("Default height",-1000,1000,0)

local tex_layer_1 = clean:add_string_tag("Texture layer 1","")
local prop_layer_1 = clean:add_int_tag("Effect layer 1",-2,9999999999,-2)
local tex_layer_2 = clean:add_string_tag("Texture layer 2","")
local prop_layer_2 = clean:add_int_tag("Effect layer 2",-2,9999999999,-2)
local tex_layer_3 = clean:add_string_tag("Texture layer 3","")
local prop_layer_3 = clean:add_int_tag("Effect layer 3",-2,9999999999,-2)
local tex_layer_4 = clean:add_string_tag("Texture layer 4","")
local prop_layer_4 = clean:add_int_tag("Effect layer 4",-2,9999999999,-2)

function clean:on_left_hold(evt)
    local sel = select_origin(evt:pos(), evt:outer_radius(), evt:outer_radius())
    if(holding_shift()) then
        for i,chunk in pairs(sel:chunks()) do
            chunk:clear_textures()
            chunk:clear_colors()
            if tex_layer_1:get() ~= "" then chunk:add_texture(tex_layer_1:get(),prop_layer_1:get()) end
            if tex_layer_2:get() ~= "" then chunk:add_texture(tex_layer_2:get(),prop_layer_2:get()) end
            if tex_layer_3:get() ~= "" then chunk:add_texture(tex_layer_3:get(),prop_layer_3:get()) end
            if tex_layer_4:get() ~= "" then chunk:add_texture(tex_layer_4:get(),prop_layer_4:get()) end
            chunk:apply_all()
        end
    end

    if(holding_ctrl()) then
        for i,vert in pairs(sel:verts()) do
            vert:set_height(def_height:get())
        end
    end

    if(holding_alt()) then
        for i,model in pairs(sel:models()) do
            model:remove()
        end
    end

    sel:apply()
end