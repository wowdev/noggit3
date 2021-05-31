# Class: noisemap

Represents a map of floats values, typically use with a
noise generator.

## Table of contents

### Constructors

- [constructor](noisemap.md#constructor)

### Methods

- [get](noisemap.md#get)
- [height](noisemap.md#height)
- [is\_highest](noisemap.md#is_highest)
- [set](noisemap.md#set)
- [width](noisemap.md#width)

## Constructors

### constructor

\+ **new noisemap**(): [*noisemap*](noisemap.md)

**Returns:** [*noisemap*](noisemap.md)

## Methods

### get

▸ **get**(`pos`: [*vector\_3d*](vector_3d.md)): *number*

Returns the float value at a specific 3d position.

**`note`** The 'y' value of pos is ignored by this operation.

#### Parameters:

Name | Type |
:------ | :------ |
`pos` | [*vector\_3d*](vector_3d.md) |

**Returns:** *number*

___

### height

▸ **height**(): *number*

**Returns:** *number*

___

### is\_highest

▸ **is_highest**(`pos`: [*vector\_3d*](vector_3d.md), `check_radius`: *number*): *boolean*

Returns true if the float value at a 3d position
is the highest position within a given range.

This is typically used for object placement.

#### Parameters:

Name | Type |
:------ | :------ |
`pos` | [*vector\_3d*](vector_3d.md) |
`check_radius` | *number* |

**Returns:** *boolean*

___

### set

▸ **set**(`x`: *number*, `y`: *number*, `value`: *number*): *void*

#### Parameters:

Name | Type |
:------ | :------ |
`x` | *number* |
`y` | *number* |
`value` | *number* |

**Returns:** *void*

___

### width

▸ **width**(): *number*

**Returns:** *number*
