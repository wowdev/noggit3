# Class: chunk\_iterator

An iterator of chunks in the world.

This iterator always starts out "before" the first entry,
meaning you must call "next" before you call "get":

**`example`** (lua)
while chunk_iter:next() do
   local chunk = chunk_iter:get()
end

## Table of contents

### Constructors

- [constructor](chunk_iterator.md#constructor)

### Methods

- [get](chunk_iterator.md#get)
- [next](chunk_iterator.md#next)
- [reset](chunk_iterator.md#reset)

## Constructors

### constructor

\+ **new chunk_iterator**(): [*chunk\_iterator*](chunk_iterator.md)

**Returns:** [*chunk\_iterator*](chunk_iterator.md)

## Methods

### get

▸ **get**(): [*chunk*](chunk.md)

Returns the currently selected chunk.

May only be called after a call to 'next' has returned true.

**Returns:** [*chunk*](chunk.md)

___

### next

▸ **next**(): *boolean*

Selects the next chunk in this iterator.

**Returns:** *boolean*

___

### reset

▸ **reset**(): *void*

Resets this iterator to again point _before_ the first entry.

**Returns:** *void*
