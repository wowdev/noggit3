# Scripting

[API Documentation](api/modules.md)

This version of Noggit has support for scriptable brushes, meaning you can use the Lua programming language (version 5.1) to customize your own workflow.

Script brushes should be placed in the `scripts` subdirectory in your Noggit installation. Script brushes can be loaded inside Noggit from the scripting tool (book icon) in the menu to the left.

**Warning**: Scripts are created by third party developers and are not maintained by Noggit. Scripts downloaded from the internet might contain malicious code. 

## Hello World

This example illustrates a simple working script brush that you can load in the scripting tool.

```lua
-- We create a new script brush that becomes 
local my_brush = brush("My Brush")

-- We register an event for when this brush clicks on the ground
function my_brush:on_left_click(evt)
    print("Hello world!")
end
```

## Script Parameters

Script brushes can register custom settings that the user can configure without editing the script.

```lua
local parameter_brush = brush("Parameter Brush")

-- Editable string
local string_param = parameter_brush:add_string_tag("Parameter Name","default value")

-- Integer between 1 and 50, default value of 5
local int_param = parameter_brush:add_int_tag("Int Tag",1,50,5)

-- Real between 0.0005 and 1, default value of 0.001 and at most 5 decimal points.
local real_param = parameter_brush:add_real_tag("Real Tag",0.0005,1.0,0.001,5)

-- Checkbox
local bool_param = parameter_brush:add_bool_tag("Bool Tag",false)

function parameter_brush:on_left_click(evt)
    -- We can now access the tag values from the brush event
    print("String: " .. string_param:get())
    print("Bool: " .. bool_param:get())
    print("Int: " .. int_param:get())
    print("Real: " .. real_param:get())

    -- And we can access the special radius settings from the event parameter
    print("Outer brush radius:" .. evt:outer_radius())
    print("Inner brush radius" .. evt:inner_radius())
end
```

## Selections

Most noggit operations involve some kind of selection using the Noggit API.

The following code illustrates how we can iterate various data in Noggit: 

```lua
local selection_brush = brush("Selection Brush")

function selection_brush:on_left_click(evt)
    -- Creates a square selection at the clicked position
    -- containing the entire outer brush circle
    local sel = select_origin(evt:pos(), evt:outer_radius())

    -- iterate selected chunks
    for i,chunk in pairs(sel:chunks()) do
        -- do something to the chunk here
    end
    
    -- iterate selected vertices
    for i,vert in pairs(sel:verts()) do
        -- do something to the vertex here
    end

    -- iterate selected texture units
    for i,tex in pairs(sel:tex()) do
        -- do something to the texture unit here
    end

    -- iterate selected models (m2 and wmo)
    for i,model in pairs(sel:model()) do
        -- do something to the model here
    end
end
```

## Noggit API

To see all the predefined functions you can call from scripts, see the [API documentation](api/modules.md). 

### File System Permissions

The functions [image::save](api/classes/image####save) and [write_file](api/modules.md####write_file) can be used to write to the users file systems. By default, when a script tries to call either of these functions the user will be prompted with a popup to confirm they wish to write to that file. If they answer no, an exception will be thrown and stop execution of the script, and if they answer yes the file will be written, and can be written to again without asking until Noggit is restarted. 

This default behavior can be disabled in the Noggit settings under `Allow scripts to write to any file`.
