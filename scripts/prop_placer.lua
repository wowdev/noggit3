-- This file is part of Noggit3, licensed under GNU General Public License (version 3).
local prop_placer = brush("Prop Placer")

local seed = prop_placer:add_string_tag("Seed","noggit")
local dist = prop_placer:add_int_tag("Distance",1,50,5)
local delete_all = prop_placer:add_bool_tag("Delete all models",false)
local circle_brush = prop_placer:add_bool_tag("Circle brush",false)

function Prop(index)
    local prop = {}

    prop.model_name = prop_placer:add_string_tag("Prop "..index)
    prop.min_scale = prop_placer:add_real_tag("Prop "..index.." Min Scale",0.001,10,0.7,2)
    prop.max_scale = prop_placer:add_real_tag("Prop "..index.." Max Scale",0.001,10,0.7,2)

    return prop
end

prop_placer:add_null_tag()
local prop1 = Prop(1);
prop_placer:add_null_tag();
local prop2 = Prop(2);
prop_placer:add_null_tag();
local prop3 = Prop(3);

local props = {prop1,prop2,prop3}

function prop_placer:on_left_hold(evt)
    local cur_props = {}
    local count = 0
    for _,v in pairs(props) do
        if string.len(v.model_name:get())>0 then
            table.insert(cur_props,v)
            count = count + 1
        end
    end

    if count == 0 then 
        print("No models selected")
        return 
    end

    local sel = select_origin(evt:pos(),evt:outer_radius(),evt:outer_radius())
    for _,v in pairs(cur_props) do

        for i,model in pairs(sel:models()) do
            if(
                (delete_all:get() or model:has_filename(v))
                and 
                ((not circle_brush:get()) 
                    or dist_2d(
                          model:get_pos()
                        , evt:pos()) < evt:outer_radius())
            ) then
                model:remove()
            end
        end
    end

    local map = make_noise(
        sel:min().x-dist:get()
      , sel:min().z-dist:get()
      , sel:size().x+2*dist:get()
      , sel:size().z+2*dist:get()
      , 1
      , "SIMPLEX"
      , seed:get()
    )

    for i,vert in pairs(sel:verts()) do
        if (not circle_brush:get()) or dist_2d(
            vert:get_pos(),
            evt:pos()
        ) < evt:outer_radius() then
            local loc = vert:get_pos()
            if map:is_highest(loc,dist:get()) then
                local rnd = random_from_seed(seed:get().." "..loc.x.." "..loc.z)
                local index = rnd:integer(1,count+1)
                local prop = cur_props[index]
                local size = rnd:real(prop.min_scale:get(),prop.max_scale:get())
                local rot = rnd:real(0,360.0)
                add_m2(prop.model_name:get(),loc,size,vec(0,rot,0))
            end
        end
    end

    sel:apply()
end