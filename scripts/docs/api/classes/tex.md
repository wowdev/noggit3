# Class: tex

Represents a single texture unit in the worlds texture layers.

A texture unit represents the smallest area where it's possible to
affect textures alpha layers. This is much smaller than the areas made
up by heightmap vertices.

## Table of contents

### Constructors

- [constructor](tex.md#constructor)

### Methods

- [get\_alpha](tex.md#get_alpha)
- [get\_pos\_2d](tex.md#get_pos_2d)
- [set\_alpha](tex.md#set_alpha)

## Constructors

### constructor

\+ **new tex**(): [*tex*](tex.md)

**Returns:** [*tex*](tex.md)

## Methods

### get\_alpha

▸ **get_alpha**(`index`: *number*): *number*

#### Parameters:

Name | Type |
:------ | :------ |
`index` | *number* |

**Returns:** *number*

___

### get\_pos\_2d

▸ **get_pos_2d**(): [*vector\_3d*](vector_3d.md)

**Returns:** [*vector\_3d*](vector_3d.md)

___

### set\_alpha

▸ **set_alpha**(`index`: *number*, `alpha`: *number*): *void*

#### Parameters:

Name | Type |
:------ | :------ |
`index` | *number* |
`alpha` | *number* |

**Returns:** *void*
