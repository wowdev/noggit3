local texture_printer = brush("Texture Printer");

texture_printer:add_description("<b>Description</b>:")
texture_printer:add_description("Prints out texture paths and effect ids ")
texture_printer:add_description("in the clicked chunk.")
texture_printer:add_null_tag()

function texture_printer:on_left_click(evt)
    local sel = select_origin(evt:pos(), 1, 1)

    for i,chunk in pairs(sel:chunks()) do
        if chunk:get_texture_count() == 0 then
            print("Chunk has no textures")
        end

        print("== Chunk Textures ==")
        for i=0,chunk:get_texture_count()-1 do
            local tex = chunk:get_texture(i)
            local eff = chunk:get_effect(i)
            print("Layer "..i..":")
            print("    Texture: "..tex)
            print("    Effect: "..eff)
        end
        print("")
    end
end