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

▸ **chunks**(): [*chunk\_iterator*](chunk_iterator.md)

Creates and returns an iterator for all chunks inside this selection

**Returns:** [*chunk\_iterator*](chunk_iterator.md)

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

▸ **models**(): [*model\_iterator*](model_iterator.md)

Creates and returns an iterator for all models inside this selection

**Returns:** [*model\_iterator*](model_iterator.md)

___

### size

▸ **size**(): [*vector\_3d*](vector_3d.md)

Returns a vector representing the size of this selection on each axis.

**`note`** for iterators, only x and z values are respected. y (height) is ignored.

**Returns:** [*vector\_3d*](vector_3d.md)

___

### tex

▸ **tex**(): [*tex\_iterator*](tex_iterator.md)

Creates and returns an iterator for all texture units inside this selection

**Returns:** [*tex\_iterator*](tex_iterator.md)

___

### verts

▸ **verts**(): [*vert\_iterator*](vert_iterator.md)

Creates and returns an iterator for all vertices inside this selection

**Returns:** [*vert\_iterator*](vert_iterator.md)
