# 

## Table of contents

### Classes

- [chunk](classes/chunk.md)
- [image](classes/image.md)
- [model](classes/model.md)
- [noisemap](classes/noisemap.md)
- [procedures\_class](classes/procedures_class.md)
- [random](classes/random.md)
- [script\_brush](classes/script_brush.md)
- [script\_brush\_event](classes/script_brush_event.md)
- [selection](classes/selection.md)
- [tag](classes/tag.md)
- [tex](classes/tex.md)
- [vector\_3d](classes/vector_3d.md)
- [vert](classes/vert.md)

### Type aliases

- [brush\_callback](modules.md#brush_callback)
- [callback](modules.md#callback)
- [nil](modules.md#nil)

### Variables

- [procedures](modules.md#procedures)

### Functions

- [abs](modules.md#abs)
- [acos](modules.md#acos)
- [acosh](modules.md#acosh)
- [add\_m2](modules.md#add_m2)
- [add\_wmo](modules.md#add_wmo)
- [append\_file](modules.md#append_file)
- [asin](modules.md#asin)
- [asinh](modules.md#asinh)
- [atan](modules.md#atan)
- [atanh](modules.md#atanh)
- [brush](modules.md#brush)
- [cam\_pitch](modules.md#cam_pitch)
- [cam\_yaw](modules.md#cam_yaw)
- [camera\_pos](modules.md#camera_pos)
- [cbrt](modules.md#cbrt)
- [ceil](modules.md#ceil)
- [cos](modules.md#cos)
- [cosh](modules.md#cosh)
- [create\_image](modules.md#create_image)
- [dist\_2d](modules.md#dist_2d)
- [dist\_2d\_compare](modules.md#dist_2d_compare)
- [exp](modules.md#exp)
- [floor](modules.md#floor)
- [get\_area\_id](modules.md#get_area_id)
- [get\_chunk](modules.md#get_chunk)
- [get\_map\_id](modules.md#get_map_id)
- [holding\_alt](modules.md#holding_alt)
- [holding\_ctrl](modules.md#holding_ctrl)
- [holding\_left\_mouse](modules.md#holding_left_mouse)
- [holding\_right\_mouse](modules.md#holding_right_mouse)
- [holding\_shift](modules.md#holding_shift)
- [holding\_space](modules.md#holding_space)
- [lerp](modules.md#lerp)
- [load\_png](modules.md#load_png)
- [log](modules.md#log)
- [log10](modules.md#log10)
- [make\_noise](modules.md#make_noise)
- [path\_exists](modules.md#path_exists)
- [pow](modules.md#pow)
- [print](modules.md#print)
- [random\_from\_seed](modules.md#random_from_seed)
- [random\_from\_time](modules.md#random_from_time)
- [read\_file](modules.md#read_file)
- [rotate\_2d](modules.md#rotate_2d)
- [round](modules.md#round)
- [select\_between](modules.md#select_between)
- [select\_origin](modules.md#select_origin)
- [sin](modules.md#sin)
- [sinh](modules.md#sinh)
- [sqrt](modules.md#sqrt)
- [tan](modules.md#tan)
- [tanh](modules.md#tanh)
- [vec](modules.md#vec)
- [write\_file](modules.md#write_file)

## Type aliases

### brush\_callback

Ƭ **brush\_callback**: [*callback*](modules.md#callback)<(`brush`: [*script\_brush*](classes/script_brush.md), `event`: [*script\_brush\_event*](classes/script_brush_event.md)) => *void*\>

The type of callback used for brush events.

**`note`** In lua, the first argument becomes "self" argument if using colon notation

___

### callback

Ƭ **callback**<T\>: T \| [*nil*](modules.md#nil)

Callback functions are unassigned by default, but may be assigned to
by the user

#### Type parameters:

Name |
:------ |
`T` |

___

### nil

Ƭ **nil**: *undefined*

This is the documentation for the Noggit scripting API.
Functions not connected to a class are global and can be called from
anywhere in a script.

## Variables

### procedures

• `Const` **procedures**: [*procedures\_class*](classes/procedures_class.md)

singleton

## Functions

### abs

▸ **abs**(`arg`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`arg` | *number* |

**Returns:** *any*

___

### acos

▸ **acos**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### acosh

▸ **acosh**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### add\_m2

▸ **add_m2**(`filename`: *string*, `pos`: [*vector\_3d*](classes/vector_3d.md), `scale`: *number*, `rotation`: [*vector\_3d*](classes/vector_3d.md)): *void*

Spawns an m2 model in the world.

#### Parameters:

Name | Type |
:------ | :------ |
`filename` | *string* |
`pos` | [*vector\_3d*](classes/vector_3d.md) |
`scale` | *number* |
`rotation` | [*vector\_3d*](classes/vector_3d.md) |

**Returns:** *void*

___

### add\_wmo

▸ **add_wmo**(`filename`: *string*, `pos`: [*vector\_3d*](classes/vector_3d.md), `rot`: [*vector\_3d*](classes/vector_3d.md)): *void*

Spawns a wmo model in the world.

**`note`** wmo models cannot be scaled.

#### Parameters:

Name | Type |
:------ | :------ |
`filename` | *string* |
`pos` | [*vector\_3d*](classes/vector_3d.md) |
`rot` | [*vector\_3d*](classes/vector_3d.md) |

**Returns:** *void*

___

### append\_file

▸ **append_file**(`file`: *string*, `content`: *string*): *void*

Appends text to a file

**`note`** This operation REQUIRES explicit permission from the user,
or it will throw an error.

#### Parameters:

Name | Type |
:------ | :------ |
`file` | *string* |
`content` | *string* |

**Returns:** *void*

___

### asin

▸ **asin**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### asinh

▸ **asinh**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### atan

▸ **atan**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### atanh

▸ **atanh**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### brush

▸ **brush**(`name`: *string*): [*script\_brush*](classes/script_brush.md)

Creates a new script brush

#### Parameters:

Name | Type |
:------ | :------ |
`name` | *string* |

**Returns:** [*script\_brush*](classes/script_brush.md)

___

### cam\_pitch

▸ **cam_pitch**(): *number*

Returns the cameras pitch rotation (the one you almost NEVER want)

**Returns:** *number*

___

### cam\_yaw

▸ **cam_yaw**(): *number*

Returns the cameras yaw rotation (the one you almost ALWAYS want)

**Returns:** *number*

___

### camera\_pos

▸ **camera_pos**(): [*vector\_3d*](classes/vector_3d.md)

Returns the current camera position

**Returns:** [*vector\_3d*](classes/vector_3d.md)

___

### cbrt

▸ **cbrt**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### ceil

▸ **ceil**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### cos

▸ **cos**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### cosh

▸ **cosh**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### create\_image

▸ **create_image**(`width`: *number*, `height`: *number*): [*image*](classes/image.md)

Creates a new blank image

#### Parameters:

Name | Type |
:------ | :------ |
`width` | *number* |
`height` | *number* |

**Returns:** [*image*](classes/image.md)

___

### dist\_2d

▸ **dist_2d**(`from`: [*vector\_3d*](classes/vector_3d.md), `to`: [*vector\_3d*](classes/vector_3d.md)): *any*

Returns the 2d distance (ignoring y) between two vectors

#### Parameters:

Name | Type |
:------ | :------ |
`from` | [*vector\_3d*](classes/vector_3d.md) |
`to` | [*vector\_3d*](classes/vector_3d.md) |

**Returns:** *any*

___

### dist\_2d\_compare

▸ **dist_2d_compare**(`from`: [*vector\_3d*](classes/vector_3d.md), `to`: [*vector\_3d*](classes/vector_3d.md), `dist`: *number*): *number*

Compares the 2d distance (ignoring y value) between two vectors to a given distance.
This operation is significantly faster than manually comparing to the result of dist_2d

#### Parameters:

Name | Type |
:------ | :------ |
`from` | [*vector\_3d*](classes/vector_3d.md) |
`to` | [*vector\_3d*](classes/vector_3d.md) |
`dist` | *number* |

**Returns:** *number*

___

### exp

▸ **exp**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### floor

▸ **floor**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### get\_area\_id

▸ **get_area_id**(`pos`: [*vector\_3d*](classes/vector_3d.md)): *number*

Returns the area id at a specific position.

The 'y' value is ignored for this operation.

#### Parameters:

Name | Type |
:------ | :------ |
`pos` | [*vector\_3d*](classes/vector_3d.md) |

**Returns:** *number*

___

### get\_chunk

▸ **get_chunk**(`position`: [*vector\_3d*](classes/vector_3d.md)): [*chunk*](classes/chunk.md)

Returns the chunk at a given position.
The tile at the position must be loaded into memory for the
operation to be successful.

#### Parameters:

Name | Type |
:------ | :------ |
`position` | [*vector\_3d*](classes/vector_3d.md) |

**Returns:** [*chunk*](classes/chunk.md)

___

### get\_map\_id

▸ **get_map_id**(): *number*

Returns the id of the currently open map

**Returns:** *number*

___

### holding\_alt

▸ **holding_alt**(): *boolean*

Returns true if the user is currently pressing the alt key

**Returns:** *boolean*

___

### holding\_ctrl

▸ **holding_ctrl**(): *boolean*

Returns true if the user is currently pressing the ctrl key

**Returns:** *boolean*

___

### holding\_left\_mouse

▸ **holding_left_mouse**(): *boolean*

Returns true if the user is currently pressing the left mouse button

**Returns:** *boolean*

___

### holding\_right\_mouse

▸ **holding_right_mouse**(): *boolean*

Returns true if the user is currently pressing the right mouse button

**Returns:** *boolean*

___

### holding\_shift

▸ **holding_shift**(): *boolean*

Returns true if the user is currently pressing the shift key

**Returns:** *boolean*

___

### holding\_space

▸ **holding_space**(): *boolean*

Returns true if the user is currently pressing the spacebar

**Returns:** *boolean*

___

### lerp

▸ **lerp**(`from`: *number*, `to`: *number*, `ratio`: *number*): *number*

Returns the value at some percentage between two values.

#### Parameters:

Name | Type | Description |
:------ | :------ | :------ |
`from` | *number* | the minimum range value   |
`to` | *number* | the maximum range value   |
`ratio` | *number* | the percentage to take (typically between 0-1)    |

**Returns:** *number*

___

### load\_png

▸ **load_png**(`path`: *string*): [*image*](classes/image.md)

Loads a png file into an in-memory image.

#### Parameters:

Name | Type |
:------ | :------ |
`path` | *string* |

**Returns:** [*image*](classes/image.md)

___

### log

▸ **log**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### log10

▸ **log10**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### make\_noise

▸ **make_noise**(`start_x`: *number*, `start_y`: *number*, `width`: *number*, `height`: *number*, `frequency`: *number*, `algorithm`: *string*, `seed`: *string*): *any*

Creates a new noisemap

#### Parameters:

Name | Type |
:------ | :------ |
`start_x` | *number* |
`start_y` | *number* |
`width` | *number* |
`height` | *number* |
`frequency` | *number* |
`algorithm` | *string* |
`seed` | *string* |

**Returns:** *any*

___

### path\_exists

▸ **path_exists**(`path`: *string*): *boolean*

Returns true if a pathname exists already

#### Parameters:

Name | Type |
:------ | :------ |
`path` | *string* |

**Returns:** *boolean*

___

### pow

▸ **pow**(`a1`: *number*, `a2`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a1` | *number* |
`a2` | *number* |

**Returns:** *any*

___

### print

▸ **print**(): *void*

**Returns:** *void*

▸ **print**(...`args`: *any*[]): *any*

Prints out a message to the script window.
(sometimes, with errors, print messages will be suppressed)

#### Parameters:

Name | Type |
:------ | :------ |
`...args` | *any*[] |

**Returns:** *any*

___

### random\_from\_seed

▸ **random_from_seed**(`seed`: *string*): [*random*](classes/random.md)

Creates a new random generator from a specific seed.

#### Parameters:

Name | Type |
:------ | :------ |
`seed` | *string* |

**Returns:** [*random*](classes/random.md)

___

### random\_from\_time

▸ **random_from_time**(): [*random*](classes/random.md)

Creates a new random generator from the current system time.

**Returns:** [*random*](classes/random.md)

___

### read\_file

▸ **read_file**(`file`: *string*): *string*

Reads a file from the file system.

**`note`** This operation does NOT require explicit permission from the user.

#### Parameters:

Name | Type |
:------ | :------ |
`file` | *string* |

**Returns:** *string*

___

### rotate\_2d

▸ **rotate_2d**(`point`: [*vector\_3d*](classes/vector_3d.md), `origin`: [*vector\_3d*](classes/vector_3d.md), `angle`: *number*): [*vector\_3d*](classes/vector_3d.md)

Returns a 3d point around an origin, ignoring the y value.

#### Parameters:

Name | Type |
:------ | :------ |
`point` | [*vector\_3d*](classes/vector_3d.md) |
`origin` | [*vector\_3d*](classes/vector_3d.md) |
`angle` | *number* |

**Returns:** [*vector\_3d*](classes/vector_3d.md)

___

### round

▸ **round**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### select\_between

▸ **select_between**(`point1`: [*vector\_3d*](classes/vector_3d.md), `point2`: [*vector\_3d*](classes/vector_3d.md)): [*selection*](classes/selection.md)

Makes and returns a rectangular selection between two points.

#### Parameters:

Name | Type |
:------ | :------ |
`point1` | [*vector\_3d*](classes/vector_3d.md) |
`point2` | [*vector\_3d*](classes/vector_3d.md) |

**Returns:** [*selection*](classes/selection.md)

___

### select\_origin

▸ **select_origin**(`origin`: [*vector\_3d*](classes/vector_3d.md), `xRadius`: *number*, `zRadius`: *number*): [*selection*](classes/selection.md)

Makes and returns a rectangular selection around an origin point

#### Parameters:

Name | Type | Description |
:------ | :------ | :------ |
`origin` | [*vector\_3d*](classes/vector_3d.md) | The center point of the selection   |
`xRadius` | *number* |  |
`zRadius` | *number* |  |

**Returns:** [*selection*](classes/selection.md)

selection

___

### sin

▸ **sin**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### sinh

▸ **sinh**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### sqrt

▸ **sqrt**(`arg`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`arg` | *number* |

**Returns:** *any*

___

### tan

▸ **tan**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### tanh

▸ **tanh**(`a`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`a` | *number* |

**Returns:** *any*

___

### vec

▸ **vec**(`x`: *number*, `y`: *number*, `z`: *number*): [*vector\_3d*](classes/vector_3d.md)

Creates a new vector from its components

#### Parameters:

Name | Type |
:------ | :------ |
`x` | *number* |
`y` | *number* |
`z` | *number* |

**Returns:** [*vector\_3d*](classes/vector_3d.md)

___

### write\_file

▸ **write_file**(`file`: *string*, `content`: *string*): *void*

Writes text to a file

**`note`** This operation REQUIRES explicit permission from the user,
or it will throw an error.

#### Parameters:

Name | Type |
:------ | :------ |
`file` | *string* |
`content` | *string* |

**Returns:** *void*
