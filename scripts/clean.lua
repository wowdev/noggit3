-- This file is part of Noggit3, licensed under GNU General Public License (version 3).
local clean = brush("Clean");

local def_height = clean:add_real_tag("Default height",-1000,1000,0)

function clean:on_left_hold(evt)
    local sel = select_origin(evt:pos(), evt:outer_radius(), evt:outer_radius())
    if(holding_shift()) then
        local chunks = sel:chunks()
        while chunks:next() do
            local chunk = chunks:get()
            chunk:clear_textures()
            chunk:apply_all()
        end
    end

    if(holding_ctrl()) then
        local verts = sel:verts()
        while verts:next() do
            local vert = verts:get()
            vert:set_height(def_height:get())
        end
    end

    if(holding_alt()) then
        local models = sel:models()
        while models:next() do
            local model = models:get()
            model:remove()
        end
    end

    sel:apply()
end