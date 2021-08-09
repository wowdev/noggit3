local texture_brush = brush("Texture Brush")
texture_brush:add_description("<b>Description</b>:")
texture_brush:add_description("- Uses a black / white texture to paint alpha layers. ")
texture_brush:add_description("- The painting is only additive at the moment,")
texture_brush:add_description("  meaning it cannot paint layers from \"underneath\"")
texture_brush:add_description("  another layer.")

local texture = texture_brush:add_string_tag("Brush Texture", "")
local pressure = texture_brush:add_real_tag("Pressure", 0, 1000, 1, 2)
local index = texture_brush:add_int_tag("Texture Index", 1, 3, 1)

texture_brush.on_left_hold = function(brush, evt)
    local image = load_png(
        texture:get()
    )
    local sel = select_origin(
        evt:pos(),
        evt:outer_radius(),
        evt:outer_radius()
    )
    procedures:paint_texture(sel,image,index:get(),pressure:get(),cam_yaw()-90)
    sel:apply()
end