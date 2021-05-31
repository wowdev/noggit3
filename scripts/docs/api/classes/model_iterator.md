# Class: model\_iterator

An iterator of models in the world. Iterates both m2 and wmo models.

This iterator always starts out "before" the first entry,
meaning you must call "next" before you call "get":

**`example`** (lua)
while model_iter:next() do
   local model = model_iter:get()
end

## Table of contents

### Constructors

- [constructor](model_iterator.md#constructor)

### Methods

- [get](model_iterator.md#get)
- [next](model_iterator.md#next)
- [reset](model_iterator.md#reset)

## Constructors

### constructor

\+ **new model_iterator**(): [*model\_iterator*](model_iterator.md)

**Returns:** [*model\_iterator*](model_iterator.md)

## Methods

### get

▸ **get**(): [*model*](model.md)

Returns the currently selected vertex.

May only be called after a call to 'next' has returned true.

**Returns:** [*model*](model.md)

___

### next

▸ **next**(): *boolean*

Selects the next model in this iterator.

**Returns:** *boolean*

___

### reset

▸ **reset**(): *void*

Resets this iterator to again point _before_ the first entry.

**Returns:** *void*
