-- This file is part of Noggit3, licensed under GNU General Public License (version 3).
local painter_brush = brush("Image Painter")
local image_path = painter_brush:add_string_tag("Filename", "")

function painter_brush:on_left_click(evt)
    local img = load_png(
        image_path:get()
    )
    local origin = evt:pos()
    local width = img:width()
    local height = img:height()
    local half_width = width / 2
    local half_height = height / 2
    local sel = select_origin(origin, width, height)

    for i,vert in pairs(sel:verts()) do
        local angle = cam_yaw() - 90
        local pos = rotate_2d(
            vert:get_pos(),
            origin,
            angle
        )
        local local_x = round((pos.x - origin.x) + half_width)
        local local_z = round((pos.z - origin.z) + half_height)

        if local_x >= 0 
            and local_x < width 
            and local_z >= 0 
            and local_z < height 
            and img:get_alpha(local_x,local_z)>0.5 then
            local red = img:get_red(local_x, local_z)
            local green = img:get_green(local_x, local_z)
            local blue = img:get_blue(local_x, local_z)
            vert:set_color(red, green, blue)
        end
    end
    sel:apply()
end