# Class: vert

Represents a single heightmap vertex in the world.

Changes to this vertex takes visible effect in Noggit after you call
"apply" on the chunk or selection that contains it.

## Table of contents

### Constructors

- [constructor](vert.md#constructor)

### Methods

- [add\_height](vert.md#add_height)
- [get\_alpha](vert.md#get_alpha)
- [get\_pos](vert.md#get_pos)
- [is\_water\_aligned](vert.md#is_water_aligned)
- [set\_alpha](vert.md#set_alpha)
- [set\_color](vert.md#set_color)
- [set\_height](vert.md#set_height)
- [set\_hole](vert.md#set_hole)
- [set\_water](vert.md#set_water)
- [sub\_height](vert.md#sub_height)

## Constructors

### constructor

\+ **new vert**(): [*vert*](vert.md)

**Returns:** [*vert*](vert.md)

## Methods

### add\_height

▸ **add_height**(`y`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`y` | *number* |

**Returns:** *any*

___

### get\_alpha

▸ **get_alpha**(`index`: *number*): *any*

Returns the average alpha of all texture units closest to this vertex.

#### Parameters:

Name | Type |
:------ | :------ |
`index` | *number* |

**Returns:** *any*

___

### get\_pos

▸ **get_pos**(): [*vector\_3d*](vector_3d.md)

Returns the full position of this vertex

**Returns:** [*vector\_3d*](vector_3d.md)

___

### is\_water\_aligned

▸ **is_water_aligned**(): *boolean*

Returns true if this vertex is aligned with water tiles.

**Returns:** *boolean*

___

### set\_alpha

▸ **set_alpha**(`index`: *number*, `alpha`: *number*): *void*

Sets a texture alpha layer of all texture units closest to this vertex.

#### Parameters:

Name | Type |
:------ | :------ |
`index` | *number* |
`alpha` | *number* |

**Returns:** *void*

___

### set\_color

▸ **set_color**(`red`: *number*, `green`: *number*, `blue`: *number*): *void*

Changes the vertex color that this vertex blends with the
underlying texture. Values generally range between 0-1, but can
also go higher.

#### Parameters:

Name | Type | Description |
:------ | :------ | :------ |
`red` | *number* | How much red should be used (default: 1)   |
`green` | *number* | How much green should be used (default: 1)   |
`blue` | *number* | How much blue should be used (default: 1)    |

**Returns:** *void*

___

### set\_height

▸ **set_height**(`y`: *number*): *any*

Changes the height of this vertex

#### Parameters:

Name | Type |
:------ | :------ |
`y` | *number* |

**Returns:** *any*

___

### set\_hole

▸ **set_hole**(`has_hole`: *boolean*): *void*

Sets whether this vertex should be a hole, but only if the
vertex is aligned with hole tiles. If the vertex is not aligned
with a hole tile, this function does nothing.

#### Parameters:

Name | Type |
:------ | :------ |
`has_hole` | *boolean* |

**Returns:** *void*

___

### set\_water

▸ **set_water**(`type`: *number*, `height`: *number*): *void*

Changes the water type on this vertex, but only if the vertex is
aligned with water tiles. If the vertex is not aligned
with a water tile, this function does nothing.

**`note`** The C++ function backing this operation is very slow for the moment.
use with care.

#### Parameters:

Name | Type |
:------ | :------ |
`type` | *number* |
`height` | *number* |

**Returns:** *void*

___

### sub\_height

▸ **sub_height**(`y`: *number*): *any*

#### Parameters:

Name | Type |
:------ | :------ |
`y` | *number* |

**Returns:** *any*
