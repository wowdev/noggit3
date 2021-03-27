-- This file is part of Noggit3, licensed under GNU General Public License (version 3).
local prop_placer = brush("Prop Placer")

local seed = prop_placer:add_string_tag("Seed","noggit")
local dist = prop_placer:add_int_tag("Distance",1,50,5)

function Prop(index)
    local prop = {}

    prop.model_name = prop_placer:add_string_tag("Prop "..index)
    prop.min_scale = prop_placer:add_real_tag("Prop "..index.." Min Scale",0.001,10,0.7)
    prop.max_scale = prop_placer:add_real_tag("Prop "..index.." Max Scale",0.001,10,0.7)

    function prop:place(evt)
        if string.len(self.model_name:get())==0 then
            return 
        end

        local sel = select_origin(evt:pos(),evt:outer_radius(),evt:outer_radius())

        local models = sel:models()
        while models:next() do
            local model = models:get()
            if(model:has_filename(self.model_name:get())) then
                model:remove()
            end
        end

        local verts = sel:verts()

        local map = make_noise(
              sel:min().x-dist:get()
            , sel:min().z-dist:get()
            , sel:size().x+2*dist:get()
            , sel:size().z+2*dist:get()
            , 10.0
            , "SIMPLEX"
            , seed:get()..self.model_name:get()
        )

        while verts:next() do
            local vert = verts:get()
            local loc = vert:get_pos()
            if map:is_highest(loc,dist:get()) then
                local rnd = random_from_seed(seed:get().." "..loc.x.." "..loc.z)
                local size = rnd:real(self.min_scale:get(),self.max_scale:get())
                local rot = rnd:real(0,360.0)
                add_m2(self.model_name:get(),loc,size,vec(0,rot,0))
            end
        end
        sel:apply()
    end
    return prop
end

prop_placer:add_null_tag()
local prop1 = Prop(1);
prop_placer:add_null_tag();
local prop2 = Prop(2);
prop_placer:add_null_tag();
local prop3 = Prop(3);

function prop_placer:on_left_hold(ctx)
    prop1:place(ctx)
    prop2:place(ctx)
    prop3:place(ctx)
end