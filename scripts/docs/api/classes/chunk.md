# Class: chunk

Represents a chunk in the world

## Table of contents

### Constructors

- [constructor](chunk.md#constructor)

### Methods

- [add\_texture](chunk.md#add_texture)
- [apply\_all](chunk.md#apply_all)
- [apply\_heightmap](chunk.md#apply_heightmap)
- [apply\_textures](chunk.md#apply_textures)
- [apply\_vertex\_color](chunk.md#apply_vertex_color)
- [clear\_colors](chunk.md#clear_colors)
- [clear\_textures](chunk.md#clear_textures)
- [get\_area\_id](chunk.md#get_area_id)
- [get\_texture](chunk.md#get_texture)
- [remove\_texture](chunk.md#remove_texture)
- [set\_area\_id](chunk.md#set_area_id)
- [set\_hole](chunk.md#set_hole)
- [set\_impassable](chunk.md#set_impassable)

## Constructors

### constructor

\+ **new chunk**(): [*chunk*](chunk.md)

**Returns:** [*chunk*](chunk.md)

## Methods

### add\_texture

▸ **add_texture**(`texture`: *string*): *number*

Adds a new texture at the current topmost layer.

**`note`** A chunk can hold at most 4 texture layers.

#### Parameters:

Name | Type |
:------ | :------ |
`texture` | *string* |

**Returns:** *number*

___

### apply\_all

▸ **apply_all**(): *void*

Applies all changes in this chunk

**Returns:** *void*

___

### apply\_heightmap

▸ **apply_heightmap**(): *void*

Applies all changes to the heightmap in this chunk

**Returns:** *void*

___

### apply\_textures

▸ **apply_textures**(): *void*

Applies all changes to texture alphamaps in this chunk

**Returns:** *void*

___

### apply\_vertex\_color

▸ **apply_vertex_color**(): *void*

Applies all changes to vertex colors in this chunk

**Returns:** *void*

___

### clear\_colors

▸ **clear_colors**(): *void*

Removes all vertex colors in this chunk

**Returns:** *void*

___

### clear\_textures

▸ **clear_textures**(): *void*

Removes all texture layers in this chunk

**Returns:** *void*

___

### get\_area\_id

▸ **get_area_id**(): *number*

Returns the area id of a chunk

**Returns:** *number*

___

### get\_texture

▸ **get_texture**(`index`: *number*): *string*

Returns the name of the texture at the specified layer.

#### Parameters:

Name | Type |
:------ | :------ |
`index` | *number* |

**Returns:** *string*

___

### remove\_texture

▸ **remove_texture**(`index`: *number*): *void*

Removes a texture layer from this chunk
and decreases the texture ids of all higher layers by 1

#### Parameters:

Name | Type |
:------ | :------ |
`index` | *number* |

**Returns:** *void*

___

### set\_area\_id

▸ **set_area_id**(`value`: *number*): *void*

Changes the area id of a chunk

#### Parameters:

Name | Type |
:------ | :------ |
`value` | *number* |

**Returns:** *void*

___

### set\_hole

▸ **set_hole**(`hole`: *boolean*): *void*

Creates or removes a hole in this chunk

#### Parameters:

Name | Type |
:------ | :------ |
`hole` | *boolean* |

**Returns:** *void*

___

### set\_impassable

▸ **set_impassable**(`impassable`: *boolean*): *void*

Sets whether this chunk should be impassable for players or not

#### Parameters:

Name | Type |
:------ | :------ |
`impassable` | *boolean* |

**Returns:** *void*
