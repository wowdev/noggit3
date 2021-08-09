# Class: selection

Represents a rectangular selection in the world and provides
iterators for heightmap vertices, texture units, chunks and models within it.

## Table of contents

### Constructors

- [constructor](selection.md#constructor)

### Methods

- [apply](selection.md#apply)
- [center](selection.md#center)
- [chunks](selection.md#chunks)
- [make\_noise](selection.md#make_noise)
- [max](selection.md#max)
- [min](selection.md#min)
- [models](selection.md#models)
- [size](selection.md#size)
- [tex](selection.md#tex)
- [verts](selection.md#verts)

## Constructors

### constructor

\+ **new selection**(): [*selection*](selection.md)

**Returns:** [*selection*](selection.md)

## Methods

### apply

▸ **apply**(): *void*

Applies all changes made inside this selection.
You almost always want to call this function when you're done
with a selection.

**Returns:** *void*

___

### center

▸ **center**(): [*vector\_3d*](vector_3d.md)

Returns the center point of this selection

**Returns:** [*vector\_3d*](vector_3d.md)

___

### chunks

▸ **chunks**(): [*chunk*](chunk.md)[]

Creates and returns an iterator for all chunks inside this selection

**Returns:** [*chunk*](chunk.md)[]

___

### make\_noise

▸ **make_noise**(`frequency`: *number*, `algorithm`: *string*, `seed`: *string*): [*noisemap*](noisemap.md)

Creates a noisemap matching the location of this selection

#### Parameters:

Name | Type |
:------ | :------ |
`frequency` | *number* |
`algorithm` | *string* |
`seed` | *string* |

**Returns:** [*noisemap*](noisemap.md)

___

### max

▸ **max**(): [*vector\_3d*](vector_3d.md)

Returns the highest point of this selection

**Returns:** [*vector\_3d*](vector_3d.md)

___

### min

▸ **min**(): [*vector\_3d*](vector_3d.md)

Returns the smallest point of this selection

**Returns:** [*vector\_3d*](vector_3d.md)

___

### models

▸ **models**(): [*model*](model.md)[]

Creates and returns an iterator for all models inside this selection

**Returns:** [*model*](model.md)[]

___

### size

▸ **size**(): [*vector\_3d*](vector_3d.md)

Returns a vector representing the size of this selection on each axis.

**`note`** for iterators, only x and z values are respected. y (height) is ignored.

**Returns:** [*vector\_3d*](vector_3d.md)

___

### tex

▸ **tex**(): [*tex*](tex.md)[]

Creates and returns an iterator for all texture units inside this selection

**Returns:** [*tex*](tex.md)[]

___

### verts

▸ **verts**(): [*vert*](vert.md)[]

Creates and returns an iterator for all vertices inside this selection

**Returns:** [*vert*](vert.md)[]
