# Class: vert\_iterator

An iterator of vertices in the worlds heightmap.

This iterator always starts out "before" the first entry,
meaning you must call "next" before you call "get":

**`example`** (lua)
while vert_iter:next() do
   local vert_unit = vert_iter:get()
end

## Table of contents

### Constructors

- [constructor](vert_iterator.md#constructor)

### Methods

- [get](vert_iterator.md#get)
- [next](vert_iterator.md#next)

## Constructors

### constructor

\+ **new vert_iterator**(): [*vert\_iterator*](vert_iterator.md)

**Returns:** [*vert\_iterator*](vert_iterator.md)

## Methods

### get

▸ **get**(): [*vert*](vert.md)

Returns the currently selected vertex.

May only be called after a call to 'next' has returned true.

**Returns:** [*vert*](vert.md)

___

### next

▸ **next**(): *boolean*

Selects the next vertex in this iterator.

**Returns:** *boolean*
