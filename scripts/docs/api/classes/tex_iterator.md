# Class: tex\_iterator

An iterator of texture units in the worlds texture layers.

This iterator always starts out "before" the first entry,
meaning you must call "next" before you call "get":

**`example`** (lua)
while tex_iter:next() do
   local tex_unit = tex_iter:get()
end

## Table of contents

### Constructors

- [constructor](tex_iterator.md#constructor)

### Methods

- [get](tex_iterator.md#get)
- [next](tex_iterator.md#next)

## Constructors

### constructor

\+ **new tex_iterator**(): [*tex\_iterator*](tex_iterator.md)

**Returns:** [*tex\_iterator*](tex_iterator.md)

## Methods

### get

▸ **get**(): [*tex*](tex.md)

Returns the currently selected vertex.

May only be called after a call to 'next' has returned true.

**Returns:** [*tex*](tex.md)

___

### next

▸ **next**(): *boolean*

Selects the next vertex in this iterator.

**Returns:** *boolean*
