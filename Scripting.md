# Scripting

This version of noggit has support for "Script Brushes", a special type of brush with scripted behavior.
Script brushes allow us to manipulate heightmaps, textures, vertex colors, water and m2/wmo objects in a customized manner.
Script brushes is an experimental feature, so please be careful and keep regular backups of your work when working with them.

See [this video](https://youtu.be/J5-8zQ5U8gY) for an introduction to the UI.


## Introduction to Da Scripting

The scripting language we use is called "Da". It's a statically typed language that is otherwise very similar to Python, 
and is thus indention-based but has optional support for C-style brackets too. You can use the below guide to quickly learn how to write in Da,

We recommend that you use 
[VSCodium](https://vscodium.com/#install) 
with our 
[minimal da extension](https://github.com/tswow/das-minimal-extension/releases/tag/v0.0.1) 
to easily get autocompletion of function names and type specifications for the Noggit API.

To create a new script brush, create a `.das` file in the `scripts` subdirectory in your noggit installation.
We can register events by special named functions. The below code illustrates the bare minimum required for a simple left-click event with a custom title:

```py
require noggit

[export]
def title(): string
  return "Hello World Brush"

[export]
def left_click()
    print("Hello World!")
```

### Events

All event listeners are created as global functions with the `[export]` decorator, and none of them take any arguments. 

- `left_click`: Fires when the user first presses the left mouse button in the world.
- `left_hold`: Fires continuously as the user holds the left mouse button in the world.
- `left_release`: Fires when the user releases the left mouse button.
- `right_click`, `right_hold`, `right_release`: Right mouse button versions of the above three functions. 
- `select`: Fires when the user selects this script in the tool window. This event is used to register custom brush options
- `title`: Only used to return a display name for this brush in the tool window. It must have the signature `def title(): string` if implemented.


### Quickstart for Da syntax and types

Da has a lot of weird constructs and types, but interfacing the Noggit API mostly just consists of simple function calls and while loops. 
This section aims to give you the basic knowledge you will need to use Da with Noggit, please refer to 
[the official documentation](https://dascript.org/doc/reference/language/) for a more in-depth overview of what the language can actually do.

Note: Da is statically typed language, and it's a little annoying that it has **no** automatic type conversions, 
meaning you have to explicitly cast between `ints` and `floats`, and even between `ints` and `unsigned ints`.

#### Numbers
  - Because of the issue with type conversions, the Noggit API uses `int` for most integer parameters, and `float` for all floating point parameters.
  - Number literals are assumed to be `int` if they are written like normal base-10 numbers with no fraction: `10`
  - Float literals are defined by appending a fractional part to a number: `10.0`
    - If a function takes a float parameter, you **must** specify the fractional part: `my_func(10.0)`
  - Unsigned int literals are defined by appending "u" to an int literal: `10u` or by writing it in hexadecimal: `0xA`
    - Currently, unsigned ints are only used for defining pixel values in `script_images`
  
  - Explicit casting between types is necessary: 
  ```py
  var a: float = 10.0
  var b: int = 25
  var c: float = float(b)+a
  var d: int = int(a)+b
  var e = 0xff
  var f = b + int(e)
  ```

#### Arrays
  - Create array: `var arr <- [{auto "one"; "two"; "three"}]`
  - Push a value: `push(arr,"new value")`
  - Length of array: `var len = length(arr)`
  - Erase from array: `erase(arr,2)`
  - Clear array: `clear(arr)`

#### Tables
  - Created table: `var tab: table<string,int>`
  - Write a value: `tab["key"] = 10`.
  - Read a value: `var value = find(tab,"key")`
  - Check if a value exists: `var exists = key_exists(tab,"key")`

### Blocks and flow control

Just like in python, empty blocks are not allowed and must at least have a "pass" statement in them if there are no other statements.

#### Functions
```py
def my_function(arg1: int, arg2: string): int
  return 25;
```

#### For loops

Ranges:
```py
for i in range(0,10)
  pass
```

#### Arrays
```py
var arr <- [{auto "one"; "two"; "three"}]
for val in arr
  print("{val}")
```

#### Tables
```py
var tab: table<string,int>
for k,v in keys(tab), values(tab)
  print("{k} {v}")
```

#### While loops
```py
while some_value
  pass
```

#### If statements
```py
if some_value
  pass
elif some_other_value
  pass
```

**TODO**: Explain structs

### Exporting/Importing

We can import the noggit API with the statement `require noggit` at the top of our script files,
and similarly we can import other .das files the same way. To allow other modules to access a function, use the same `[export]` tag as on event listeners:

**file_a.das**:
```py
def local_function()
  print("This function is only accessible inside file_a.das")

[export]
def library_function()
  print("This function is accessible to any file that 'requires' it")
```

**file_b.das**:

```py
require noggit
require file_a

[export]
def left_click()
  print("This is an event listener, so it also has the 'export' tag")
  library_function()
```

### Noggit API

The Noggit API are the functions we expose from inside Noggit to scripts. The file `scripts/noggit.spec.das` contains specifications of all the functions we expose.
`.spec.das` files contain no actual implementations since they only describe existing C++ functions, and we use them similarly to `d.ts` files in JavaScript.

To use the Noggit API, your script files must contain the line `require noggit` at the top. 
If you can't see any noggit functions in your autocompletions, it's likely you forgot this line.

Noggit API functions should not be called globally in your scripts, but we still made them globally accessible to make them less cumbersome to write.
The following two rules define where it's safe to call Noggit functions from in your scripts:

- All functions `add_x_param` should be called from the `select` function **only** (or any functions it calls).
- All other functions should be called from click/hold/release events **only** (or any functions they call).

#### Script Parameters

Scripts can register custom parameters that can later be read when the user paints with them in the world. The below code illustrates a basic example property:

```py
[export]
def select()
  add_string_param("String Parameter","Value")
  
[export]
def left_click()
   var str = get_string_param("String Parameter")
   print("The parameter was {str}")
```

### Selections

Most Noggit operations involve some kind of selection using the Noggit API. The typical workflow is as follows:

1. Make a selection around a clicked location
2. Iterate all models
3. Iterate all chunks
  3.1. For each chunk, iterate all vertices
  3.2. For each chunk, iterate all texture units
  
The following code example illustrates how this can be achieved.

```py
require noggit

[export]
def left_click()
   var sel = make_selector()
   select_origin(sel,pos(),outer_radius(),outer_radius())
   while sel_next_model(sel)
      var model = sel_get_model(sel)
      model_remove(model)
    
   while sel_next_chunk(sel)
      var chunk = sel_get_chunk(sel)
      while(chunk_next_vert(chunk))
         var vert = chunk_get_vert(chunk)
         vert_set_height(vert,0)
    
   while(chunk_next_tex(chunk))
      var tex = chunk_get_tex(chunk)
      tex_set_alpha(tex,1,0.5)
```
